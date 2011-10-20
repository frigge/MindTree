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

#include "data_node_socket.h"

#include "stdio.h"

#include "source/graphics/nodes/graphics_node_socket.h"
#include "data_node.h"
#include "source/graphics/nodes/graphics_node.h"
#include "source/data/base/frg.h"
#include "source/graphics/base/vnspace.h"

QHash<unsigned short, DSocket*>LoadSocketIDMapper::loadIDMapper;
unsigned short DSocket::count = 0;
QHash<DSocket*, DSocket*> CopySocketMapper::socketMap;
QHash<unsigned short, DSocket*> DSocket::socketIDHash;

unsigned short LoadSocketIDMapper::getID(DSocket *socket)    
{
    return loadIDMapper.key(socket);
}

void LoadSocketIDMapper::setID(DSocket *socket, unsigned short ID)    
{
    loadIDMapper.insert(ID, socket);
}

DSocket * LoadSocketIDMapper::getSocket(unsigned short ID)    
{
//    if(!loadIDMapper.contains(ID))
//        printf("could not resolve pointer for ID: %2d", ID);
    return loadIDMapper.value(ID);
}

void LoadSocketIDMapper::remap()    
{
    foreach(DSocket *socket, loadIDMapper.values())
        if(socket->getDir() == IN)
            if(socket->toIn()->getTempCntdID() > 0)
                socket->toIn()->cntdSocketFromID();
    loadIDMapper.clear();
}

void CopySocketMapper::setSocketPair(DSocket *original, DSocket *copy)    
{
   socketMap.insert(original, copy); 
}

DSocket * CopySocketMapper::getCopy(DSocket *original)    
{
    if(!socketMap.contains(original)) return 0;
    return socketMap.value(original);
}

void CopySocketMapper::remap()    
{
    foreach(DSocket *socket, socketMap.keys())
        if(socket->getDir() == IN)
            if(socket->toIn()->getCntdSocket())
                if(socketMap.contains(socket->toIn()->getCntdSocket()))
                    socketMap[socket]->toIn()->setCntdSocket(socketMap[socket->toIn()->getCntdSocket()]->toOut());
                else
                    socket->toIn()->getCntdSocket()->registerSocket(socketMap[socket]->toIn());

    socketMap.clear();
}

DSocket::DSocket(QString name, socket_type type, DNode *node)
	: name(name), type(type), ID(++count), Variable(false), socketVis(0x0), node(node), is_array(false), index(0)
{
    socketIDHash.insert(ID, this);
}

DSocket::DSocket(DSocket* socket, DNode *node)
: name(socket->getName()), type(socket->getType()), ID(++count), 
    node(node), is_array(socket->isArray()),
    Variable(socket->getVariable()), socketVis(0)
{
    CopySocketMapper::setSocketPair(socket, this);
    socketIDHash.insert(ID, this);
}

DSocket::~DSocket()
{
    if(!is_array || (is_array && arrayID == ID))
        killSocketVis();
    socketIDHash.remove(ID);
}

bool DSocket::operator==(DSocket &socket)    
{
    if(getVariable() != socket.getVariable()
        ||getName() != socket.getName()
        ||getType() != socket.getType())
        return false;
    return true;
}

bool DSocket::operator !=(DSocket &socket)
{
    return(!(*this == socket));
}

DinSocket* DSocket::toIn()
{
    return static_cast<DinSocket*>(this);
}

DoutSocket* DSocket::toOut()
{
    return static_cast<DoutSocket*>(this);
}

void DSocket::setSocketVis(VNSocket *vis)    
{
    socketVis = vis;
}

void DSocket::killSocketVis()    
{
    if(!socketVis)
        return;
    socketVis->killNameVis();
    socketVis->setParentItem(0);
    FRG::Space->removeItem(socketVis);
    delete socketVis;
    socketVis = 0;
    const_cast<VNode*>(node->getNodeVis())->updateNodeVis();
}

bool DSocket::isCompatible(DSocket *s1, DSocket *s2)    
{
    if(s1 == s2)
        return false;
    if(s1->getNode() == s2->getNode())
        return false;
    if(s1->getType() == VARIABLE || s2->getType() == VARIABLE)
        return true;
    if(s1->getType() == s2->getType())
        return true;
    if((s1->getType() == VECTOR && s2->getType() == NORMAL)
        ||s2->getType() == VECTOR && s1->getType() == NORMAL)
        return true;
    return false;
}

DNodeLink DSocket::createLink(DSocket *socket1, DSocket *socket2)    
{
    DinSocket *in;
    DoutSocket *out;

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
    in->addLink(out);
    return DNodeLink(in, out);
}

void DSocket::removeLink(DinSocket *in, DoutSocket *out)    
{
//    if(in->getVariable()) 
//    {
//        in->clearLink(false);
//        out->unregisterSocket(in, false);
//        DNode *inNode = in->getNode();
//        inNode->dec_var_socket(in);
//        return;
//    }
//    if(out->getVariable()) 
//    {
//        in->clearLink(false);
//        DNode *outNode = out->getNode();
//        outNode->dec_var_socket(out);
//        return;
//    }
    in->clearLink();
}

void DSocket::addRenameCB(Callback *cb)    
{
    renameCallbacks.add(cb);
}

void DSocket::remRenameCB(Callback *cb)    
{
    renameCallbacks.remove(cb);
}

void DSocket::addTypeCB(Callback *cb)    
{
    changeTypeCallbacks.add(cb);
}

void DSocket::remTypeCB(Callback *cb)    
{
    changeTypeCallbacks.remove(cb);
}

bool DSocket::isArray()    
{
    return is_array; 
}

void DSocket::setArray(unsigned short arrID)    
{
    if(arrID > 0)
        is_array = true;
    else
        is_array = false;

    arrayID = arrID;

    if(arrayID  == ID)
        node->addArray(this);
    else if(arrayID > 0)
        getSocket(arrayID)->getArray()->addSocket(this);
}

ArrayContainer* DSocket::getArray()    
{
    return array;
}

int DSocket::getIndex()    
{
    return index;
}

void DSocket::setIndex(int ind)    
{
    index = ind;
}

unsigned short DSocket::getArrayID()
{
    return arrayID;
}

DSocket* DSocket::getSocket(unsigned short ID)    
{
    return socketIDHash.value(ID);
}

ArrayContainer::ArrayContainer(DSocket *first)
{
    ID = first->getID();
    addSocket(first);
}

void ArrayContainer::addSocket(DSocket *socket)    
{
    if(array.isEmpty())
        socket->setIndex(0);
    else
        socket->setIndex(array.last()->getIndex()+1);
    array.append(socket);
}

DinSocket::DinSocket(QString name, socket_type type, DNode *node)
	: DSocket(name, type, node), cntdSocket(0), tempCntdID(0), Token(false)
{
	setDir(IN);
    getNode()->addSocket(this);
};

DinSocket::DinSocket(DinSocket* socket, DNode *node)
    : DSocket(socket, node),
    cntdSocket(socket->getCntdSocket()),
    tempCntdID(0), Token(socket->getToken())
{
    setDir(IN);
    getNode()->addSocket(this);
}

DinSocket::~DinSocket()
{
    if(cntdSocket)
        cntdSocket->unregisterSocket(this);
}

void DinSocket::addLink(DoutSocket *socket)
{
    if(cntdSocket == socket)
        return;

    if(!isCompatible(this, socket))
        return;

    if(cntdSocket)
    {
        setCntdSocket(socket);
        return;
    }

    //here we set the actual link
    setCntdSocket(socket);

    if (getType() == VARIABLE)
    {
        setType(socket->getType());
        setName(socket->getName());
    }
    if (getVariable())
        getNode()->inc_var_socket();
}

void DinSocket::clearLink(bool unregister)
{
    if(cntdSocket&&unregister)cntdSocket->unregisterSocket(this);
	cntdSocket = 0;

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

QDataStream & operator<<(QDataStream &stream, DSocket *socket)
{
    stream<<(unsigned short)socket->getID()<<socket->getVariable();
    stream<<socket->getName()<<socket->getType();
	if(socket->getDir() == IN)
	{
		stream<<socket->toIn()->getToken();
        if(socket->toIn()->getCntdSocket())
            stream<<socket->toIn()->getCntdSocket()->getID();
        else
            stream<<(unsigned short)(0);
	}
    return stream;
}

QDataStream & operator>>(QDataStream &stream, DSocket **socket)
{
	int type;
    unsigned short ID;
    QString name;
    bool isVar; 
    stream>>ID;
    stream>>isVar;
    stream>>name;
    stream>>type;

	DSocket *newsocket = *socket;
    newsocket->setType((socket_type)type);
    newsocket->setName(name);
    newsocket->setVariable(isVar);
	if(newsocket->getDir() == IN)
	{
		bool istoken;
		unsigned short tempID;
		stream>>istoken;
		stream>>tempID;
		newsocket->toIn()->setTempCntdID(tempID);
		newsocket->toIn()->setToken(istoken); 
	}
    LoadSocketIDMapper::setID(newsocket, ID);
    return stream;
}

bool DinSocket::operator==(DinSocket &socket)
{
    if(DSocket::operator!=(socket)
        ||getToken() != socket.getToken())
        return false;
    return true;
}

bool DinSocket::operator !=(DinSocket &socket)
{
    return(!(*this == socket));
}

void DSocket::setNode(DNode *node)
{
    this->node = node;
    if(getVariable())node->setVarSocket(this);
}

void DSocket::createSocketVis(VNode *parent)
{
    if(socketVis) return;
    if(!is_array || (is_array && arrayID == ID))
        socketVis = new VNSocket(this, parent);	
    else
        socketVis = getSocket(arrayID)->getSocketVis();
}

QString DSocket::getName() const
{
	return name;
}

void DSocket::setName(QString value)
{
	name = value;
    renameCallbacks();
}

socket_type DSocket::getType() const
{
	return type;
}

void DSocket::setType(socket_type value)
{
	type = value;
    changeTypeCallbacks();
}

socket_dir DSocket::getDir() const
{
	return dir;
}

void DSocket::setDir(socket_dir value)
{
	dir = value;
}

DNode* DSocket::getNode() const
{
	return node;
}

DoutSocket::DoutSocket(QString name, socket_type type, DNode *node)
	: DSocket(name, type, node)
{
	setDir(OUT);
    getNode()->addSocket(this);
    //setSocketVarName(); 
}

DoutSocket::DoutSocket(DoutSocket* socket, DNode *node)
    : DSocket(socket, node)
{
    setDir(OUT);
    getNode()->addSocket(this);
//    setSocketVarName();
}

DoutSocket::~DoutSocket()
{
    foreach(DinSocket *socket, cntdSockets)
        socket->clearLink(false);
}

bool DoutSocket::operator==(DoutSocket &socket)
{
    if(DSocket::operator!=(socket))
        return false;
    return true;
}

bool DoutSocket::operator !=(DoutSocket &socket)
{
    return(!(*this == socket));
}

void DoutSocket::setName(QString name)    
{
    DSocket::setName(name);
}

QString DoutSocket::setSocketVarName(QHash<QString, unsigned short> *SocketNameCnt)
{
    QString name = getName();
    QString varname_raw = name.replace(" ", "_");

    switch(getNode()->getNodeType())
    {
        case SURFACEINPUT:
        case DISPLACEMENTINPUT:
        case VOLUMEINPUT:
        case LIGHTINPUT:
        case INSOCKETS:
        case LOOPINSOCKETS:
            varname = name;
            break;
        default:
            if(getNode()->isContainer())
                break;

            if(SocketNameCnt->contains(varname_raw))
            {
                unsigned short cnt = SocketNameCnt->value(varname_raw);
                SocketNameCnt->remove(varname_raw);
                SocketNameCnt->insert(varname_raw, ++cnt);
                varname = varname_raw + QString::number(SocketNameCnt->value(varname_raw));
            }
            else
            {
                SocketNameCnt->insert(varname_raw, 1);
                varname = varname_raw;
            }
            break;
    }
    return varname;
}

QString DoutSocket::getVarName() const
{
    return varname;
}

void DoutSocket::registerSocket(DSocket *socket)    
{
    if(this == getNode()->getVarSocket()) 
        getNode()->inc_var_socket();
    if(!cntdSockets.contains(socket->toIn()))
        cntdSockets.append(socket->toIn()); 
}

QList<DNodeLink> DoutSocket::getLinks() const
{
    QList<DNodeLink> links;
    foreach(DinSocket *socket, cntdSockets)
        links.append(DNodeLink(socket, const_cast<DoutSocket*>(this)));
    return links;
}

void DoutSocket::unregisterSocket(DinSocket *socket, bool decr)
{
    cntdSockets.removeAll(socket); 
    if(cntdSockets.isEmpty() && getVariable() && decr) getNode()->dec_var_socket(this);
}

void DinSocket::setCntdSocket(DoutSocket *socket)
{
    if(!socket)
    {
        clearLink();
        return;
    }
    if(cntdSocket)
        cntdSocket->unregisterSocket(this, false);

	cntdSocket = socket;
    if(socket->getType() == VARIABLE)
    {
        socket->setType(getType());
        socket->setName(getName());
    }
    if(getType() == VARIABLE)
    {
        setType(socket->getType());
        setName(socket->getName());
    }
    cntdSocket->registerSocket(this);
}

void DinSocket::cntdSocketFromID()
{
    setCntdSocket(LoadSocketIDMapper::getSocket(getTempCntdID())->toOut());
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
            return 0;
        if(cntdSocket->getNode()->isContainer())
            return cntdSocket->getNode()->getDerived<ContainerNode>()->getSocketInContainer(cntdSocket)->toIn()->getCntdFunctionalSocket();
        else if(cntdSocket->getNode()->getNodeType() == INSOCKETS
            ||cntdSocket->getNode()->getNodeType() == LOOPINSOCKETS)
            return cntdSocket->getNode()->getDerived<SocketNode>()->getContainer()->getSocketOnContainer(cntdSocket)->toIn()->getCntdFunctionalSocket();

        return cntdSocket;
}

bool DinSocket::getToken() const
{
	return Token;
}

void DinSocket::setToken(bool value)
{
	Token = value;
}

bool DSocket::getVariable() const
{
	return Variable;
}

void DSocket::setVariable(bool value)
{
	Variable = value;
    if(getNode()&&Variable)getNode()->setVarSocket(this);
}

VNSocket* DSocket::getSocketVis()
{
    createSocketVis(getNode()->getNodeVis());
	return socketVis;
}
unsigned short DSocket::getID() const
{
	return ID;
}
