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

#include "QColor"
#include "boost/python.hpp"
#include "data/python/wrapper.h"
#include "unordered_map"

namespace BPy=boost::python;

namespace MindTree
{

class Property;

template<class T>
class PropertyData
{
public:
    PropertyData(){}
    PropertyData(T data) : data(data) {}
    PropertyData(const PropertyData &prop) : data(prop.data){}
    virtual ~PropertyData() {}

    T getData()const
    {
        return data;
    }

    void setData(T d)
    {
        data = d;
    }

    static void registerType(std::string id)
    {
        propertyTypeID = id;
    }

    static std::string getType()
    {
        return propertyTypeID;
    }

    static void* clone(const void *other)
    {
        return new PropertyData<T>(*((PropertyData<T>*)other));
    }

private:
    static std::string propertyTypeID;
    T data;
};

//template<class T>
//class PropertyData<T*>
//{
//public:
//    PropertyData() : data(0) {}
//    PropertyData(T *data) : data(data) {}
//    PropertyData(const PropertyData &prop) : data(new T(*prop.data)) {}
//    virtual ~PropertyData() {if(data)delete data;}
//
//    T* getData()const
//    {
//        return data;
//    }
//
//    void setData(T *d)
//    {
//        data = d;
//    }
//
//    static void registerType(std::string id)
//    {
//        propertyTypeID = id;
//    }
//
//    static std::string getType()
//    {
//        return propertyTypeID;
//    }
//
//    static QString getType(T type)
//    {
//        return propertyTypeID;
//    }
//
//    static void* clone(const void *other)
//    {
//        return new PropertyData<T>(*((PropertyData<T>*)other));
//    }
//
//private:
//    static std::string propertyTypeID;
//    T *data;
//};

template<class T> std::string PropertyData<T>::propertyTypeID = "undefined";
//template<class T> std::string PropertyData<T*>::propertyTypeID = "undefined";

class Property
{
public:
    Property(std::string name="");

    template<typename T>
    Property(T data, std::string name="")
        : Property(name)
    {
        setData<T>(data);
    }

    Property(const Property &other);
    virtual ~Property();

    Property& operator=(const Property &other);

    Property clone()const;
    static Property createPropertyFromPython(std::string name, const BPy::object &pyobj);


    template<typename T>
    void setData(T d){
        cloneData = [this](Property &other) {
            other.setData<T>(this->getData<T>());
        };

        if(data)deleteFunc();
        pyconverter = [this]{ return PyConverter<T>::pywrap(this->getData<T>()); };
        data = new PropertyData<T>(d);
        type = PropertyData<T>::getType();
        deleteFunc = [this]{delete reinterpret_cast<PropertyData<T>*>(this->data); this->data=0; type="undefined";};
    }

    template<typename T>
    T getData(){
        //initialize on demand with default value
        if(!data) {
            data = new PropertyData<T>();
        }
        if(PropertyData<T>::getType() != type)
            return T();
        return ((PropertyData<T>*)data)->getData();
    }

    BPy::object toPython();
    std::string getType();
    std::string getName();

private:
    std::function<void(Property&)> cloneData;
    std::function<void()> deleteFunc;
    std::function<BPy::object()> pyconverter;
    void *data;
    std::string type;
    std::string name;
};

typedef std::unordered_map<std::string, Property> PropertyMap;
    
} /* MindTree */
#endif /* end of include guard: PROPERTIES_K7LMQN2D */
