#ifndef MT_TYPEINFO_H
#define MT_TYPEINFO_H

#include "string"
#include "vector"
#include "iostream"
#include "mutex"
#include "atomic"

namespace MindTree {

class TypeBase
{
public:
    TypeBase (const std::string &typeStr="undefined", int id=-1)
        : typeString(typeStr), _id(id)
    {
    }

    std::string toStr()const
    {
        return typeString;
    }

    int id()const
    {
        return _id;
    }

    bool operator==(const TypeBase &other)const
    {
        return other._id == _id;
    }

    bool operator!=(const TypeBase &other)const
    {
        return !((*this)==other);
    }

    bool operator==(const char *str)const
    {
        return typeString == std::string(str);
    }

    bool operator!=(const char *str)const
    {
        return !((*this)==str);
    }

    bool operator==(const std::string &str)const
    {
        return typeString == str;
    }

    bool operator!=(const std::string &str)const
    {
        return !((*this)==str);
    }

protected:
    std::string typeString;
    int _id;
};

template<typename T>
class Type : public TypeBase
{
public:
    Type(const std::string &typeStr="undefined")
        : TypeBase(typeStr, getID(typeStr))
    { }

    void operator=(const Type<T> &t)
    {
        typeString = t.typeString;
        _id = getID(typeString);
    }

    static Type byID(int id)
    {
        int size = 0;
        {
            std::lock_guard<std::mutex> lock(_id_map_mutex);
            size = id_map.size();
        }

        if(id >= size) {
            std::cout<<"Type not registered: "<<id<<std::endl;
            return Type();
        }
        {
            std::lock_guard<std::mutex> lock(_id_map_mutex);
            return Type(id_map[id]);
        }
    }

    static std::vector<std::string> getTypes()
    {
        std::lock_guard<std::mutex> lock(_id_map_mutex);
        return id_map;
    }

    static int registerType(std::string name)
    {
        std::lock_guard<std::mutex> lock(_id_map_mutex);
        id_map.push_back(name);
        return ++id_cnt;
    }

    static void unregisterType(std::string name)
    {
    }

    static int getID(std::string name)
    {
        if(name.empty()) return -1;
        {
            std::lock_guard<std::mutex> lock(_id_map_mutex);
            int i = 0;
            for(auto s = id_map.begin(); s != id_map.end(); s++, i++){
                if(name == *s) return i;
            }
        }
        return registerType(name);
    }

private:
    static std::atomic<int> id_cnt;
    static std::vector<std::string> id_map;
    static std::mutex _id_map_mutex;
};

class DataType : public Type<DataType>
{
public:
    DataType(const std::string &typeStr="")
        : Type<DataType>(typeStr)
    {}

    DataType(const char *typeStr)
        : Type<DataType>(typeStr)
    {}
};

template<typename T>
std::vector<std::string> Type<T>::id_map;

template<typename T>
std::mutex Type<T>::_id_map_mutex;

template<typename T>
std::atomic<int> Type<T>::id_cnt{-1};

template<typename T, typename Content>
class TypeDispatcher
{
public:
    TypeDispatcher()
    {
    }

    void add(T type, Content c)
    {
        int typeID = type.id();
        if(typeID >= _contentList.size()) {
            _contentList.resize(typeID + 1);
        }

        _contentList[typeID] = c;
    }

    const std::vector<Content>& getAll()
    {
        return _contentList;
    }

    Content* get(T type)
    {
        if (type.id() < _contentList.size())
            return &_contentList[type.id()];
        else
            return nullptr;
    }

    Content& operator[](T type)
    {
        if (type.id() >= _contentList.size())
            _contentList.resize(type.id() + 1);

        return _contentList[type.id()];
    }

    size_t size() const
    {
        return _contentList.size();
    }

private:
    std::vector<Content> _contentList;
};
}

#endif
