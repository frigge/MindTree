#include "iostream"
#include "cstdint"
#include "array"
#include "cassert"

#include "data/dnspace.h"
#include "data/datatypes.h"
#include "data/nodes/containernode.h"
#include "data/nodes/data_node.h"
#include "data/debuglog.h"

#include "io.h"

//#define DEBUG_IO

using namespace MindTree::IO;

MindTree::TypeDispatcher<MindTree::NodeType, std::function<void(OutStream&, const void*)>> 
    OutStream::_nodeStreamDispatcher;

OutStream::OutStream(std::string filename)
    : _stream(filename, std::ios::binary)
{
    auto container = _nodeStreamDispatcher["CONTAINER"];
    if(!container) {
        _nodeStreamDispatcher["CONTAINER"] = dispatchedOutStreamer<ContainerNode>;
    }
}

OutStream::~OutStream()
{
    _stream.close();
    assert(_blockStack.empty());
}

void OutStream::beginBlock(std::string blockName)
{
#ifdef DEBUG_IO
    std::string indent(_blockStack.size() * 2, ' ');
    std::cout << indent << "begin block: " << blockName << std::endl;
#endif
    _blockStack.push(std::vector<char>());
    *this << std::string("BLOCK:") + blockName;
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

#ifdef DEBUG_IO
    std::string indent(_blockStack.size() * 2, ' ');
    std::cout << indent << "end block: " << blockName << "\n"
              << indent << "block size: " << currentBlock.size() << std::endl;
#endif
    
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

    char num = value;
    write(&num, 1);
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

OutStream& OutStream::operator<<(const Vec2i &point)
{
    *this << point.x() << point.y();
    return *this;
}

OutStream& OutStream::operator<<(const glm::ivec2 &vec)
{
    *this << vec.x << vec.y;
    return *this;
}

OutStream& OutStream::operator<<(const glm::vec2 &vec)
{
    *this << static_cast<double>(vec.x) << static_cast<double>(vec.y);
    return *this;
}

OutStream& OutStream::operator<<(const glm::vec3 &vec)
{
    double x = vec.x;
    double y = vec.y;
    double z = vec.z;
    *this << x << y << z;
    return *this;
}

OutStream& OutStream::operator<<(const glm::vec4 &vec)
{
    double x = vec.x;
    double y = vec.y;
    double z = vec.z;
    double w = vec.w;
    *this << x << y << z << w;
    return *this;
}

OutStream& OutStream::operator<<(const DNode &node)
{
    beginBlock("DNode");
    NodeType type = node.getType();
    *this << static_cast<const TypeBase&>(type);
    *this << node.getNodeName();
    *this << node.getPos();

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

OutStream& OutStream::operator<<(const ContainerNode &node)
{
    auto *space = node.getContainerData();
    *this << static_cast<const DNSpace&>(*space);
    return *this;
}

MindTree::TypeDispatcher<MindTree::NodeType, std::function<void(InStream&, void*)>> 
    InStream::_nodeStreamDispatcher;

InStream::InStream(std::string filename)
    : _stream(filename, std::ios::binary)
{
    auto container = _nodeStreamDispatcher["CONTAINER"];
    if(!container) {
        _nodeStreamDispatcher["CONTAINER"] = dispatchedInStreamer<ContainerNode>;
    }
}

void InStream::beginBlock(std::string blockName)
{
    _blocks.push(BlockInfo());
    auto &info = _blocks.top();
    _stream.read(reinterpret_cast<char*>(&info.size), sizeof(info.size));
    info.pos += 4;

#ifdef DEBUG_IO
    std::string indent(_blocks.size() * 2, ' ');
    std::cout << indent << "reading block: " << blockName << " size: " << info.size << std::endl;
#endif

    if(info.size < 4) {
        std::cerr
#ifdef DEBUG_IO
            << indent
#endif
            << "ERROR: corrupt block, skipping over" << std::endl;
    }

    *this >> info.name;

    if("BLOCK:" + blockName != info.name)
        std::cout << blockName << " and " << info.name << " don't match" << std::endl;
    assert("BLOCK:" + blockName == info.name);
}

void InStream::endBlock(std::string blockName)
{
    BlockInfo currentBlock = _blocks.top();
#ifdef DEBUG_IO
    std::string indent(_blocks.size() * 2, ' ');
    std::cout << indent << "end block: " << blockName << std::endl;
#endif
    int32_t remaining_bytes = currentBlock.size - currentBlock.pos;

    auto block = std::vector<char>(remaining_bytes);
    
    if(remaining_bytes) {
        std::cout << "corrupted block: " << currentBlock.name 
            << "; " << remaining_bytes 
            << " bytes were not read from this block" << std::endl;
    }
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
    if(!_blocks.empty()) {
        auto &currentBlock = _blocks.top();
        if(currentBlock.pos + size > currentBlock.size) {
            std::cout << "corrupted block; trying to read "
                << size << " bytes from a block where only "
                << currentBlock.size - currentBlock.pos << " bytes are left" << std::endl;
            endBlock();
        }
        currentBlock.pos += size;
    }
    _stream.read(val, size);
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

    //#ifdef DEBUG_IO
    //    std::string indent(_blocks.size() * 2, ' ');
    //    std::cout << indent << "string: " << str << std::endl;
    //#endif
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

InStream& InStream::operator>>(Vec2i &point)
{
    int x, y;
    *this >> x >> y;
    point = Vec2i(x, y);
    return *this;
}

InStream& InStream::operator>>(glm::ivec2 &vec)
{
    *this >> vec.x >> vec.y;
    return *this;
}

InStream& InStream::operator>>(glm::vec2 &vec)
{
    double x, y;
    *this >> x >> y;
    vec.x = x;
    vec.y = y;
    return *this;
}

InStream& InStream::operator>>(glm::vec3 &vec)
{
    double x, y, z;
    *this >> x >> y >> z;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    return *this;
}

InStream& InStream::operator>>(glm::vec4 &vec)
{
    double x, y, z, w;
    *this >> x >> y >> z >> w;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    vec.w = w;
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
    node.setName(name);
    Vec2i vec;
    *this >> vec;
    node.setPos(vec);
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

    auto type = node.getType();
    auto derived = _nodeStreamDispatcher[type];
    if (derived) _nodeStreamDispatcher[type](*this, &node);
    return *this;
}

InStream& InStream::operator>>(ContainerNode &node)
{
    auto *space = node.getContainerData();
    space->setName(node.getNodeName());

    auto in = std::make_shared<SocketNode>(DSocket::IN, &node, true);
    node.setInSockets(in.get());
    node.getContainerData()->addNode(in);
    auto out = std::make_shared<SocketNode>(DSocket::OUT, &node, true);
    node.setOutSockets(out.get());
    node.getContainerData()->addNode(out);

    *this >> static_cast<DNSpace&>(*space);
    return *this;
}

