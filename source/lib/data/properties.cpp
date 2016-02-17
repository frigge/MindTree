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
#include "exception"
#include "glm/glm.hpp"
#include "iostream"
#include "data/debuglog.h"

#include "properties.h"

using namespace MindTree;

template<class T> const MindTree::DataType MindTree::PropertyTypeInfo<T>::_type;

PROPERTY_TYPE_INFO(float, "FLOAT");
PROPERTY_TYPE_INFO(double, "FLOAT");
PROPERTY_TYPE_INFO(int, "INTEGER");
PROPERTY_TYPE_INFO(bool, "BOOLEAN");
PROPERTY_TYPE_INFO(PropertyMap, "PROPERTYMAP");
PROPERTY_TYPE_INFO(std::string, "STRING");
PROPERTY_TYPE_INFO(glm::vec2, "VECTOR2D");
PROPERTY_TYPE_INFO(glm::ivec2, "INTVECTOR2D");
PROPERTY_TYPE_INFO(glm::vec3, "VECTOR3D");
PROPERTY_TYPE_INFO(glm::vec4, "COLOR");
PROPERTY_TYPE_INFO(glm::mat4, "MAT4");

TypeDispatcher<DataType, ConverterList> PropertyConverter::_converters;

void PropertyConverter::registerConverter(DataType from, DataType to, ConverterFunctor fn)
{
    _converters[from][to] = fn; 
}

bool PropertyConverter::isConvertible(DataType from, DataType to)
{
    auto fn = _converters[from][to];
    if(fn)
        return true;
    return false;
}

ConverterFunctor PropertyConverter::get(DataType from, DataType to)
{
    return _converters[from][to];
}

Property::Property()
    : data(nullptr),
     type("undefined")
     /* default fn objects to avoid std::bad_function_call*/
     //by default reset property value as this property is empty as well
{
}

Property::Property(const Property &other) noexcept
    : data(nullptr),
    type("undefined")
{
    other._meta.cloneData(*this);
}

Property::Property(const Property &&other) noexcept
{
    other._meta.moveData(*this);
}

Property::~Property() noexcept
{
}

Property& Property::operator=(const Property &other)     noexcept
{
    other._meta.cloneData(*this);
    return *this;
}

Property& Property::operator=(const Property &&other) noexcept
{
    other._meta.moveData(*this);
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
        if(BPy::len(tuple) == 2) {
            std::string elemType = Python::type(pyobj[0]);
            if(elemType == "int") { 
                int x, y;
                x = BPy::extract<int>(pyobj[0]);
                y = BPy::extract<int>(pyobj[1]);
                glm::ivec2 val(x, y);
                return Property(val);
            }        
            else if(elemType == "float") { 
                double x, y;
                x = BPy::extract<float>(pyobj[0]);
                y = BPy::extract<float>(pyobj[1]);
                glm::vec2 val(x, y);
                return Property(val);
            }
            else if(elemType == "long") { 
                long x, y;
                x = BPy::extract<long>(pyobj[0]);
                y = BPy::extract<long>(pyobj[1]);
                glm::ivec2 val(x, y);
                return Property(val);
            }
        } else if(BPy::len(tuple) == 3) {
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
    return Property();
}

Property Property::clone()const
{
    return Property(*this);
}

const MindTree::DataType& Property::getType() const
{
    return type;
}

BPy::object Property::toPython() const
{
    if(!data) {
        return BPy::object();
    }
    return _meta.pyconverter();
}

PropertyMap::PropertyMap(std::initializer_list<Info> init)
{
    _properties.insert(std::begin(_properties), init);
}

void PropertyMap::insert(Info value)
{
    _properties.push_back(value);
}

void PropertyMap::insert(Iterator b, Iterator e)
{
    _properties.insert(_properties.end(), b, e);
}

void PropertyMap::clear()
{
    _properties.clear();
}

size_t PropertyMap::size() const
{
    return _properties.size();
}

bool PropertyMap::empty() const
{
    return _properties.empty();
}

Property PropertyMap::at(const std::string &name)
{
    return *std::find_if(std::begin(_properties),
                         std::end(_properties),
                         [&name](const Info& info) {
            return info.first == name;
        });
}

const Property& PropertyMap::at(const std::string &name) const
{
    size_t pos = std::distance(_properties.cbegin(),
                               std::find_if(_properties.cbegin(),
                                            _properties.cend(),
                                            [&name](const Info& info) {
                                                return info.first == name;
                                            }));
    return _properties.at(pos).second;
}

Property& PropertyMap::operator[](const std::string &name)
{
    auto it = std::find_if(_properties.begin(),
                           _properties.end(),
                           [&name](const Info& info) {
                               return info.first == name;
                           });
    if (it == _properties.end()) {
        _properties.push_back(std::make_pair(name, Property()));
        return _properties.back().second;
    }
    size_t pos = std::distance(_properties.begin(), it);
    return _properties.at(pos).second;
}

Property& PropertyMap::operator[](std::string &&name)
{
    auto it = std::find_if(_properties.begin(),
                           _properties.end(),
                           [&name](const Info& info) {
                               return info.first == name;
                           });
    if (it == _properties.end()) {
        _properties.emplace_back(std::make_pair(name, Property()));
        return _properties.back().second;
    }
    size_t pos = std::distance(_properties.begin(), it);
    return _properties.at(pos).second;
}

PropertyMap::Iterator PropertyMap::begin()
{
    return _properties.begin();
}

PropertyMap::Iterator PropertyMap::end()
{
    return _properties.end();
}

PropertyMap::CIterator PropertyMap::cbegin() const
{
    return _properties.cbegin();
}

PropertyMap::CIterator PropertyMap::cend() const
{
    return _properties.cend();
}

PropertyMap::Iterator PropertyMap::find(const std::string& name)
{
    return std::find_if(_properties.begin(),
                        _properties.end(),
                        [&name](const Info& info) {
                            return info.first == name;
                        });
}

PropertyMap::CIterator PropertyMap::find(const std::string& name) const
{
    return std::find_if(_properties.begin(),
                        _properties.end(),
                        [&name](const Info& info) {
                            return info.first == name;
                        });
}

void PropertyMap::erase(const std::string& name)
{
    _properties.erase(find(name));
}

PropertyMap::Iterator begin(PropertyMap& map)
{
    return map.begin();
}

PropertyMap::Iterator end(PropertyMap& map)
{
    return map.end();
}

PropertyMap::CIterator begin(const PropertyMap& map)
{
    return map.cbegin();
}

PropertyMap::CIterator end(const PropertyMap& map)
{
    return map.cend();
}

IO::Input::ReaderList IO::Input::_readers;

Property IO::Input::read(IO::InStream& stream) noexcept
{
    DataType t;
    stream >> t;

    auto reader = _readers[t];
    Property prop;
    if(reader) prop = reader(stream);

    return prop;
}

IO::OutStream& MindTree::operator<<(IO::OutStream &stream, const Property &prop)
{
    stream.beginBlock("Property");
    stream << static_cast<const TypeBase&>(prop.getType());
    prop._meta.writeData(stream, prop);
    stream.endBlock("Property");
    return stream;
}

IO::InStream& MindTree::operator>>(IO::InStream &stream, Property &prop)
{
    stream.beginBlock("Property");
    prop = IO::Input::read(stream);
    stream.endBlock("Property");

    return stream;
}
