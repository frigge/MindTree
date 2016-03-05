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

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <stdexcept>

#include "data/python/wrapper.h"
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
    static std::shared_timed_mutex converter_mutex_;
    static TypeDispatcher<DataType, ConverterList> _converters;
};

class PropertyDataBase
{
 public:
    virtual ~PropertyDataBase() {};
};

template<class T>
class PropertyData : public PropertyDataBase
{
public:
    PropertyData(){}
    PropertyData(T data) : data(data) {}
    PropertyData(const PropertyData &prop) : data(prop.data){}
    ~PropertyData() {}

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

template<typename T>
struct isSTDVectorTrait {
    static const bool value = false;
};

template<typename T>
struct isSTDVectorTrait<std::vector<T>> {
    static const bool value = true;
};

struct PropertyDataTraits {
    PropertyDataTraits(Property *self) : self_(self) {}

    //general traits
    virtual void cloneData(Property &other) = 0;
    virtual void moveData(Property &other) = 0;
    virtual BPy::object pyconverter() = 0;

    virtual void writeData(IO::OutStream&, const Property&) = 0;

    //vector traits
    virtual Property createList(int cnt, Property def) const = 0;
    virtual Property getItem(int) const = 0;
    virtual void setItem(int index, Property) = 0;
    virtual size_t getSize() const = 0;
    virtual bool isList() const = 0;

    Property *self_ = nullptr;
};

template<typename T>
struct PropertyTypeTraits;

class Property
{
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
        data_ = std::make_unique<PropertyData<T>>(d);
        setMetaData<T>();
    }

    template<typename T>
    void setMetaData()
    {
        traits_ = std::make_unique<PropertyTypeTraits<T>>(this);
        type_ = PropertyTypeInfo<T>::getType();
    }

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    T getData() const
    {
        if (!data_) return T();

        //initialize on demand with default value
        if(PropertyTypeInfo<T>::getType() != type_) {
            if(!PropertyConverter::isConvertible(PropertyTypeInfo<T>::getType(), type_))
                return T();

            auto converter = PropertyConverter::get(type_, PropertyTypeInfo<T>::getType());
            T converted;
            converter(data_.get(), reinterpret_cast<void*>(&converted));

            return converted;
        }
        return static_cast<PropertyData<T>*>(data_.get())->getData();
    }

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    T& getDataRef()
    {
        //initialize on demand with default value
        if(!data_) {
            data_ = std::make_unique<PropertyData<T>>();
        }
        return reinterpret_cast<PropertyData<T>*>(data_.get())->getData();
    }

    template<typename T,
        typename std::enable_if<!std::is_same<T, Property*>::value>::type* = nullptr>
    const T& getDataRef() const
    {
        if(!data_) throw std::runtime_error("property is empty");
        return reinterpret_cast<PropertyData<T>*>(data_.get())->getData();
    }

    BPy::object toPython() const;
    const MindTree::DataType& getType() const;
    inline operator bool() const
    {
        return data_.get();
    }

    inline bool isList() const
    {
        if(!traits_) return false;
        return traits_->isList();
    }

    inline static Property getItem(const Property &list, int index)
    {
        if(!list.isList()) return Property();
        return list.traits_->getItem(index);
    }

    inline static void setItem(Property &list, int index, Property value)
    {
        if(!list.isList()) return;
        list.traits_->setItem(index, value);
    }

    inline size_t size() const
    {
        if(!isList()) {
            return 1;
        }

        return traits_->getSize();
    }

    inline Property createList(size_t cnt)
    {
        if(isList()) return *this;
        return traits_->createList(cnt, *this);
    }

private:
    template<typename T>
    friend struct PropertyTypeTraits;

    friend IO::OutStream& MindTree::operator<<(IO::OutStream& stream, const Property &prop);

    std::unique_ptr<PropertyDataBase> data_;
    std::unique_ptr<PropertyDataTraits> traits_;
    DataType type_;
};

template<typename T>
struct PropertyListTraits {
    static Property getItem(const Property &, int) { return Property(); }
    static void setItem(Property &, int, Property) {}
    static size_t getSize(const Property &) { return 1; }
    static bool isList() { return false; }
    static Property createList(int cnt, Property def)
    {
        T value = def.getData<T>();

        std::vector<T> vec(cnt, value);
        auto prop = MindTree::Property(vec);
        return prop;
    }
};

template<typename T>
struct PropertyListTraits<std::vector<T>> {
    static Property getItem(const Property &self, int index)
    {
        const auto &vec = self.getDataRef<std::vector<T>>();
        if(index >= vec.size())
            return Property();

        return vec.at(index);
    }

    static void setItem(Property &self, int index, Property value)
    {
        auto &vec = self.getDataRef<std::vector<T>>();
        if(vec.size() <= index)
            vec.resize(index + 1);

        vec[index] = value.getData<T>();
    }

    static size_t getSize(const Property &self)
    {
        return self.getDataRef<std::vector<T>>().size();
    }

    static Property createList(int cnt, Property def) { return Property(); }
    static bool isList() { return true; }
};

template<typename T>
struct PropertyListTraits<std::shared_ptr<std::vector<T>>> {
    static Property getItem(const Property &self, int index)
    {
        const auto &vec = self.getDataRef<std::shared_ptr<std::vector<T>>>();
        if(index >= vec->size())
            return Property();

        return vec->at(index);
    }

    static void setItem(Property &self, int index, Property value)
    {
        auto &vec = self.getDataRef<std::shared_ptr<std::vector<T>>>();
        if(vec->size() <= index)
            vec->resize(index + 1);

        (*vec)[index] = value.getData<T>();
    }

    static size_t getSize(const Property &self)
    {
        return self.getDataRef<std::shared_ptr<std::vector<T>>>()->size();
    }

    static bool isList() { return true; }
    static Property createList(int cnt, Property def) { return Property(); }
};

template<typename T>
struct PropertyTypeTraits : public PropertyDataTraits {

    PropertyTypeTraits(Property *self) : PropertyDataTraits(self) {}
    void cloneData(Property &other) override
    {
        other.setData<T>(self_->getData<T>());
    }

    void moveData(Property &other) override
    {
        other.data_ = std::move(self_->data_);
        other.setMetaData<T>();
    }

    void writeData(IO::OutStream &stream, const Property &prop) override
    {
        IO::Writer<T>::write(stream, prop);
    }

    BPy::object pyconverter() override
    {
        return PyConverter<T>::pywrap(self_->getData<T>());
    }

    Property createList(int cnt, Property def) const override
    {
        return PropertyListTraits<T>::createList(cnt, def);
    }

    Property getItem(int index) const override
    {
        return PropertyListTraits<T>::getItem(*self_, index);
    }

    void setItem(int index, Property value) override
    {
        return PropertyListTraits<T>::setItem(*self_, index, value);
    }

    size_t getSize() const override
    {
        return PropertyListTraits<T>::getSize(*self_);
    }

    bool isList() const override
    {
        return PropertyListTraits<T>::isList();
    }

};

class PropertyMap {
public:
    typedef std::pair<std::string, Property> Info;
    typedef std::vector<Info>::iterator Iterator;
    typedef std::vector<Info>::const_iterator CIterator;

    PropertyMap() = default;
    PropertyMap(std::initializer_list<Info> init);
    void insert(Info value);
    void insert(Iterator b, Iterator e);
    void clear();
    size_t size() const;
    bool empty() const;

    Property at(const std::string &name);
    const Property& at(const std::string &name) const;
    Property& operator[](const std::string &name);
    Property& operator[](std::string &&name);

    Iterator begin();
    Iterator end();
    CIterator cbegin() const;
    CIterator cend() const;

    Iterator find(const std::string& name);
    CIterator find(const std::string& name) const;

    void erase(const std::string& name);

private:
    std::vector<Info> _properties;
};

//typedef std::unordered_map<std::string, Property> PropertyMap;

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

        _readers[PropertyTypeInfo<T>::getType()] = reader;
    }
    static Property read(IO::InStream &stream) noexcept;

private:
    static ReaderList _readers;

};

}
} /* MindTree */
 MindTree::PropertyMap::Iterator begin(MindTree::PropertyMap&);
 MindTree::PropertyMap::Iterator end(MindTree::PropertyMap&);
 MindTree::PropertyMap::CIterator begin(const MindTree::PropertyMap&);
 MindTree::PropertyMap::CIterator end(const MindTree::PropertyMap&);

#endif /* end of include guard: PROPERTIES_K7LMQN2D */
