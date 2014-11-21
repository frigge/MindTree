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

#include "type_traits"
#include "data/python/wrapper.h"
#include "unordered_map"
#include "vector"
#include "data/type.h"
#include "data/io.h"

namespace BPy=boost::python;

namespace MindTree
{

class Property;

template<typename T>
class PropertyTypeInfo
{
public:
    static DataType getType()
    {
        return _type;
    }


private:
    static const DataType _type;
};

template<typename T>
class PropertyTypeInfo<std::vector<T>>
{
public:
    static DataType getType()
    {
        return "LIST:" + PropertyTypeInfo<T>::getType().toStr();
    }
};

#define PROPERTY_TYPE_INFO(TYPE, STR) \
    template<> const MindTree::DataType MindTree::PropertyTypeInfo<TYPE>::_type{STR}

typedef std::function<void(void*, void*)> ConverterFunctor;
typedef TypeDispatcher<DataType, ConverterFunctor> ConverterList;

class PropertyConverter
{
public:
    static void registerConverter(DataType from, DataType to, ConverterFunctor fn);
    static bool isConvertible(DataType from, DataType to);
    static ConverterFunctor get(DataType from, DataType to);

private:
    static TypeDispatcher<DataType, ConverterList> _converters;
};

template<class T>
class PropertyData
{
public:
    PropertyData(){}
    PropertyData(T data) : data(data) {}
    PropertyData(const PropertyData &prop) : data(prop.data){}

    const T& getData()const
    {
        return data;
    }

    T& getData()
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

template<typename F, typename T>
void defaultPropertyConverter(void* from, void* to)
{
    F _from = reinterpret_cast<PropertyData<F>*>(from)->getData();
    T *_to = reinterpret_cast<T*>(to);
    *_to = _from;
}

namespace IO {
template<class T> struct Writer;
}

IO::OutStream& operator<<(IO::OutStream &stream, const Property &prop);
IO::InStream& operator>>(IO::InStream &stream, Property &prop);

class Property
{
private:
    template<typename T>
    struct isSTDVectorTrait {
        static const bool value = false;
    };

    template<typename T>
    struct isSTDVectorTrait<std::vector<T>> {
        static const bool value = true;
    };

    struct ListToolsBase {
        virtual Property createList(int cnt, Property def) = 0;
        virtual Property getItem(const Property &list, int index) const = 0;
        virtual void setItem(Property *list, int index, Property value) = 0;
        virtual size_t getSize(const Property *list) const = 0;
    };

    template<typename T>
    struct ListTools : public ListToolsBase {
        virtual Property createList(int cnt, Property def) override
        {
            T value = def.getData<T>();

            std::vector<T> vec(cnt, value);
            auto prop = MindTree::Property(vec);
            return prop;
        }

        virtual Property getItem(const Property &, int) const override { return Property(); }
        virtual void setItem(Property *, int, Property) override {}
        virtual size_t getSize(const Property *) const override { return 1; }
    };

    template<typename T>
    struct ListTools<std::vector<T>> : public ListToolsBase {
        Property getItem(const Property &list, int index) const override
        {
            const auto &vec = list.getDataRef<std::vector<T>>();
            if(index >= vec.size())
                return Property();

            return vec.at(index);
        }

        void setItem(Property *list, int index, Property value) override
        {
            auto &vec = list->getDataRef<std::vector<T>>();
            if(vec.size() <= index)
                vec.resize(index + 1);

            vec[index] = value.getData<T>();
        }

        size_t getSize(const Property *list) const override
        {
            return list->getData<std::vector<T>>().size();
        }

        Property createList(int cnt, Property def) override { return Property(); }
    };

    struct Meta {
        Meta() 
            : cloneData([](Property &prop){prop._meta.deleteFunc();}),
             moveData([](Property &prop){prop._meta.deleteFunc();}),
             writeData([](IO::OutStream& stream, const Property &) { }),
             deleteFunc([]{}),
             pyconverter([]{return BPy::object();})
        {}

        std::function<void(Property&)> cloneData;
        std::function<void(Property&)> moveData;
        std::function<void(IO::OutStream&, const Property&)> writeData;
        std::function<void()> deleteFunc;
        std::function<BPy::object()> pyconverter;
        std::unique_ptr<ListToolsBase> listTools;
    };

public:
    Property();

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    Property(T data)
    : Property()
    {
        setData<T>(data);
    }

    Property(const Property &other) noexcept;
    Property(const Property &&other) noexcept;

    virtual ~Property() noexcept;

    Property& operator=(const Property &other) noexcept;
    Property& operator=(const Property &&other) noexcept;

    Property clone()const;
    static Property createPropertyFromPython(const BPy::object &pyobj);

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    void setData(T d){
        if(data) _meta.deleteFunc();
        setMetaData<T>();

        data = new PropertyData<T>(d);
    }

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    T getData() const
    {
        //initialize on demand with default value
        if(!data) {
            data = new PropertyData<T>();
        }
        if(PropertyTypeInfo<T>::getType() != type) {
            if(!PropertyConverter::isConvertible(PropertyTypeInfo<T>::getType(), type))
                return T();

            auto converter = PropertyConverter::get(type, PropertyTypeInfo<T>::getType());
            T converted;
            converter(data, reinterpret_cast<void*>(&converted));

            return converted;
        }
        return reinterpret_cast<PropertyData<T>*>(data)->getData();
    }

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    T& getDataRef()
    {
        //initialize on demand with default value
        if(!data) {
            data = new PropertyData<T>();
        }
        return reinterpret_cast<PropertyData<T>*>(data)->getData();
    }

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    const T& getDataRef() const
    {
        //initialize on demand with default value
        if(!data) {
            data = new PropertyData<T>();
        }
        return reinterpret_cast<PropertyData<T>*>(data)->getData();
    }

    BPy::object toPython() const noexcept;
    const MindTree::DataType& getType() const;
    inline operator bool() const
    {
        return data;
    }
    
    inline bool isList() const
    {
        return _meta.listTools.get();
    }

    inline static Property getItem(const Property &list, int index)
    {
        if(!list._meta.listTools) return Property();
        return list._meta.listTools->getItem(list, index);
    }

    inline static void setItem(Property &list, int index, Property value)
    {
        if(!list._meta.listTools) return;
        list._meta.listTools->setItem(&list, index, value);
    }

    inline size_t size() const
    {
        if(!_meta.listTools.get()) {
            return 1;
        }

        return _meta.listTools->getSize(this);
    }

    inline Property createList(size_t cnt)
    {
        if(!_meta.listTools) return Property();
        return _meta.listTools->createList(cnt, *this);
    }

private:
    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    void setMetaData()
    {
        _meta.cloneData = [this](Property &other) {
            other.setData<T>(this->getData<T>());
        };

        _meta.moveData = [this](Property &other) {
            other._meta.deleteFunc();
            other.data = this->data;
            this->data = nullptr;
            other.setMetaData<T>();
        };

        _meta.writeData = &IO::Writer<T>::write;

        if(data)_meta.deleteFunc();
        _meta.pyconverter = [this]{ return PyConverter<T>::pywrap(this->getData<T>());};
        type = PropertyTypeInfo<T>::getType();
        _meta.deleteFunc = [this]{
            if(this->data) delete reinterpret_cast<PropertyData<T>*>(this->data); 
            this->data=nullptr; 
            this->type=DataType();
        };

        _meta.listTools = std::unique_ptr<ListToolsBase>(new ListTools<T>());
    }

    friend IO::OutStream& MindTree::operator<<(IO::OutStream& stream, const Property &prop);

    mutable void *data;

    Meta _meta;
    DataType type;
};


typedef std::unordered_map<std::string, Property> PropertyMap;

namespace IO {
template <typename T>
struct Writer {
    static void write(IO::OutStream &stream, const Property &prop)
    {
        T data = prop.getData<T>();
        stream << data;
    }
};

template <typename T>
struct Writer<std::shared_ptr<T>> {
    static void write(IO::OutStream &stream, const Property &prop)
    {
        std::shared_ptr<T> data = prop.getData<std::shared_ptr<T>>();
        stream << *data;
    }
};

template<class T>
Property defaultReader(IO::InStream &stream)
{
    T data;
    stream >> data;
    Property prop{data};
    return prop;
}

class Input
{
public:
    typedef std::function<Property(IO::InStream&)> ReaderFunction;
    typedef TypeDispatcher<DataType, ReaderFunction> ReaderList;

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    static void registerReader()
    {
        auto reader = [](IO::InStream &stream) {
            T data;
            stream >> data;
            Property prop(data);
            return prop;
        };

        _readers.add(PropertyTypeInfo<T>::getType(), reader);
    }
    static Property read(IO::InStream &stream) noexcept;
        
private:
    static ReaderList _readers;

};

}
} /* MindTree */

#endif /* end of include guard: PROPERTIES_K7LMQN2D */
