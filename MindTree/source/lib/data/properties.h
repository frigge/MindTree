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

#ifndef PROPERTIES_K7LMQN2D

#define PROPERTIES_K7LMQN2D

#include "data/python/wrapper.h"
#include "unordered_map"
#include "data/type.h"

namespace BPy=boost::python;

namespace MindTree
{

class Property;
template<class T>
struct PropertyTypeInfo
{
    static const DataType _type;
};

#define PROPERTY_TYPE_INFO(TYPE, STR) \
    template<> const MindTree::DataType MindTree::PropertyTypeInfo<TYPE>::_type{STR}

typedef std::pair<char*, size_t> RawData;
typedef std::function<RawData(RawData)> ConverterFunctor;
typedef std::vector<ConverterFunctor> ConverterList;

template<class T>
class PropertyConverter
{
public:
private:
    static ConverterList _fromTConverter;
    friend class Property;
};

template<class T>
class PropertyData
{
public:
    PropertyData(){}
    PropertyData(T data) : data(data) {}
    PropertyData(const PropertyData &prop) : data(prop.data){}

    T getData()const
    {
        return data;
    }

    void setData(T d)
    {
        data = d;
    }

    static void* clone(const void *other)
    {
        return new PropertyData<T>(*((PropertyData<T>*)other));
    }

private:
    T data;
};

namespace IO {
template<class T> struct Writer;
std::ostream& writeProperty(std::ostream& stream, const Property &prop);
}


class Property
{
public:
    Property();

    template<typename T>
    Property(T data)
    : Property()
    {
        setData<T>(data);
    }

    Property(const Property &other);
    virtual ~Property();

    Property& operator=(const Property &other);

    Property clone()const;
    static Property createPropertyFromPython(const BPy::object &pyobj);


    template<typename T>
    void setData(T d){
        cloneData = [this](Property &other) {
            other.setData<T>(this->getData<T>());
        };
        
        writeData = &IO::Writer<T>::write;

        datasize = sizeof(d);

        if(data)deleteFunc();
        pyconverter = [this]{ return PyConverter<T>::pywrap(this->getData<T>()); };
        data = new PropertyData<T>(d);
        type = PropertyTypeInfo<T>::_type;
        deleteFunc = [this]{delete reinterpret_cast<PropertyData<T>*>(this->data); this->data=0; this->type=DataType();};
    }

    template<typename T>
    T getData() const{
        //initialize on demand with default value
        if(!data) {
            data = new PropertyData<T>();
        }
        if(PropertyTypeInfo<T>::_type != type)
            return T();
        return ((PropertyData<T>*)data)->getData();
    }

    BPy::object toPython() const;
    DataType getType() const;

private:
    friend std::ostream& MindTree::IO::writeProperty(std::ostream& stream, const Property &prop);

    std::function<void(Property&)> cloneData;
    std::function<std::ostream&(std::ostream&, const Property&)> writeData;
    std::function<void()> deleteFunc;
    std::function<BPy::object()> pyconverter;
    size_t datasize;
    mutable void *data;
    ConverterList *_converterList;
    DataType type;
};


typedef std::unordered_map<std::string, Property> PropertyMap;
    
namespace IO {
template <typename T>
struct Writer {
    static std::ostream& write(std::ostream &stream, const Property &prop)
    {
        T data = prop.getData<T>();
        const char *raw = reinterpret_cast<const char*>(&data);
        stream.write(raw, sizeof(data));
        return stream;
    }
};

template <>
struct Writer<std::string> {
    static std::ostream& write(std::ostream &stream, const Property &prop)
    {
        std::string data = prop.getData<std::string>();
        const char *raw = data.c_str();
        stream.write(raw, data.size());
        return stream;
    }
};

class Input
{
public:
    static void registerReader(std::function<Property(std::istream&)> reader,  std::string t);
    static Property read(std::istream &stream);
        
private:
    static std::vector<std::function<Property(std::istream&)>> _readers;
};
}
} /* MindTree */

#endif /* end of include guard: PROPERTIES_K7LMQN2D */
