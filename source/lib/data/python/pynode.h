#include "pynode.h"

PyNodeWrapper::PyNodeWrapper()
{
}

PyNodeWrapper::~PyNodeWrapper()
{
}

void PyNodeWrapper::wrap()
{
    BPy::class_<PyNodeWrappe>("Node", BPy::no_init);
}

BPy::object PyNodeWrapper::pyObj()    
{
    return pynode;
}
