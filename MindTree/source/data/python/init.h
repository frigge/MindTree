/*
    FRG Shader Editor, a Node-based Renderman Shading Language Editor
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

#ifndef INIT_WYB0E8S9

#define INIT_WYB0E8S9

#include "QPointF"
#include "QString"
#include "Python.h"
#include "data/nodes/data_node_socket.h"
#include "boost/python.hpp"

namespace BPy = boost::python;
namespace MindTree
{
    
namespace Python{

std::string type(const BPy::object &obj);
int getGraphicsCount();
void wrap_all();

struct QPointFToPython
{
    static PyObject* convert(QPointF const &pos);
};

struct QStringToPython
{
    static PyObject* convert(QString const &s);
};

template<class T>
struct QListToPython
{
    static PyObject* convert(QList<T> const &l)
    {
        BPy::list list;
        foreach(T e, l){
            list.append<T>(e);
        }
        return BPy::incref(list.ptr());
    }
};

struct DSocketListToPython
{
    static PyObject* convert(MindTree::DSocketList const &l);
};

//template<class T>
//struct PointerToPython
//{
//    static PyObject* convert(const T* t)
//    {
//        return BPy::incref(BPy::ptr(t));
//    }
//};
//
struct PythonToQString
{
    PythonToQString();
    static void* convertible(PyObject *obj);
    static void construct(PyObject* obj, boost::python::converter::rvalue_from_python_stage1_data* data);
};

void init(int argc, char* argv[]);
void finalize();
void load(QString plugin);

void loadPlugins();
void loadIntern();
void loadSettings();

} /* Pyton */

} /* MindTree */
#endif /* end of include guard: INIT_WYB0E8S9 */
