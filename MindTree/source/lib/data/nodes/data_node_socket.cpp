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

#include "data_node_socket.h"

using namespace MindTree;

std::unordered_map<unsigned short, const DSocket*>LoadSocketIDMapper::loadIDMapper;
unsigned short DSocket::count = 0;
std::unordered_map<const DSocket*, const DSocket*> CopySocketMapper::socketMap;
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
        if(socket->getDir() == DSocket::IN)
            if(socket->toIn()->getTempCntdID() > 0)
                const_cast<DSocket*>(socket)->toIn()->cntdSocketFromID();
        }
    loadIDMapper.clear();
}

void CopySocketMapper::setSocketPair(const DSocket *original, const DSocket *copy)    
{
   socketMap.insert({original, copy}); 
}

const DSocket * CopySocketMapper::getCopy(const DSocket *original)    
{
    if(socketMap.find(original) == socketMap.end()) return 0;
    return socketMap[original];
}

/** Loops through the socketmap and resets the connections.
 * If a connected socket is in the socketMap it means it is copied as well
 * so we link to  the copy. Otherwise we link to the original*/
void CopySocketMapper::remap()    
{
    for(const auto p : socketMap) {
        //look in the socket map for all the insockets
        const DSocket *socket = p.first;
        if(socket->getDir() == DSocket::IN) {
            const DoutSocket *cntd = nullptr;
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

void IO::write(std::ostream& stream, const DSocket *socket)
{
    stream.write(socket->name.c_str(), socket->name.size());
    stream.write(reinterpret_cast<char*>(socket->ID), sizeof(short));
    stream.write(socket->type.toStr().c_str(), socket->type.toStr().size());
    stream.write(reinterpret_cast<char*>(socket->variable), sizeof(bool));

    if(socket->getDir() == DSocket::IN)
        IO::write(stream, socket->toIn());
}

DSocket::DSocket(std::string name, SocketType type, DNode *node)
:   name(name),
    type(type),
    node(node),
    variable(false),
    ID(++count)
{
    socketIDHash.insert({ID, this});
}

DSocket::DSocket(const DSocket& socket, DNode *node)
:   name(socket.getName()), 
    type(socket.getType()), 
    node(node),
    variable(socket.getVariable()),
    ID(++count)
{
    CopySocketMapper::setSocketPair(&socket, this);
    socketIDHash.insert({ID, this});
}

DSocket::~DSocket()
{
    std::cout << "delete DinSocket" << std::endl;
    socketIDHash.erase(ID);
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

bool DSocket::isCompatible(DSocket *s1, DSocket *s2)    
{
    if(s1 == s2)
        return false;
    if(s1->getNode() == s2->getNode())
        return false;
    return isCompatible(s1->getType(), s2->getType());
}

bool DSocket::isCompatible(SocketType s1, SocketType s2)    
{
    return true;
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

void IO::write(std::ostream &stream, const DinSocket *socket)
{
    if(socket->cntdSocket)
        stream.write(reinterpret_cast<char*>(socket->cntdSocket->ID),
                     sizeof(short));
    else
        stream.write(reinterpret_cast<char*>((short)-1), sizeof(short));

    IO::writeProperty(stream, socket->prop);
}

DinSocket::DinSocket(std::string name, SocketType type, DNode *node)
:   DSocket(name, type, node), 
    tempCntdID(0),
    cntdSocket(nullptr)
{
	setDir(IN);
    getNode()->addSocket(this);
};

DinSocket::DinSocket(const DinSocket& socket, DNode *node)
:   DSocket(socket, node),
    tempCntdID(0),
    cntdSocket(socket.getCntdSocket()),
    prop(socket.prop)
{
    setDir(IN);
    getNode()->addSocket(this);
}

DinSocket::~DinSocket()
{
    std::cout << "delete DinSocket" << std::endl;
    if(cntdSocket)
        cntdSocket->unregisterSocket(this);
}

Property DinSocket::getProperty()const
{
    return prop;
}

void DinSocket::setProperty(Property property)    
{
    prop = property;
    MT_CUSTOM_SIGNAL_EMITTER("socketChanged", this);
}

void DinSocket::addLink(DoutSocket *socket)
{
    if(cntdSocket == socket)
        return;

    if(!isCompatible(this, socket))
        return;

    //here we set the actual link
    setCntdSocket(socket);
    if (getVariable())
        getNode()->inc_var_socket();
}

void DinSocket::clearLink()
{
    if(cntdSocket)cntdSocket->unregisterSocket(this);
	cntdSocket = nullptr;

    if(getVariable())
        getNode()->dec_var_socket(this);
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
}

SocketType DSocket::getType() const
{
	return type;
}

void DSocket::setType(SocketType value)
{
    if(value == type) return;
	type = value;
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
    std::cout << "delete DinSocket" << std::endl;
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

void DoutSocket::registerSocket(DSocket *socket)    
{
    if(this == getNode()->getVarSocket()) 
        getNode()->inc_var_socket();

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
    auto it = std::find(cntdSockets.begin(), cntdSockets.end(), socket);
    cntdSockets.erase(it); 
    if(cntdSockets.size() == 0 && getVariable() && decr) 
        getNode()->dec_var_socket(this);
}

void DinSocket::setCntdSocket(DoutSocket *socket)
{
    if(!socket) {
        clearLink();
        return;
    }
    if(cntdSocket == socket)
        return;

    //here we set the actual link
    if(cntdSocket) cntdSocket->unregisterSocket(this, false);

	cntdSocket = socket;
    cntdSocket->registerSocket(this);
    if (getVariable())
        getNode()->inc_var_socket();

    MT_SIGNAL_EMITTER(this);
    MT_CUSTOM_SIGNAL_EMITTER("createLink", this);
}

void DinSocket::cntdSocketFromID()
{
    cntdSocket = const_cast<DSocket*>(LoadSocketIDMapper::getSocket(getTempCntdID()))->toOut();
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

DoutSocket* DinSocket::getCntdFunctionalSocket() const
{
        if(!cntdSocket)
            return nullptr;

        DNode *cntdNode = cntdSocket->getNode();
        if(cntdNode->isContainer())
        {
            return cntdNode->getDerived<ContainerNode>()->getSocketInContainer(cntdSocket)->toIn()->getCntdFunctionalSocket();
        }
        return cntdSocket;
}

DoutSocket* DinSocket::getCntdWorkSocket() const
{
        if(!cntdSocket)
            return nullptr;

        DNode *cntdNode = cntdSocket->getNode();
        if(cntdSocket->getNode()->isContainer())
        {
            //if (cntdNode->getNodeType() != CONTAINER)
            //    return cntdSocket;
            //else
            //    return cntdSocket->getNode()->getDerived<ContainerNode>()->getSocketInContainer(cntdSocket)->toIn()->getCntdWorkSocket();
        }
        //else if(cntdSocket->getNode()->getNodeType() == INSOCKETS) {
        //    //||cntdSocket->getNode()->getNodeType() == LOOPINSOCKETS){
        //    DSocket *onContainer = cntdSocket->getNode()->getDerived<SocketNode>()->getContainer()->getSocketOnContainer(cntdSocket); 
        //    if(onContainer)
        //        return onContainer->toIn()->getCntdWorkSocket();
        //    else 
        //        return cntdSocket;
        //}
        /*  
        else if (cntdSocket->getNode()->getNodeType() == LOOPINSOCKETS)
        {
            ns = cntdSocket->getNode()->getDerived<LoopSocketNode>()->getPartnerSocket(cntdSocket);
            if(ns)return ns->toIn()->getCntdFunctionalSocket();
            else
            {
                ns = cntdSocket->getNode()->getDerived<SocketNode>()->getContainer()->getSocketOnContainer(cntdSocket);
                if(ns) return ns->toIn()->getCntdFunctionalSocket();
            }
        }*/

        return cntdSocket;
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

