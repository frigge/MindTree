#include "consolewindow.h"

#include "iostream"
#include "QKeyEvent"
#include "QLabel"
#include "QLineEdit"
#include "QScrollBar"
#include "QVBoxLayout"

#include "data/frg.h"
#include "source/graphics/base/mindtree_mainwindow.h"
#include "data/python/console.h"

TabFilter::TabFilter(QObject *parent)
    : QObject(parent)
{
}

bool TabFilter::eventFilter(QObject *obj, QEvent *event)    
{
    if(event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Tab){
            return true;
        }
    }
    return false;
}

ConsoleInput::ConsoleInput(QWidget *parent)
    : QLineEdit(parent), historyIndex(-1)
{
    setFont(QFont("Courier New"));
}

ConsoleInput::~ConsoleInput()
{
}

bool ConsoleInput::event(QEvent *event)    
{
    if(event->type() == QEvent::KeyPress
        &&((QKeyEvent*)event)->key() == Qt::Key_Tab){
            setText(text() + "    ");
            return true;
    }
    else return QLineEdit::event(event);
}

void ConsoleInput::keyPressEvent(QKeyEvent *event)    
{
    switch(event->key()){
        case Qt::Key_Up:
            if(cmdHistory.isEmpty()) break;
            if(historyIndex == cmdHistory.size() - 1 && text() != ""
                && text() != cmdHistory[cmdHistory.size() -1]){
                currentInput = text();
            }
            setText(cmdHistory[historyIndex]);
            if(historyIndex > 0)historyIndex--;
            break;
        case Qt::Key_Down:
            if(cmdHistory.isEmpty()) break;
            if(historyIndex < cmdHistory.size()-1) {
                historyIndex++;
                setText(cmdHistory[historyIndex]);
            }
            else
                setText(currentInput);
            break;
        case Qt::Key_Return:
            if(!text().isEmpty() && (cmdHistory.isEmpty() || text() != cmdHistory[cmdHistory.size() - 1])) {
                cmdHistory << text();
                historyIndex = cmdHistory.size() - 1;
            }
            QLineEdit::keyPressEvent(event);
            break;
        default:
            QLineEdit::keyPressEvent(event);
    }
}

ConsoleArea::ConsoleArea(QWidget *parent)
    : QScrollArea(parent), label(new QLabel(this)), widget(new QWidget())
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    label->setFont(QFont("Courier New"));
    auto lay = new QVBoxLayout();
    widget->setLayout(lay);
    lay->addStretch();
    lay->addWidget(label);
    lay->setMargin(0);
    lay->setSpacing(0);
    label->setWordWrap(true);
    widget->setMinimumSize(size());
    setWidget(widget);
}

ConsoleArea::~ConsoleArea()
{
}

void ConsoleArea::resizeEvent(QResizeEvent*)    
{
     widget->setMinimumSize(size());
}

void ConsoleArea::addText(QString text)    
{
    label->setText(label->text() + text);
    label->adjustSize();
    label->setFixedWidth(width() - 20);
    scrollDown();
    widget->adjustSize();
}

void ConsoleArea::scrollDown()    
{
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

ConsoleWindow::ConsoleWindow(QWidget *parent)
    : QWidget(parent), area(new ConsoleArea(this)), edit(new ConsoleInput(this)), prompt(new QLabel(">>>")),
        diffParen(0), diffSqParen(0), scope(0), console(new PyMT::PyConsole())
{
    connect(console, SIGNAL(updated(QString)), this, SLOT(printOutput(QString)));
    connect(edit, SIGNAL(returnPressed()), this, SLOT(enter()));
    connect(this, SIGNAL(execCmd(QString)), console, SLOT(command(QString)));
    QVBoxLayout *lay = new QVBoxLayout();
    setLayout(lay);
    edit->setTextMargins(0, 0, 0, 0);
    edit->setFrame(false);
    lay->addWidget(area);
    QHBoxLayout *hlay = new QHBoxLayout;
    lay->addLayout(hlay);
    hlay->addWidget(prompt);
    hlay->addWidget(edit);
    lay->setMargin(0);
    lay->setSpacing(0);
    hlay->setMargin(0);
    hlay->setSpacing(0);
    edit->setStyleSheet("background-color: black;"
                        "color: white;"
                        "border-image: none;");
    prompt->setStyleSheet("background-color: black;"
                        "color: white;"
                        "border-image: none;");
    area->setStyleSheet("background-color: black;"
                        "color: white;"
                        "border-image: none;");
    prompt->setFont(QFont("Courier New"));
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::printOutput(QString out)    
{
    area->addText(out);
}

QString ConsoleWindow::getPyString()    
{
    QString cmd = edit->text();
    edit->clear();
    area->addText(prompt->text() + " " + cmd + "\n");
    return cmd;
}

bool ConsoleWindow::matchingParen(QString cmd)    
{
    diffParen += cmd.count("(") - cmd.count(")");
    diffSqParen += cmd.count("[") - cmd.count("]");
    return diffParen == 0 && diffSqParen == 0;
}

void ConsoleWindow::enter()    
{
    QString cmd = getPyString();
    multilineCmd += "\n"+cmd;
    if(cmd.isEmpty() && scope) scope--;
    if(cmd.endsWith(":")) scope++;

    if(matchingParen(cmd) 
            &&!scope 
            &&!cmd.startsWith("@")
            &&!cmd.endsWith("\\")){
        emit execCmd(multilineCmd);
        multilineCmd = "";
        prompt->setText(">>> ");
        return;
    }
    if(!cmd.isEmpty())prompt->setText("... ");
}

ConsoleWindowDock::ConsoleWindowDock()
    : console(new ConsoleWindow(this))
{
    setWidget(console);
    MindTree::FRG::Author->addDockWidget(Qt::LeftDockWidgetArea, this);
}

ConsoleWindowDock::~ConsoleWindowDock()
{
}   
