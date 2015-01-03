#ifndef CONSOLE

#define CONSOLE
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

    Q_SLOT void command(QString cmd);

    Q_SIGNAL void updated(QString);

private:
    QString readConsole();
    FILE *file;
    QThread thread;
    BPy::object catcher;
};
}

#endif /* end of include guard: CONSOLE*/
