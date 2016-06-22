#ifndef MT_TYPEINFO_H
#define MT_TYPEINFO_H

#include <string>
#include <functional>
#include <unordered_map>

namespace MindTree {

class TypeBase
{
public:
    TypeBase (const std::string &typeStr="undefined")
        : typeString(typeStr), _id(std::hash<std::string>()(typeStr))
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
    size_t _id;
};

template<typename T>
class Type : public TypeBase
{
public:
    Type(const std::string &typeStr="type_undefined")
        : TypeBase(typeStr)
    { }

    Type<T>& operator=(const Type<T> &t) = default;
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
struct type_hash {
    size_t operator()(const T &type) const {
        return type.id();
    }
};

template<typename Type, typename Content>
using TypeDispatcher = std::unordered_map<Type, Content, type_hash<Type>>;

}

#endif
