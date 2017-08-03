/*
    frg_Shader_Author Shader Editor, a Node-based Renderman Shading Language Editor
    Copyright (C) 2011  Sascha Fricke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "QPainterPath"
#include "QWheelEvent"
#include "QPalette"
#include "QMenu"
#include "QDockWidget"
#include "QTimer"
#include "QDateTime"
#include "QCursor"
#include "QPushButton"
#include "QStatusBar"
#include "QMenuBar"
#include "QResource"
#include "QTextStream"
#include "QFileDialog"
#include "QVBoxLayout"
#include "QDir"

#include "iostream"

#include "math.h"

#include "data/dnspace.h"
#include "data/nodes/data_node.h"
#include "source/graphics/python/consolewindow.h"
#include "source/data/python/init.h"
#include "source/data/base/init.h"
#include "data/properties.h"
#include "graphics/windowlist.h"
#include "data/signal.h"
#include "data/debuglog.h"
#include "mindtree_mainwindow.h"

using namespace MindTree;


SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    QPushButton *reloadPluginsButton = new QPushButton("Reload Plugins");
    lay->addWidget(reloadPluginsButton);
    connect(reloadPluginsButton, SIGNAL(clicked()), MainWindow::window(), SLOT(loadPlugins()));
}

SettingsDialog::~SettingsDialog()
{
}

MainWindow* MainWindow::_window = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), style_age(-1)
{
    setObjectName("MindTreeMainWindow");
	QString title = "MindTree";
	auto filename = Project::instance()->getFilename();

	if (!filename.empty())
		title += QString(" - ") + filename.c_str();
    setWindowTitle(title);
    resize(1500, 1000);

    setAnimated(false);
    setDockNestingEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDocumentMode(true);

    auto windowList = MindTree::WindowList::instance();
    auto viewerList = MindTree::ViewerList::instance();
    windowList->setMainWindow(this);
    viewerList->setMainWindow(this);

    connect(windowList, SIGNAL(windowFactoryAdded(MindTree::WindowFactory*)), this, SLOT(registerWindow(MindTree::WindowFactory*)));

	addFileMenu();
    registerMenu(new QMenu("&Window"));
    auto fac = new WindowFactory("Console", addConsole);
    MindTree::WindowList::instance()->addFactory(fac);

    statusBar()->showMessage("Welcome to MindTree");

    setupStyle();

#ifdef QT_DEBUG
    qApp->installEventFilter(this);
    QTimer *t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(lookupStyle()));
    t->start(2000);
#endif

    MindTree::Signal::getHandler<std::string>().connect("filename_changed",[this](std::string title){
			this->change_window_title(title.c_str());
    }).detach();

    MindTree::Signal::getHandler<std::string>().connect("STATUSUPDATE",[this](std::string message){
			QString qmessage(message.c_str());
			QMetaObject::invokeMethod(this,
									  "updateStatusBar",
									  Qt::QueuedConnection,
									  Q_ARG(QString, qmessage));
    }).detach();
};

MainWindow::~MainWindow()
{
    MindTree::finalizeApp();
}

void MainWindow::addFileMenu()
{
	QMenu *file = new QMenu("&File");
	registerMenu(file);

	QAction *newAction = new QAction("New");
	QAction *openAction = new QAction("Open");
	QAction *saveAction = new QAction("Save");
	QAction *saveAsAction = new QAction("Save As");

	file->addAction(newAction);
	file->addAction(openAction);
	file->addAction(saveAction);
	file->addAction(saveAsAction);

	connect(newAction, &QAction::triggered, [](){ Project::create(); });
	connect(openAction, &QAction::triggered, this, &MainWindow::openProject);
	connect(saveAction, &QAction::triggered, this, &MainWindow::saveProject);
	connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsProject);
}

void MainWindow::openProject()
{
	QString project = QFileDialog::getOpenFileName(this, "Open Project", "../", "*.mt");
	Project::load(project.toStdString());
}

void MainWindow::saveProject()
{
	auto filename = Project::instance()->getFilename();
	if (filename.empty()) {
		saveAsProject();
		return;
	}
	Project::instance()->save();
}

void MainWindow::saveAsProject()
{
	QString project = QFileDialog::getSaveFileName(this, "Save Project", "../", "*.mt");
	Project::instance()->setFilename(project.toStdString());
	Project::instance()->save();
}

void MainWindow::updateStatusBar(QString message)
{
	statusBar()->showMessage(message);
}

MainWindow* MainWindow::create()
{
    _window = new MainWindow(); 
    return _window;
}

MainWindow* MainWindow::window()
{
    return _window;
}

void MainWindow::showDock(QDockWidget* widget)    
{
    addDockWidget(Qt::LeftDockWidgetArea, widget);
    widget->show();
}

void MainWindow::registerMenu(QMenu *menu)    
{
    menus.insert(menu->title(), menu);
    menuBar()->addMenu(menu);
}

void MainWindow::registerWindow(WindowFactory *factory)    
{
    //windowFactories.append(factory); 
    std::cout << "adding menu entry for window" << std::endl;
    menus.value("&Window")->addAction(factory->action());
}

#ifdef QT_DEBUG
void MainWindow::lookupStyle()    
{
    if(style_age == -1){
        setupStyle();
        style_age = QFileInfo(PyWrapper::getStylePath()).lastModified().toMSecsSinceEpoch();
        return;
    }
    qint64 age = QFileInfo(PyWrapper::getStylePath()).lastModified().toMSecsSinceEpoch();
    if(age > style_age) {
        std::cout<<"Style changed, reloading ..." << std::endl;
        style_age = age;
        setupStyle();
    }
}
#endif

void MainWindow::setupStyle()    
{
#ifdef QT_DEBUG
    QResource::unregisterResource(PyWrapper::getStylePath());
    QResource::registerResource(PyWrapper::getStylePath());
#endif
    QFile css(":/style.css");
    css.open(QIODevice::ReadOnly);
    QTextStream str(&css);
    QString style(str.readAll());
    qApp->setStyleSheet(style);
}

void MainWindow::openSettings()    
{
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->show(); 
}

QWidget* MainWindow::addConsole()    
{
    ConsoleWindow *console = new ConsoleWindow();
    return console;
}

void MainWindow::change_window_title(QString title)
{
    QString newtitle("MindTree");
    newtitle.append(" - ");
    newtitle.append(title);
    setWindowTitle(newtitle);
}
