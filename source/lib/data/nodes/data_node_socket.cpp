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


#include "cassert"
#include "iostream"
#include "stdio.h"

#include "data/signal.h"
#include "data_node.h"

#include "data/debuglog.h"
#include "data_node_socket.h"

using namespace MindTree;

std::unordered_map<unsigned short, const DSocket*>LoadSocketIDMapper::loadIDMapper;
unsigned short DSocket::count = 0;
std::unordered_map<DSocket*, DSocket*> CopySocketMapper::socketMap;
std::unordered_map<unsigned short, DSocket*> DSocket::socketIDHash;

unsigned short LoadSocketIDMapper::getID(const DSocket *socket)
{
    for(auto p : loadIDMapper)
        if (p.second == socket) return p.first;
    return -1;
}

void LoadSocketIDMapper::setID(const DSocket *socket, unsigned short ID)
{
    loadIDMapper.insert({ID, socket});
}

const DSocket * LoadSocketIDMapper::getSocket(unsigned short ID)
{
    return loadIDMapper[ID];
}

void LoadSocketIDMapper::remap()
{
    for(auto p : loadIDMapper) {
        const DSocket *socket = p.second;
        if(!socket) continue;
        if(socket->getDir() == DSocket::IN)
            if(socket->toIn()->getTempCntdID() > 0)
                const_cast<DSocket*>(socket)->toIn()->cntdSocketFromID();
        }
    loadIDMapper.clear();
}

void CopySocketMapper::setSocketPair(DSocket *original, DSocket *copy)
{
   socketMap.insert({original, copy});
}

DSocket * CopySocketMapper::getCopy(const DSocket *original)
{
    auto* orig = const_cast<DSocket*>(original);
    if(socketMap.find(orig) == socketMap.end()) return nullptr;
    return socketMap[orig];
}

/** Loops through the socketmap and resets the connections.
 * If a connected socket is in the socketMap it means it is copied as well
 * so we link to  the copy. Otherwise we link to the original*/
void CopySocketMapper::remap()
{
    for(const auto p : socketMap) {
        //look in the socket map for all the insockets
        DSocket *socket = p.first;
        if(socket->getDir() == DSocket::IN) {
            DoutSocket *cntd = nullptr;
            //if this socket is connected and the connected was copied as well
            if((cntd = socket->toIn()->getCntdSocket())) {
                DinSocket *inCopy = const_cast<DSocket*>(socketMap[socket])->toIn();
                if(socketMap.find(cntd) != socketMap.end()) {
                    DoutSocket *outCopy = const_cast<DSocket*>(socketMap[cntd])->toOut();
                    inCopy->setCntdSocket(outCopy);
                }
                else
                    inCopy->setCntdSocket(const_cast<DoutSocket*>(cntd));
            }
        }
    }

    socketMap.clear();
}

IO::OutStream& MindTree::operator<<(IO::OutStream& stream, const DSocket &socket)
{
    stream << socket.getName()
        << socket.getID()
        << static_cast<const TypeBase&>(socket.getType())
        << socket.getVariable();
    return stream;
}

IO::InStream& MindTree::operator>>(IO::InStream& stream, DSocket &socket)
{
    std::string name, type;
    int id;
    bool variable;
    stream >> name >> id >> type >> variable;
    socket.name = name;
    socket.type = type;
    LoadSocketIDMapper::setID(&socket, id);
    return stream;
}

DSocket::DSocket(std::string name, SocketType type, DNode *node)
:   _signalLiveTime(this),
    name(name),
    type(type),
    node(node),
    variable(false),
    ID(++count),
    _propagateType([](SocketType t) { return t; })
{
    socketIDHash.insert({ID, this});
}

DSocket::DSocket(const DSocket& socket, DNode *node)
:   _signalLiveTime(this),
    name(socket.getName()),
    type(socket.getType()),
    node(node),
    variable(socket.getVariable()),
    ID(++count),
    _propagateType(socket._propagateType)
{
    CopySocketMapper::setSocketPair(const_cast<DSocket*>(&socket), this);
    socketIDHash.insert({ID, this});
}

DSocket::~DSocket()
{
    socketIDHash.erase(ID);
}

void DSocket::addChildNode(NodePtr child)
{
    _childNodes.push_back(child);
}

NodeList DSocket::getChildNodes() const
{
    return _childNodes;
}

bool DSocket::operator==(DSocket &socket)    const
{
    if(getVariable() != socket.getVariable()
        ||getName() != socket.getName()
        ||getType() != socket.getType())
        return false;
    return true;
}

bool DSocket::operator !=(DSocket &socket)const
{
    return(!(*this == socket));
}

DinSocket* DSocket::toIn()
{
    return static_cast<DinSocket*>(this);
}

const DinSocket* DSocket::toIn()const
{
    return static_cast<const DinSocket*>(this);
}

DoutSocket* DSocket::toOut()
{
    return static_cast<DoutSocket*>(this);
}

const DoutSocket* DSocket::toOut()const
{
    return static_cast<const DoutSocket*>(this);
}

void DSocket::createLink(DSocket *socket1, DSocket *socket2)
{
    DinSocket *in = 0;
    DoutSocket *out = 0;

    if(socket1->getDir() == IN)
    {
        in = socket1->toIn();
        out = socket2->toOut();
    }
    else
    {
        out = socket1->toOut();
        in = socket2->toIn();
    }
    if(in) {
        in->setCntdSocket(out);
    }
}

void DSocket::removeLink(DinSocket *in, DoutSocket *out)
{
    in->clearLink();
}

DSocket* DSocket::getSocket(unsigned short ID)
{
    return socketIDHash[ID];
}

bool DSocket::getVariable() const
{
	return variable;
}

void DSocket::setVariable(bool value)
{
	variable = value;
    if(getNode()&&variable)getNode()->setVarSocket(this);
}

unsigned short DSocket::getID() const
{
	return ID;
}

std::string DSocket::getIDName()
{
    return idName;
}

void DSocket::setIDName(std::string value)
{
    idName = value;
}

void DSocket::setNode(DNode *node)
{
    this->node = node;
    if(getVariable())node->setVarSocket(this);
}

std::string DSocket::getName() const
{
	return name;
}

void DSocket::setName(std::string value)
{
    if(value == name) return;
	name = value;
    MT_CUSTOM_BOUND_SIGNAL_EMITTER(&_signalLiveTime, "nameChanged", value);
}

const SocketType& DSocket::getType() const
{
    std::lock_guard<std::mutex> lock(_typeLock);
	return type;
}

void DSocket::setType(SocketType value)
{
    {
        if(value == type) return;
        std::lock_guard<std::mutex> lock(_typeLock);
        type = value;
    }

    MT_CUSTOM_BOUND_SIGNAL_EMITTER(&_signalLiveTime, "typeChanged", value);
}

void DSocket::setTypePropagationFunction(std::function<SocketType(SocketType)> fn)
{
    _propagateType = fn;
}

void DSocket::listenToNameChange(DSocket *other)
{
    auto cb = Signal::getBoundHandler<std::string>(other)
        .connect("nameChanged", [this] (std::string newName) {
                   this->setName(newName);
                 });
    _callbacks.push_back(cb);
}

void DSocket::listenToTypeChange(DSocket *other)
{
    auto cb = Signal::getBoundHandler<SocketType>(other)
        .connect("typeChanged", [this] (SocketType newType) {
                   this->setType(this->_propagateType(newType));
                 });
    _callbacks.push_back(cb);
}

void DSocket::listenToChange(DSocket *other)
{
    listenToNameChange(other);
    listenToTypeChange(other);
}

DSocket::SocketDir DSocket::getDir() const
{
	return dir;
}

void DSocket::setDir(DSocket::SocketDir value)
{
	dir = value;
}

DNode* DSocket::getNode() const
{
	return node;
}

IO::OutStream& MindTree::operator<<(IO::OutStream &stream, const DinSocket &socket)
{
    stream << static_cast<const DSocket&>(socket);
    int cntdID = -1;
    if(socket.getCntdSocket())
        cntdID = socket.getCntdSocket()->getID();
    else
        cntdID = -1;

    stream << cntdID;
    auto prop = socket.getProperty();
    stream << prop;
    return stream;
}

IO::InStream& MindTree::operator>>(IO::InStream& stream, DinSocket &socket)
{
    stream >> static_cast<DSocket&>(socket);

    int cntdID = -1;
    Property prop;
    stream >> cntdID >> prop;
    socket.prop = prop;

    socket.setTempCntdID(cntdID);

    return stream;
}

DinSocket::DinSocket(std::string name, SocketType type, DNode *node)
:   DSocket(name, type, node),
    tempCntdID(0),
    cntdSocket(nullptr)
{
	setDir(IN);
    getNode()->addSocket(this);
    createChildNodes();
};

DinSocket::DinSocket(const DinSocket& socket, DNode *node)
:   DSocket(socket, node),
    tempCntdID(0),
    cntdSocket(socket.getCntdSocket()),
    prop(socket.prop)
{
    setDir(IN);
    getNode()->addSocket(this);

    for(auto child : socket.getChildNodes()) {
        addChildNode(std::make_shared<DNode>(*child));
    }
}

DinSocket::~DinSocket()
{
    if(cntdSocket)
        cntdSocket->unregisterSocket(this);
}

void DinSocket::addChildNode(NodePtr child)
{
    DSocket::addChildNode(child);
    for (auto *in : child->getInSockets()) {
        auto cb = Signal::getBoundHandler<DoutSocket*>(in)
            .connect("linkChanged", [this, child](DoutSocket *out) {
                    this->setCntdSocket(child->getOutSockets()[0]);
            });
        _callbacks.push_back(cb);
    }
}

void DinSocket::createChildNodes()
{
    for (auto *factory : NodeDataBase::getConverters(getType())) {
        addChildNode((*factory)(false));
    }
}


Property DinSocket::getProperty()const
{
    std::lock_guard<std::mutex> lock(_propLock);
    return prop;
}

void DinSocket::setProperty(Property property)
{
    {
        std::lock_guard<std::mutex> lock(_propLock);
        prop = property;
    }
    setType(prop.getType());
    MT_CUSTOM_SIGNAL_EMITTER("socketChanged", this);
}

void DinSocket::addLink(DoutSocket *socket)
{
    if(cntdSocket == socket)
        return;

    //here we set the actual link
    setCntdSocket(socket);
    if (getVariable())
        getNode()->incVarSocket();
}

void DinSocket::clearLink()
{
    if(cntdSocket)cntdSocket->unregisterSocket(this);
	cntdSocket = nullptr;

    if(getVariable())
        getNode()->decVarSocket(this);
}

unsigned short DinSocket::getTempCntdID() const
{
	return tempCntdID;
}

void DinSocket::setTempCntdID(unsigned short value)
{
	tempCntdID = value;
}

bool DinSocket::operator==(DinSocket &socket)const
{
    if(DSocket::operator!=(socket))
        return false;
    return true;
}

bool DinSocket::operator !=(DinSocket &socket)const
{
    return(!(*this == socket));
}

void DinSocket::listenToLinkedName()
{
    auto cb = Signal::getBoundHandler<DoutSocket*>(this)
        .connect("linkChanged", [this] (DoutSocket *socket) {
                    this->listenToNameChange(socket);
                    this->setName(socket->getName());
                 });
    _linkedNameChangeCallback = cb;
}

void DinSocket::listenToLinkedType()
{
    auto cb = Signal::getBoundHandler<DoutSocket*>(this)
        .connect("linkChanged", [this] (DoutSocket *socket) {
                    this->listenToTypeChange(socket);
                    this->setType(socket->getType());
                 });
    _linkedTypeChangeCallback = cb;
}

void DinSocket::listenToLinked()
{
    auto cb = Signal::getBoundHandler<DoutSocket*>(this)
        .connect("linkChanged", [this] (DoutSocket *socket) {
                    this->listenToChange(socket);
                    this->setType(socket->getType());
                    this->setName(socket->getName());
                 });
    _linkedChangeCallback = cb;
}

void DinSocket::setCntdSocket(DoutSocket *socket)
{
    MT_CUSTOM_BOUND_SIGNAL_EMITTER(&_signalLiveTime, "linkChanged", socket);
    if(!socket) {
        clearLink();
        return;
    }
    if(cntdSocket == socket)
        return;

    //unregister the old one
    if(cntdSocket) cntdSocket->unregisterSocket(this, false);

    //here we set the actual link
	cntdSocket = socket;
    cntdSocket->registerSocket(this);
    if (getVariable())
        getNode()->incVarSocket();

    MT_CUSTOM_SIGNAL_EMITTER("createLink", this);
}

void DinSocket::cntdSocketFromID()
{
    cntdSocket = const_cast<DSocket*>(LoadSocketIDMapper::getSocket(getTempCntdID()))->toOut();
    if(cntdSocket) cntdSocket->pushSocket(this);
    setTempCntdID(0);
}

const DoutSocket* DinSocket::getCntdSocketConst() const
{
	return cntdSocket;
}

DoutSocket* DinSocket::getCntdSocket() const
{
	return cntdSocket;
}

DoutSocket::DoutSocket(std::string name, SocketType type, DNode *node)
	: DSocket(name, type, node)
{
	setDir(OUT);
    getNode()->addSocket(this);
}

DoutSocket::DoutSocket(const DoutSocket& socket, DNode *node)
    : DSocket(socket, node)
{
    setDir(OUT);
    getNode()->addSocket(this);
}

DoutSocket::~DoutSocket()
{
    for(DinSocket *socket : cntdSockets)
        socket->clearLink();
}

bool DoutSocket::operator==(DoutSocket &socket)const
{
    if(DSocket::operator!=(socket))
        return false;
    return true;
}

bool DoutSocket::operator !=(DoutSocket &socket)const
{
    return(!(*this == socket));
}

void DoutSocket::registerSocket(DinSocket *socket)
{
    MT_CUSTOM_BOUND_SIGNAL_EMITTER(&_signalLiveTime, "outLinkChanged", socket);
    if(this == getNode()->getVarSocket())
        getNode()->incVarSocket();
    
    pushSocket(socket);
}

void DoutSocket::pushSocket(DinSocket *socket)
{
    if(std::find(cntdSockets.begin(),
                 cntdSockets.end(),
                 socket->toIn()) == cntdSockets.end())
        cntdSockets.push_back(socket->toIn());
}

std::vector<DinSocket*> DoutSocket::getCntdSockets() const
{
    return cntdSockets;
}

void DoutSocket::unregisterSocket(DinSocket *socket, bool decr)
{
    MT_CUSTOM_BOUND_SIGNAL_EMITTER(&_signalLiveTime, "outLinkChanged", socket);
    auto it = std::find(cntdSockets.begin(), cntdSockets.end(), socket);
    if(it != end(cntdSockets)) {
        cntdSockets.erase(it);
    }
    if(cntdSockets.size() == 0 && getVariable() && decr)
        getNode()->decVarSocket(this);
}

void DoutSocket::listenToLinkedName()
{
    auto cb = Signal::getBoundHandler<DoutSocket*>(this)
        .connect("outLinkChanged", [this] (DoutSocket *socket) {
                    this->listenToNameChange(socket);
                    this->setName(socket->getName());
                 });
    _linkedNameChangeCallback = cb;
}

void DoutSocket::listenToLinkedType()
{
    auto cb = Signal::getBoundHandler<DinSocket*>(this)
        .connect("outLinkChanged", [this] (DinSocket *socket) {
                    this->listenToTypeChange(socket);
                    this->setType(socket->getType());
                 });
    _linkedTypeChangeCallback = cb;
}

void DoutSocket::listenToLinked()
{
    auto cb = Signal::getBoundHandler<DinSocket*>(this)
        .connect("outLinkChanged", [this] (DinSocket *socket) {
                    this->listenToChange(socket);
                    this->setType(socket->getType());
                    this->setName(socket->getName());
                 });
    _linkedChangeCallback = cb;
}

