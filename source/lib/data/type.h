#ifndef MT_TYPEINFO_H
#define MT_TYPEINFO_H

#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <atomic>
#include <unordered_map>

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

    int id() const
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

    Type<T>& operator=(const Type<T> &t)
    {
        typeString = t.typeString;
        _id = getID(typeString);
        return *this;
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

    static int registerType(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(_id_map_mutex);
        id_map.push_back(name);
        return ++id_cnt;
    }

    static void unregisterType(const std::string &name)
    {
    }

    static int getID(const std::string &name)
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

template<typename T>
struct type_hash {
    size_t operator()(const T &type) const {
        return type.id();
    }
};

template<typename Type, typename Content>
using TypeDispatcher = std::unordered_map<Type, Content, type_hash<Type>>;

}

#endif
