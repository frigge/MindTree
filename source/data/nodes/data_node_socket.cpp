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
QHash<QString, int> DSocket::SocketNameCnt;
unsigned short DSocket::count = 0;
QHash<DSocket*, DSocket*> CopySocketMapper::socketMap;

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

QList<DSocket*> LoadSocketIDMapper::getAllSockets()    
{
    return loadIDMapper.values();
}

void LoadSocketIDMapper::clear()    
{
    loadIDMapper.clear();
}

void CopySocketMapper::setSocketPair(DSocket *original, DSocket *copy)    
{
   socketMap.insert(original, copy); 
}

DSocket * CopySocketMapper::getCopy(DSocket *original)    
{
    return socketMap.value(original);
}

void CopySocketMapper::clear()    
{
    socketMap.clear();
}

DSocket::DSocket(QString name, socket_type type)
	: name(name), type(type), ID(++count), Variable(false), socketVis(0), node(0)
{
}

DSocket::DSocket(DSocket* socket)
    : name(socket->getName()), type(socket->getType()), ID(++count), node(0),
    Variable(socket->getVariable()), socketVis(0)
{
    CopySocketMapper::setSocketPair(socket, this);
}

DSocket::~DSocket()
{
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
    socketVis->deleteLater();
    socketVis = 0;
    node->getNodeVis()->updateNodeVis();
}

bool DSocket::isCompatible(DSocket *s1, DSocket *s2)    
{
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
       in = (DinSocket*)socket1;
       out = (DoutSocket*)socket2; 
    }
    else
    {
       out = (DoutSocket*)socket1; 
       in = (DinSocket*)socket2;
    }
    in->addLink(out);
    return DNodeLink(in, out);
}

DinSocket::DinSocket(QString name, socket_type type)
	: DSocket(name, type), cntdSocket(0), tempCntdID(0), Token(false)
{
	setDir(IN);
};

DinSocket::DinSocket(DinSocket* socket)
    : DSocket(socket),
    cntdSocket(socket->getCntdSocket()),
    tempCntdID(0), Token(socket->getToken())
{
    setDir(IN);
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

void DinSocket::clearLink()
{
    if(getVariable())
        getNode()->dec_var_socket(this);

	cntdSocket = 0;
}

unsigned short DinSocket::getTempCntdID()
{
	return tempCntdID;
}

void DinSocket::setTempCntdID(unsigned short value)
{
	tempCntdID = value;
}

QDataStream & operator<<(QDataStream &stream, DSocket *socket)
{
    stream<<(unsigned short)socket->getID()<<socket->getVariable()<<socket->getDir();
    stream<<socket->getName()<<socket->getType();
	if(socket->getDir() == IN)
	{
		DinSocket *inSocket = (DinSocket*)socket;
		stream<<inSocket->getToken();
        if(inSocket->getCntdSocket())
            stream<<inSocket->getCntdSocket()->getID();
        else
            stream<<(unsigned short)(0);
	}
    return stream;
}

QDataStream & operator>>(QDataStream &stream, DSocket **socket)
{
    int dir;
	int type;
    unsigned short ID;
    QString name;
    bool isVar; 
    stream>>ID;
    stream>>isVar;
    stream>>dir;
    stream>>name;
    stream>>type;
	DSocket *newsocket;
	if(dir == IN)
	{
		bool istoken;
		unsigned short tempID;
		stream>>istoken;
		stream>>tempID;
		DinSocket* newInSocket = new DinSocket(name, (socket_type)type);
		newInSocket->setTempCntdID(tempID);
		newInSocket->setToken(istoken); 
		newsocket = (DSocket*)newInSocket;
	}
	else
	{
		newsocket = (DSocket*) new DoutSocket(name, (socket_type)type);
	}
    newsocket->setDir((socket_dir)dir);
    newsocket->setVariable(isVar);
    LoadSocketIDMapper::setID(newsocket, ID);
    *socket = newsocket;
    return stream;
}

bool DinSocket::operator==(DinSocket &socket)
{
    if(getVariable() != socket.getVariable()
            ||getName() != socket.getName()
            ||getType() != socket.getToken()
            ||getType() != socket.getType())
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
    if(!socketVis)socketVis = new VNSocket(this, parent);	
}

QString DSocket::getName()
{
	return name;
}

void DSocket::setName(QString value)
{
	name = value;
}

socket_type DSocket::getType()
{
	return type;
}

void DSocket::setType(socket_type value)
{
	type = value;
}

socket_dir DSocket::getDir()
{
	return dir;
}

void DSocket::setDir(socket_dir value)
{
	dir = value;
}

DNode* DSocket::getNode()
{
	return node;
}

DoutSocket::DoutSocket(QString name, socket_type type)
	: DSocket(name, type)
{
	setDir(OUT);
}

DoutSocket::DoutSocket(DoutSocket* socket)
    : DSocket(socket)
{
    setDir(OUT);
}

DoutSocket::~DoutSocket()
{
    if(getNode()->getSpace())
        foreach(DinSocket *socket, getNode()->getSpace()->getConnected(this))
            socket->clearLink();
}

void DoutSocket::registerSocket(DSocket *socket)    
{
    if(cntdSockets.isEmpty() && getVariable()) getNode()->inc_var_socket();
    cntdSockets.append(static_cast<DinSocket*>(socket)); 
}

QList<DNodeLink> DoutSocket::getLinks()    
{
    QList<DNodeLink> links;
    foreach(DinSocket *socket, cntdSockets)
        links.append(DNodeLink(socket, this));
    return links;
}

void DoutSocket::unregisterSocket(DinSocket *socket)
{
    cntdSockets.removeAll(socket); 
    if(cntdSockets.isEmpty() && getVariable()) getNode()->dec_var_socket(this);
}

void DinSocket::setCntdSocket(DoutSocket *socket)
{
    if(!socket)return;
    if(cntdSocket)
        cntdSocket->unregisterSocket(this);
	cntdSocket = socket;
    cntdSocket->registerSocket(this);
}

DoutSocket* DinSocket::getCntdSocket()
{
	return cntdSocket;
}

bool DinSocket::getToken()
{
	return Token;
}

void DinSocket::setToken(bool value)
{
	Token = value;
}

QString DSocket::getVarName()
{
    if(varname == ""
        && getDir() == IN
        && !getVariable())
        return name;
	return varname;
}

void DSocket::setVarName(QString value)
{
	varname = value;
}

bool DSocket::getVariable()
{
	return Variable;
}

void DSocket::setVariable(bool value)
{
	Variable = value;
    if(getNode())getNode()->setVarSocket(this);
}

VNSocket* DSocket::getSocketVis()
{
    createSocketVis(getNode()->getNodeVis());
	return socketVis;
}
unsigned short DSocket::getID()
{
	return ID;
}
