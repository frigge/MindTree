#ifndef MT_TYPEINFO_H
#define MT_TYPEINFO_H

#include "string"
#include "vector"
#include "iostream"

namespace MindTree {

class TypeBase
{
public:
    TypeBase (const std::string &typeStr="undefined", int id=-1)
        : typeString(typeStr), _id(id)
    {
    }

    TypeBase (const char *typeStr="undefined", int id=-1)
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

    Type(const char *typeStr)
        : TypeBase(typeStr, getID(typeStr)) {}

    void operator=(const Type<T> &t)
    {
        typeString = t.typeString;
        _id = getID(typeString);
    }

    static Type byID(int id)
    {
        if(id >= id_map.size()) {
            std::cout<<"Type not registered: "<<id<<std::endl;
            return Type();
        }
        return Type(id_map[id]);
    }

    static std::vector<std::string> getTypes()
    {
        return id_map;
    }

    static int registerType(std::string name)
    {
        id_map.push_back(name);
        return ++id_cnt;
    }

    static void unregisterType(std::string name)
    {
    }

    static int getID(std::string name)
    {
        if(name == "") return -1;
        int i = 0;
        for(auto s = id_map.begin(); s != id_map.end(); s++, i++){
            if(name == *s) return i;
        }
        return registerType(name);
    }

private:
    static int id_cnt;
    static std::vector<std::string> id_map;
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
int Type<T>::id_cnt = -1;

template<typename T, typename Content>
class TypeDispatcher
{
public:
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
            add(type, Content());

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
