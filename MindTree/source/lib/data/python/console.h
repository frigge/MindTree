#ifndef CONSOLE

#define CONSOLE
#include "QTimer"
#include "QThread"
#include "boost/python.hpp"
namespace BPy = boost::python;

namespace PyMT{
class PyConsole;
class PyStdCatcher
{
    PyConsole *console;
public:
    PyStdCatcher(PyConsole *console);
    virtual ~PyStdCatcher();
    void write(std::string string);
};

class PyConsole : public QObject
{
    Q_OBJECT
public:
    PyConsole();
    virtual ~PyConsole();
    void write(QString string);

public slots:
    void start();
    void command(QString cmd);

signals:
    void updated(QString);

private:
    QTimer timer;
    QString readConsole();
    FILE *file;
    QThread thread;
    BPy::object catcher;
};
}

#endif /* end of include guard: CONSOLE*/
