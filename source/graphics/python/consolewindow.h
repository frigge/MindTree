#ifndef CONSOLEWINDOW

#define CONSOLEWINDOW

#include "QScrollArea"
#include "QDockWidget"
#include "QLineEdit"

class QKeyEvent;
class QLabel;

class TabFilter : public QObject
{
    Q_OBJECT
public:
    TabFilter(QObject *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

class ConsoleInput : public QLineEdit
{
public:
    ConsoleInput(QWidget *parent=0);
    virtual ~ConsoleInput();

protected:
    bool event(QEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    int historyIndex;
    QStringList cmdHistory;
    QString currentInput;
};

class ConsoleArea : public QScrollArea
{
public:
    ConsoleArea(QWidget *parent = 0);
    virtual ~ConsoleArea();
    void addText(QString text);
    void scrollDown();
    void resizeEvent(QResizeEvent*);

private:
    QLabel *label;
    QWidget *widget;
};

namespace PyMT {class PyConsole;}

class ConsoleWindow : public QWidget
{
Q_OBJECT
public:
    ConsoleWindow(QWidget *parent = 0);
    virtual ~ConsoleWindow();

    Q_SLOT void printOutput(QString out);
    Q_SLOT void enter();

    Q_SIGNAL void execCmd(QString);
    Q_SIGNAL void contentChanged();

protected:
    QLineEdit *edit;
    QLabel *prompt;
    QString multilineCmd;
    QString getPyString();
    bool matchingParen(QString cmd);


private:
    int scope;
    int diffParen, diffSqParen;
    ConsoleArea *area;
    PyMT::PyConsole* console;
};
#endif /* end of include guard: CONSOLEWINDOW*/
