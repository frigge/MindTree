#ifndef MT_IO_H
#define MT_IO_H

#include "fstream"
#include "iostream"
#include "vector"
#include "stack"
#include "data/type.h"
#include "functional"
#include "data/nodes/nodetype.h"

namespace MindTree {
class DNode;
class ContainerNode;

namespace IO {

class OutStream 
{
public:
    OutStream(std::string filename);
    virtual ~OutStream();

    OutStream& operator<<(int number);
    OutStream& operator<<(size_t number);
    OutStream& operator<<(std::string str);
    OutStream& operator<<(unsigned short number);
    OutStream& operator<<(bool value);
    OutStream& operator<<(double value);

    OutStream& operator<<(const TypeBase &t);
    OutStream& operator<<(const DNode &node);
    OutStream& operator<<(ContainerNode &node);

    void beginBlock(std::string blockName="");
    void endBlock(std::string blockName="");

private:
    void write(const char* value, size_t size);

    std::ofstream _stream;
    std::stack<std::vector<char>> _blockStack;

    static TypeDispatcher<NodeType, std::function<void(OutStream&, const void*)>> 
        _nodeStreamDispatcher;
};

template<typename T>
void dispatchedOutStreamer(OutStream &stream, const void* data)
{
    stream << *reinterpret_cast<const T*>(data);
}


class InStream 
{
public:
    struct BlockInfo {
        BlockInfo() : size(0), pos(0) {}
        int32_t size, pos;
    };

    InStream(std::string filename);

    InStream& operator>>(int8_t &number);
    InStream& operator>>(int16_t &number);
    InStream& operator>>(int32_t &number);
    InStream& operator>>(int64_t &number);

    InStream& operator>>(uint8_t &number);
    InStream& operator>>(uint16_t &number);
    InStream& operator>>(uint32_t &number);
    InStream& operator>>(uint64_t &number);

    InStream& operator>>(std::string &str);

    InStream& operator>>(bool &val);
    InStream& operator>>(double &value);

    InStream& operator>>(DataType &t);
    InStream& operator>>(NodeType &t);
    InStream& operator>>(DNode &node);

    void beginBlock(std::string blockName="");
    void endBlock(std::string blockName="");

private:
    void finishBlock();
    void read(char* val, size_t size);

    std::ifstream _stream;
    std::stack<BlockInfo> _blocks;
};
}
template<typename T>
IO::OutStream& operator<<(IO::OutStream& stream, const T &data) 
{
    std::cout << "Type does not support serialization" << std::endl;
    return stream;
}

template<typename T>
IO::InStream& operator>>(IO::InStream& stream, T &data) 
{
    std::cout << "Type does not support serialization" << std::endl;
    return stream;
}
}

#endif
