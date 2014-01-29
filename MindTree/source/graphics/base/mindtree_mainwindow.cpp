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


//#include "QtGui"
#include "QGraphicsItem"
#include "QGraphicsView"
#include "QGraphicsScene"
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
#include "QDir"

#include "iostream"

#include "math.h"

#include "data/frg.h"
#include "data/dnspace.h"
#include "data/nodes/data_node.h"
//#include "source/graphics/shaderpreview.h"
//#include "source/graphics/previewdock.h"
#include "source/graphics/properties_editor.h"
//#include "source/graphics/detailsview.h"
#include "source/data/data_info_box.h"
#include "source/graphics/sourcedock.h"
#include "source/graphics/python/consolewindow.h"
#include "source/data/python/init.h"
#include "source/data/base/init.h"
#include "data/properties.h"
#include "graphics/windowlist.h"
#include "data/signal.h"
//#include "source/graphics/base/consoledialog.h"
#include "mindtree_mainwindow.h"

using namespace MindTree;

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    QPushButton *reloadPluginsButton = new QPushButton("Reload Plugins");
    lay->addWidget(reloadPluginsButton);
    connect(reloadPluginsButton, SIGNAL(clicked()), FRG::Author, SLOT(loadPlugins()));
}

SettingsDialog::~SettingsDialog()
{
}

bool frg_Shader_Author::mouseNodeGraphPos = false;
bool frg_Shader_Author::pickWidget = false;

frg_Shader_Author::frg_Shader_Author(QWidget *parent)
    : QMainWindow(parent)
#ifdef QT_DEBUG
    , style_age(-1)
#endif
{
    setObjectName("MindTreeMainWindow");
    FRG::Author = this;
    setWindowTitle("MindTree");
    resize(1500, 1000);

    setAnimated(false);
    setDockNestingEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    auto windowList = MindTree::WindowList::instance();
    auto viewerList = MindTree::ViewerList::instance();
    windowList->setMainWindow(this);
    viewerList->setMainWindow(this);

    connect(windowList, SIGNAL(windowFactoryAdded(MindTree::WindowFactory*)), this, SLOT(registerWindow(MindTree::WindowFactory*)));

    registerMenu(new QMenu("&Window"));
    auto fac = new WindowFactory("Console", addConsole);
    MindTree::WindowList::instance()->addFactory(fac);

    statusBar()->showMessage("Welcome to MindTree");

    initDefaultProperties();
    setupStyle();

#ifdef QT_DEBUG
    qApp->installEventFilter(this);
    QTimer *t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(lookupStyle()));
    t->start(2000);
    std::cout<<"I'm in debug mode so i'm gonna read the styles in realtime"<<std::endl;
#endif

    MindTree::Signal::getHandler<QString>().add("filename_changed",[this](QString title){
        this->change_window_title(title); 
    }).detach();
};

frg_Shader_Author::~frg_Shader_Author()
{
    MindTree::finalizeApp();
}

void frg_Shader_Author::showDock(QDockWidget* widget)    
{
    addDockWidget(Qt::LeftDockWidgetArea, widget);
    widget->show();
}

void frg_Shader_Author::registerMenu(QMenu *menu)    
{
    menus.insert(menu->title(), menu);
    menuBar()->addMenu(menu);
}

void frg_Shader_Author::registerWindow(WindowFactory *factory)    
{
    //windowFactories.append(factory); 
    std::cout << "adding menu entry for window" << std::endl;
    menus.value("&Window")->addAction(factory->action());
}

#ifdef QT_DEBUG
void frg_Shader_Author::lookupStyle()    
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

void frg_Shader_Author::mouseMoveEvent(QMouseEvent *event)    
{
    if(!pickWidget){
        QMainWindow::mouseMoveEvent(event);
        return;
    }

    QWidget *w = qApp->widgetAt(QCursor::pos());
    qint64 ptr = (qint64)w;
    statusBar()->showMessage("Pointer: "+QString::number(ptr));
}

bool frg_Shader_Author::eventFilter(QObject *obj, QEvent *event)    
{
    if(event->type() == QEvent::MouseMove){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(pickWidget){
            QWidget *w = qApp->widgetAt(QCursor::pos());
            if(w)
                statusBar()->showMessage(w->objectName()
                                    + " " + w->metaObject()->className());
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void frg_Shader_Author::togglePickWidget()    
{
    pickWidget = !pickWidget;
    if(pickWidget) mouseNodeGraphPos = false;
}

void frg_Shader_Author::toggleMouseNodeGraphPos()    
{
    mouseNodeGraphPos = !mouseNodeGraphPos;
    if(mouseNodeGraphPos ) pickWidget = false;
}

bool frg_Shader_Author::isMouseNodeGraphPos()    
{
   return mouseNodeGraphPos; 
}

void frg_Shader_Author::setupStyle()    
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

void frg_Shader_Author::initDefaultProperties()    
{
}


void frg_Shader_Author::createSpaceDock()    
{
}

void frg_Shader_Author::createDocks()    
{
    //TODO: change into a dialog
    //nodeeditor = new QDockWidget("Node Editor");
    //nodeeditor->setWidget(new NewNodeEditor);
    //addDockWidget(Qt::TopDockWidgetArea, nodeeditor);    
    //nodeeditor->hide();

    //nodelib = new NodeLibDock(this);

    //propeditor = new QDockWidget("Properties Editor");
    //propeditor->setWidget(new PropertiesEditor(this));
    //addDockWidget(Qt::LeftDockWidgetArea, propeditor);

    //detailsview = new QDockWidget("Details View");
    //detailsview->setWidget(new DetailsView(this));
    //addDockWidget(Qt::RightDockWidgetArea, detailsview);
    //tabifyDockWidget(detailsview, spaceDock);
}

//void frg_Shader_Author::createPreviewDock(DShaderPreview *prev)    
//{
//    PreviewDock *prevDock = new PreviewDock(prev);
//    addDockWidget(Qt::RightDockWidgetArea, prevDock);
//}

void frg_Shader_Author::createMenus()
{
    //QMenu *fileMenu = new QMenu("&File");
    //QAction *newAction = fileMenu->addAction("&New");
    //newAction->setShortcuts(QKeySequence::New);
    //QAction *openAction = fileMenu->addAction("&Open");
    //openAction->setShortcuts(QKeySequence::Open);
    //QAction *saveAction = fileMenu->addAction("&Save");
    //saveAction->setShortcuts(QKeySequence::Save);
    //QAction *saveAsAction = fileMenu->addAction("Save &As");
    //saveAsAction->setShortcuts(QKeySequence::SaveAs);
    //QAction *quitAction = fileMenu->addAction("&Quit");
    //quitAction->setShortcuts(QKeySequence::Quit);

    //QMenu *editMenu = new QMenu("&Edit");
    //QAction *undoAction = editMenu->addAction("&Undo");
    //undoAction->setShortcuts(QKeySequence::Undo);
    //QAction *redoAction = editMenu->addAction("&Redo");
    //redoAction->setShortcuts(QKeySequence::Redo);
    //QAction *deleteAction = editMenu->addAction("&Delete");
    //deleteAction->setShortcuts(QKeySequence::Delete);
    //QAction *copyAction = editMenu->addAction("&Copy");
    //copyAction->setShortcuts(QKeySequence::Copy);
    //QAction *cutAction = editMenu->addAction("C&ut");
    //cutAction->setShortcuts(QKeySequence::Cut);
    //QAction *pasteAction = editMenu->addAction("&Paste");
    //pasteAction->setShortcuts(QKeySequence::Paste);
    //QAction *settingsAction = editMenu->addAction("&Settings...");

    //QMenu *nodeMenu = new QMenu("&Nodes");
    ////nodeMenu->setTearOffEnabled(true);
    //QAction *infoBoxAction = nodeMenu->addAction("&Info Box");
    //QAction *containerAction = nodeMenu->addAction("&Build Container");
    //containerAction->setShortcut(QString("c"));
    //QAction *unpackAction = nodeMenu->addAction("&Unpack Container");
    //unpackAction->setShortcut(QString("Alt+c"));
    //QAction *nodeEditorAction = nodeMenu->addAction("&New Node");
    //nodeEditorAction->setShortcut(QString("n"));
    //nodeEditorAction->setCheckable(true);
    //QAction *nodeLibAction = nodeMenu->addAction("&Node Library");
    //nodeLibAction->setCheckable(true);
    //nodeLibAction->setChecked(true);
    //nodeLibAction->setShortcut(QString("l"));

    //QMenu *previewMenu = new QMenu("&Preview");
    //QAction *editprevAction = previewMenu->addAction("&Edit Previews");
    //connect(editprevAction, SIGNAL(triggered()), this, SLOT(openPreviewEditor()));


    //connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    //connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
    //connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    //connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    //connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    //connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    //connect(deleteAction, SIGNAL(triggered()), this, SLOT(removeNodes()));
    //connect(copyAction, SIGNAL(triggered()), FRG::Space, SLOT(copy()));
    //connect(cutAction, SIGNAL(triggered()), FRG::Space, SLOT(cut()));
    //connect(pasteAction, SIGNAL(triggered()), FRG::Space, SLOT(paste()));
    //connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));

    //connect(infoBoxAction, SIGNAL(triggered()), this, SLOT(addInfoBox()));
    //connect(containerAction, SIGNAL(triggered()), this, SLOT(buildContainer()));
    //connect(unpackAction, SIGNAL(triggered()), this, SLOT(unpackContainer()));
    //connect(nodeEditorAction, SIGNAL(triggered()), this, SLOT(toggleNodeEditor()));
    //connect(nodeLibAction, SIGNAL(triggered()), this, SLOT(toggleNodeLib()));

    //registerMenu(fileMenu);
    //registerMenu(editMenu);
    //registerMenu(nodeMenu);
    //registerMenu(previewMenu);
};

void frg_Shader_Author::openSettings()    
{
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->show(); 
}

QWidget* frg_Shader_Author::addConsole()    
{
    //char *argv = new char;
    //argv = '\0';
    //Py_Main(0, &argv);
    ConsoleWindow *console = new ConsoleWindow();
    return console;
    //ConsoleDialog *console = new ConsoleDialog(this);
}

void frg_Shader_Author::addTextDock()    
{
    //new SourceDock();
}

void frg_Shader_Author::addImageDock()    
{
}

void frg_Shader_Author::toggleNodeLib()    
{
    if(nodelib->isVisible())
        nodelib->hide();
    else
        nodelib->show();
}

void frg_Shader_Author::toggleNodeEditor()    
{
    //if(nodeeditor->isVisible())
    //    nodeeditor->hide();
    //else
    //    nodeeditor->show();
}

void frg_Shader_Author::removeNodes()    
{
    //NodeList nodes = FRG::Space->selectedNodes();
    //FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoRemoveNode(nodes));
    //FRG::Space->removeNode(nodes);
}

void frg_Shader_Author::buildContainer()    
{
    //FRG::Space->buildContainer();
}

void frg_Shader_Author::unpackContainer()    
{
    //FRG::Space->unpackContainer(); 
}

void frg_Shader_Author::change_window_title(QString title)
{
    QString newtitle("MindTree");
    newtitle.append(" - ");
    newtitle.append(title);
    setWindowTitle(newtitle);
}

void frg_Shader_Author::undo()    
{
    //FRG::SpaceDataInFocus->undo();
}

void frg_Shader_Author::redo()    
{
    //FRG::SpaceDataInFocus->redo();
}

