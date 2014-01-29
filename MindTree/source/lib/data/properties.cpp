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


#include "source/data/python/init.h"
#include "data/nodes/data_node_socket.h"
#include "exception"

#include "properties.h"

using namespace MindTree;

Property::Property(std::string name)
    : data(0),
     name(name),
     type("undefined"),
     /* default fn objects to avoid std::bad_function_call*/
     //by default reset property value as this property is empty as well
     cloneData([](Property &prop){prop.deleteFunc();}),
     deleteFunc([]{}),
     pyconverter([]{return BPy::object();})
{
}

Property::Property(const Property &other)
    : Property(other.name)
{
    other.cloneData(*this);
}

Property::~Property()
{
    deleteFunc(); 
}

Property& Property::operator=(const Property &other)    
{
    other.cloneData(*this);
    return *this;
}

Property Property::createPropertyFromPython(std::string name, const BPy::object &pyobj)    
{
    auto t = MindTree::Python::type(pyobj);
    if(t == "int") { 
        int val = BPy::extract<int>(pyobj);
        return Property(val, name);
    }        
    else if(t == "str" || t == "unicode") {
        std::string val = BPy::extract<std::string>(pyobj);
        return Property(val, name);
    }
    else if(t == "float") { 
        float val = BPy::extract<float>(pyobj);
        return Property(val, name);
    }
    else if(t == "long") { 
        long val = BPy::extract<long>(pyobj);
        return Property(val, name);
    }
    else if(t == "dict") {
        std::vector<Property> prop_vec;
        auto items = BPy::dict(pyobj).items();
        for(auto i = 0; i<BPy::len(items); i++) {
            std::string n = BPy::extract<std::string>(items[i][0]);
            prop_vec.push_back(createPropertyFromPython(n, BPy::object(items[i][1])));
        }
        return Property(prop_vec, name);
    }
    else
    return Property();
}

Property Property::clone()const
{
    return Property(*this);
}

std::string Property::getType()    
{
    return type;
}

std::string Property::getName()    
{
    return name;
}

BPy::object Property::toPython()    
{
    if(!data) {
        return BPy::object();
    }
    return pyconverter(); 
}
