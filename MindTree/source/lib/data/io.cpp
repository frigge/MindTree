#include "iostream"
#include "cstdint"
#include "array"
#include "cassert"

#include "data/dnspace.h"
#include "data/nodes/containernode.h"
#include "data/nodes/data_node.h"

#include "io.h"

using namespace MindTree::IO;

MindTree::TypeDispatcher<MindTree::NodeType, std::function<void(OutStream&, const void*)>> 
    OutStream::_nodeStreamDispatcher;

OutStream::OutStream(std::string filename)
    : _stream(filename, std::ios::binary)
{
    auto container = _nodeStreamDispatcher["CONTAINER"];
    if(!container) {
        _nodeStreamDispatcher.add("CONTAINER", dispatchedOutStreamer<ContainerNode>);
    }
}

OutStream::~OutStream()
{
    _stream.close();
    assert(_blockStack.empty());
}

void OutStream::beginBlock(std::string blockName)
{
    _blockStack.push(std::vector<char>());
    *this << blockName;
}

void OutStream::endBlock(std::string blockName)
{
    std::vector<char> currentBlock = _blockStack.top();

    int32_t size = currentBlock.size() + 4;
    auto sizeVec = std::vector<char>();
    char* output = reinterpret_cast<char*>(&size);
    for(size_t i = 0; i < sizeof(size); ++i) {
        sizeVec.emplace_back(output[i]);
    }
    currentBlock.insert(begin(currentBlock), begin(sizeVec), end(sizeVec));

    _blockStack.pop();
    
    if(!_blockStack.empty()) {
        write(currentBlock.data(), currentBlock.size());
    }
    else {
        _stream.write(currentBlock.data(), currentBlock.size());
    }
}

void OutStream::write(const char* value, size_t size)
{
    auto& currentBlock = _blockStack.top();
    for(size_t i = 0; i<size; ++i) {
        currentBlock.emplace_back(value[i]);
    }
}

OutStream& OutStream::operator<<(int number)
{
    if(!_stream.is_open()) {
        std::cout << "file is not open" << std::endl;
        return *this;
    }

    int32_t num = number;
    const char* output = reinterpret_cast<const char*>(&num);
    size_t size = sizeof(num);
    write(output, size);
    return *this;
}

OutStream& OutStream::operator<<(size_t number)
{
    return *this << static_cast<int>(number);
}

OutStream& OutStream::operator<<(unsigned short number)
{
    return *this << static_cast<int>(number);
}

OutStream& OutStream::operator<<(bool value)
{
    if(!_stream.is_open()) {
        std::cout << "file is not open" << std::endl;
        return *this;
    }

    int8_t num = value ? 1 : 0;
    const char* output = reinterpret_cast<const char*>(&num);
    write(output, sizeof(num));
    return *this;
}

OutStream& OutStream::operator<<(std::string str)
{
    if(!_stream.is_open())  return *this;

    write(str.c_str(), str.size() + 1);
    return *this;
}

OutStream& OutStream::operator<<(double value)
{
    if(!_stream.is_open()) return *this;

    write(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
}

OutStream& OutStream::operator<<(const TypeBase &t)
{
    *this << t.toStr();
    return *this;
}

OutStream& OutStream::operator<<(const DNode &node)
{
    beginBlock("DNode");
    NodeType type = node.getType();
    *this << static_cast<const TypeBase&>(type);
    *this << node.getNodeName();

    int incnt = node.getInSockets().size();
    *this << incnt;
    for (const auto *in : node.getInSockets()) {
        beginBlock("DinSocket");
        *this << *in;
        endBlock("DinSocket");
    }

    *this << node.getOutSockets().size();
    for (const DSocket *out : node.getOutSockets()) {
        beginBlock("DoutSocket");
        *this << *out;
        endBlock("DoutSocket");
    }

    auto derived = _nodeStreamDispatcher[type];
    if (derived) _nodeStreamDispatcher[type](*this, &node);

    endBlock("DNode");
    return *this;
}

OutStream& OutStream::operator<<(ContainerNode &node)
{
    auto *space = new ContainerSpace();
    space->setName(node.getNodeName());

    node.setContainerData(space);
    *this << space;
}

InStream::InStream(std::string filename)
    : _stream(filename, std::ios::binary)
{
}

void InStream::beginBlock(std::string blockName)
{
    std::cout << "beginning block: " << blockName << std::endl;
    _blocks.push(BlockInfo());
    auto &info = _blocks.top();
    _stream.read(reinterpret_cast<char*>(&info.size), sizeof(info.size));
    info.pos += 4;

    std::string name;
    *this >> name;
    assert(blockName == name);
}

void InStream::endBlock(std::string blockName)
{
    BlockInfo currentBlock = _blocks.top();
    int32_t remaining_bytes = currentBlock.size - currentBlock.pos;

    auto block = std::vector<char>(remaining_bytes);
    
    read(block.data(), remaining_bytes);
    finishBlock();
}

void InStream::finishBlock()
{
    BlockInfo lastBlock = _blocks.top();
    _blocks.pop();

    if(!_blocks.empty())
        _blocks.top().pos += lastBlock.size;
}

void InStream::read(char* val, size_t size)
{
    _stream.read(val, size);

    if(!_blocks.empty()) {
        auto &currentBlock = _blocks.top();
        currentBlock.pos += size;

        assert(!(currentBlock.pos > currentBlock.size));
    }
}

InStream& InStream::operator>>(int8_t &number)
{
    int32_t num;
    *this >> num;
    number = num;
    return *this;
}

InStream& InStream::operator>>(int16_t &number)
{
    int32_t num;
    *this >> num;
    number = num;
    return *this;
}

InStream& InStream::operator>>(int32_t &number)
{
    read(reinterpret_cast<char*>(&number), sizeof(number));
    return *this;
}

InStream& InStream::operator>>(int64_t &number)
{
    int32_t num;
    *this >> num;
    number = num;
    return *this;
}

InStream& InStream::operator>>(uint8_t &number)
{
    int32_t num;
    *this >> num;
    number = num;
    return *this;
}

InStream& InStream::operator>>(uint16_t &number)
{
    int32_t num;
    *this >> num;
    number = num;
    return *this;
}

InStream& InStream::operator>>(uint32_t &number)
{
    int32_t num;
    *this >> num;
    number = num;
    return *this;
}

InStream& InStream::operator>>(uint64_t &number)
{
    int32_t num;
    *this >> num;
    number = num;
    return *this;
}

InStream& InStream::operator>>(std::string &str)
{
    char ch;
    read(&ch, 1);
    while(ch != '\0') {
        str.push_back(ch);
        read(&ch, 1);
    }
    return *this;
}

InStream& InStream::operator>>(bool &val)
{
    int8_t num;
    read(reinterpret_cast<char*>(&num), 1);
    val = num;
    return *this;
}

InStream& InStream::operator>>(double &value)
{
    read(reinterpret_cast<char*>(&value), sizeof(double));
    return *this;
}

InStream& InStream::operator>>(NodeType &t)
{
    std::string str;
    *this >> str;
    t = NodeType(str);
    return *this;
}

InStream& InStream::operator>>(DataType &t)
{
    std::string str;
    *this >> str;
    t = DataType(str);
    return *this;
}

InStream& InStream::operator>>(DNode &node)
{
    std::string name;
    *this >> name;
    node.setNodeName(name);
    int insockets = 0;
    *this >> insockets;
    for(int i = 0; i < insockets; ++i) {
        beginBlock("DinSocket");
        auto *socket = new DinSocket("", "", &node);
        *this >> *socket;
        endBlock("DinSocket");
    }

    int outsockets = 0;
    *this >> outsockets;
    for(int i = 0; i < outsockets; ++i) {
        beginBlock("DoutSocket");
        DSocket *socket = new DoutSocket("", "", &node);
        *this >> *socket;
        endBlock("DoutSocket");
    }
    return *this;
}

