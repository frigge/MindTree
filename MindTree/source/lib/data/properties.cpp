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
#include "glm/glm.hpp"
#include "iostream"

#include "properties.h"

using namespace MindTree;

template<class T> const MindTree::DataType MindTree::PropertyTypeInfo<T>::_type;

PROPERTY_TYPE_INFO(float, "FLOAT");
PROPERTY_TYPE_INFO(double, "FLOAT");
PROPERTY_TYPE_INFO(int, "INTEGER");
PROPERTY_TYPE_INFO(std::string, "STRING");
PROPERTY_TYPE_INFO(glm::vec2, "VECTOR2D");
PROPERTY_TYPE_INFO(glm::vec3, "VECTOR3D");
PROPERTY_TYPE_INFO(glm::vec4, "COLOR");

Property::Property()
    : data(0), datasize(0),
     type("undefined"),
     /* default fn objects to avoid std::bad_function_call*/
     //by default reset property value as this property is empty as well
     cloneData([](Property &prop){prop.deleteFunc();}),
     deleteFunc([]{}),
     pyconverter([]{return BPy::object();})
{
}

Property::Property(const Property &other)
    : Property()
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

Property Property::createPropertyFromPython(const BPy::object &pyobj)    
{
    auto t = MindTree::Python::type(pyobj);
    if(t == "int") { 
        int val = BPy::extract<int>(pyobj);
        return Property(val);
    }        
    else if(t == "str" || t == "unicode") {
        std::string val = BPy::extract<std::string>(pyobj);
        return Property(val);
    }
    else if(t == "float") { 
        double val = BPy::extract<float>(pyobj);
        return Property(val);
    }
    else if (t == "bool") {
        bool val = BPy::extract<bool>(pyobj);
        return Property(val);
    }
    else if(t == "long") { 
        long val = BPy::extract<long>(pyobj);
        return Property(val);
    }
    else if(t == "tuple") {
        BPy::tuple tuple(pyobj);
        if(BPy::len(tuple) == 3) {
            double x, y, z;
            x = BPy::extract<float>(pyobj[0]);
            y = BPy::extract<float>(pyobj[1]);
            z = BPy::extract<float>(pyobj[2]);
            glm::vec3 val(x, y, z);
            return Property(val);
        }
        else if(BPy::len(tuple) == 4) {
            double x, y, z, w;
            x = BPy::extract<float>(pyobj[0]);
            y = BPy::extract<float>(pyobj[1]);
            z = BPy::extract<float>(pyobj[2]);
            w = BPy::extract<float>(pyobj[3]);
            glm::vec4 val(x, y, z, w);
            return Property(val);

        }
    }
    else if(t == "dict") {
        std::vector<Property> prop_vec;
        auto items = BPy::dict(pyobj).items();
        for(auto i = 0; i<BPy::len(items); i++) {
            std::string n = BPy::extract<std::string>(items[i][0]);
            prop_vec.push_back(createPropertyFromPython(BPy::object(items[i][1])));
        }
        return Property(prop_vec);
    }
    else
    return Property();
}

Property Property::clone()const
{
    return Property(*this);
}

DataType Property::getType() const
{
    return type;
}

BPy::object Property::toPython() const
{
    if(!data) {
        return BPy::object();
    }
    return pyconverter(); 
}


std::vector<std::function<Property(std::istream&)>> IO::Input::_readers;

void IO::Input::registerReader(std::function<Property(std::istream&)> reader, std::string t)
{
    int id = SocketType::getID(t);
    if (id >= _readers.size()) {
        _readers.resize(id + 1);
        _readers[id] = reader;
    }
}

Property IO::Input::read(std::istream &stream)
{
    std::string type;
    stream >> type;

    Property prop = _readers[SocketType::getID(type)](stream);

    return prop;
}

std::ostream& MindTree::IO::writeProperty(std::ostream &stream, const Property &prop)
{
    const char *raw = prop.type.toStr().c_str();
    stream.write(raw, prop.type.toStr().size());
    stream.write(reinterpret_cast<char*>(prop.datasize), sizeof(prop.datasize));
    return prop.writeData(stream, prop);
}
