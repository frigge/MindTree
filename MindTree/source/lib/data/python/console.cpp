#include "console.h"
#include "cstdio"
#include "Python.h"

namespace PyMT {
PyStdCatcher::PyStdCatcher(PyConsole *console)
    : console(console)
{
}

PyStdCatcher::~PyStdCatcher()
{
}

void PyStdCatcher::write(std::string string)    
{
   console->write(string.c_str());
}

PyConsole::PyConsole()
{
    //sys_module.attr("__dict__")["stdin"] = BPy::class_<PyStdCatcher>("StdinCatcher")
    //                                        .def("readline", &PyStdoutCatcher::readline)();
    BPy::class_<PyStdCatcher>("StdoutCatcher", BPy::no_init)
                                            .def("write", &PyStdCatcher::write);
    catcher = BPy::object(new PyStdCatcher(this));
}

PyConsole::~PyConsole()
{
    //thread.terminate();
    //thread.wait();
}

void PyConsole::start()    
{
    //timer.start(100);
}

void PyConsole::command(QString cmd)    
{
    BPy::object main = BPy::import("__main__");
    BPy::object global = main.attr("__dict__");
    auto sys_module = BPy::import("sys");
    sys_module.attr("__dict__")["stdout"] = catcher;
    sys_module.attr("__dict__")["stderr"] = catcher;
    try{
        BPy::object eval = BPy::eval(cmd.toStdString().c_str(), global, global);
        std::string evalstr = BPy::extract<std::string>(BPy::str(eval));
        BPy::object strobj = BPy::str(eval);
        std::string str = BPy::extract<std::string>(strobj);
        if(str != "None")emit updated(QString(str.c_str())+"\n");
    } catch(BPy::error_already_set const &){
        PyErr_Clear();
        try{
            BPy::exec(cmd.toStdString().c_str(), global, global);
        }catch(BPy::error_already_set const &){
            PyErr_Print();
        }
    }
    sys_module.attr("__dict__")["stdout"] = sys_module.attr("__stdout__");
    sys_module.attr("__dict__")["stderr"] = sys_module.attr("__stderr__");
}

void PyConsole::write(QString string)    
{
    if(!string.isEmpty()) {
        emit updated(string);
    }
}

}
