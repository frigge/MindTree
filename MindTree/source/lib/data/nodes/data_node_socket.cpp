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

#include "data/frg.h"
#include "data/signal.h"
#include "data_node.h"

#include "data_node_socket.h"

using namespace MindTree;

QHash<unsigned short, const DSocket*>LoadSocketIDMapper::loadIDMapper;
unsigned short DSocket::count = 0;
QHash<const DSocket*, const DSocket*> CopySocketMapper::socketMap;
QHash<unsigned short, DSocket*> DSocket::socketIDHash;

LLsocket& LLsocket::operator++()    
{
    return *next; 
}

DSocket* LLsocket::operator*()   
{
    return socket; 
}

bool LLsocket::operator!=(const LLsocket& other)
{
    return other.socket != socket;
}

DSocketList::DSocketList()
    : first(0), length(0)
{
}

DSocketList::DSocketList(const DSocketList &other)
    : first(other.getFirst()), length(other.length)
{
}

void DSocketList::initList(DSocket *socket)    
{
    first = new LLsocket;
    first->socket = socket;
    first->prev = 0;
    first->next = 0;
    length = 1;
}

LLsocket* DSocketList::getLLlastSocket()const
{
    LLsocket *l=first;
    while(l->next){
        l = l->next;
    }
    return l;
}

const LLsocket& DSocketList::end()    
{
    return *getLLlastSocket();
}

void DSocketList::add(DSocket *socket)    
{
    if(!first){
        initList(socket);
        return;
    }

    LLsocket *last = getLLlastSocket();

    LLsocket *newLLs = new LLsocket;
    newLLs->next = 0;
    last->next = newLLs;
    newLLs->prev = last;
    newLLs->socket = socket;
    length++;
}

LLsocket* DSocketList::getFirst()    const
{
    return first; 
}

const LLsocket& DSocketList::begin()    
{
    return *first;
}

void DSocketList::rm(DSocket *socket)    
{
    if(first->socket == socket){
        //we want to remove the first entry
        //so we need to reset the *first pointer
        if(first->next){
            first = first->next;
            delete first->prev;
            first->prev = 0;
            return;
        }
        //there is only one entry
        delete first;
        first = 0;
        return;
    }

    LLsocket *f = first;
    //go through the list to find the one we want to remove
    while(f->socket != socket)
       f = f->next;

    //check if we got the right one
    //and then relink the sorrounding items
    if(f->socket == socket){
        if(f->prev) f->prev->next = f->next;
        if(f->next) f->next->prev = f->prev;
        delete f;
    }
    length--;
}

LLsocket* DSocketList::getLLsocketAt(unsigned short pos)    
{
    int i;
    if(pos >= length) return 0;
    LLsocket *f = first;
    for(i=0; i != pos; i++)
       f = f->next; 
    return f;
}

DSocket* DSocketList::at(uint pos)    
{
    auto socketiter = getLLsocketAt(pos);
    if(socketiter) return getLLsocketAt(pos)->socket;
    return 0;
}

void DSocketList::move(unsigned short oldpos, unsigned short newpos)    
{
    LLsocket *atoldpos, *atnewpos, *oldnext, *oldprev, *newprev;
    atoldpos = getLLsocketAt(oldpos);
    atnewpos = getLLsocketAt(newpos);

    oldnext = atoldpos->next;
    oldprev = atoldpos->prev;
    newprev = atnewpos->prev;

    if(first == atoldpos)
        first = oldnext;

    if(oldprev)oldprev->next = oldnext;
    if(oldnext)oldnext->prev = oldprev;

    if(len() <= newpos)
    {
        atnewpos->next = atoldpos;
        atoldpos->prev = atnewpos;
        atoldpos->next = 0;
    }
    else{
        atnewpos = getLLsocketAt(newpos);
        atnewpos->prev = atoldpos;
        atoldpos->next = atnewpos;
        atoldpos->prev = newprev;
    }
}

DoutSocketList DSocketList::returnAsOutSocketList()    const
{
    DoutSocketList list;
    LLsocket *f = first;
    if(f){
        list.append(f->socket->toOut());
        while(f = f->next)
            list.append(f->socket->toOut());
    }
    return list;
}

DinSocketList DSocketList::returnAsInSocketList()    const
{
    DinSocketList list;
    LLsocket *f = first;
    if(f){
        list.append(f->socket->toIn());
        while(f = f->next)
            list.append(f->socket->toIn());
    }
    return list;
}

unsigned short DSocketList::len()    
{
    LLsocket *f = first;
    unsigned short i=0;
    if(f) for(i=1; f=f->next; i++);
    assert(i == length);
    return i;
}

unsigned short LoadSocketIDMapper::getID(const DSocket *socket)    
{
    return loadIDMapper.key(socket);
}

void LoadSocketIDMapper::setID(const DSocket *socket, unsigned short ID)    
{
    loadIDMapper.insert(ID, socket);
}

const DSocket * LoadSocketIDMapper::getSocket(unsigned short ID)    
{
//    if(!loadIDMapper.contains(ID))
//        printf("could not resolve pointer for ID: %2d", ID);
    return loadIDMapper.value(ID);
}

void LoadSocketIDMapper::remap()    
{
    foreach(const DSocket *socket, loadIDMapper.values())
        if(socket->getDir() == IN)
            if(socket->toIn()->getTempCntdID() > 0)
                const_cast<DSocket*>(socket)->toIn()->cntdSocketFromID();
    loadIDMapper.clear();
}

void CopySocketMapper::setSocketPair(const DSocket *original, const DSocket *copy)    
{
   socketMap.insert(original, copy); 
}

const DSocket * CopySocketMapper::getCopy(const DSocket *original)    
{
    if(!socketMap.contains(original)) return 0;
    return socketMap.value(original);
}

void CopySocketMapper::remap()    
{
    foreach(const DSocket *socket, socketMap.keys())
        if(socket->getDir() == IN)
            if(socket->toIn()->getCntdSocket())
                if(socketMap.contains(socket->toIn()->getCntdSocket()))
                    const_cast<DSocket*>(socketMap[socket])->toIn()->setCntdSocket(const_cast<DSocket*>(socketMap[socket->toIn()->getCntdSocket()])->toOut());
                else
                    const_cast<DoutSocket*>(socket->toIn()->getCntdSocket())->registerSocket(const_cast<DSocket*>(socketMap[socket])->toIn());

    socketMap.clear();
}

int SocketType::id_cnt=-1;
std::vector<std::string> SocketType::id_map;

SocketType::SocketType(socket_type t)
    : old_type(t), type("")
{
    mapToNew();
}

SocketType::SocketType(std::string str)
    : old_type(CUSTOMSOCKET), type(str), _id(getID(str))
{
}

SocketType::~SocketType()
{
}

std::vector<std::string> SocketType::getTypes()    
{
    return id_map; 
}

SocketType SocketType::byID(int id)    
{
    if(id >= id_map.size()) return SocketType();
    return SocketType(id_map[id]);
}

int SocketType::getID(QString type)    
{
    return getID(type.toStdString());
}

int SocketType::getID(std::string type)    
{
    if(type == "") return -1;
    int i = 0;
    for(auto s = id_map.begin(); s != id_map.end(); s++, i++){
        if(type == *s) return i;
    }
    return registerType(type);
}

int SocketType::registerType(std::string type)    
{
    id_map.push_back(type);
    return ++id_cnt;        
}

int SocketType::id()    
{
    return _id;  
}

void SocketType::mapToNew()    
{
     switch(old_type)
     {
         case NORMAL:
             type = "NORMAL";
             break;
         case VECTOR:
             type = "VECTOR";
             break;
         case FLOAT:
             type = "FLOAT";
             break;
         case COLOR:
             type = "COLOR";
             break;
         case POINT:
             type = "POINT";
             break;
         case STRING:
             type = "STRING";
             break;
         case VARIABLE:
             type = "VARIABLE";
             break;
         case CONDITION:
             type = "CONDITION";
             break;
         case MATRIX:
             type = "MATRIX";
             break;
         case SCENEOBJECT:
             type = "SCENEOBJECT";
             break;
         case POLYGON:
             type = "POLYGON";
             break;
         case INTEGER:
             type = "INTEGER";
             break;

         case VEC2:
             type = "case ";
             break;
         case MAT2:
             type = "MAT2";
             break;
         case MAT3:
             type = "MAT3";
             break;

         case SAMPLER1D:
             type = "case ";
             break;
         case SAMPLER2D:
             type = "IMAGE";
             break;
         case SAMPLER3D:
             type = "SAMPLER3D";
             break;
         case SAMPLERCUBE:
             type = "SAMPLERCUBE";
             break;
         case SAMPLER1DSHADOW:
             type = "SAMPLER1DSHADOW";
             break;
         case SAMPLER2DSHADOW:
             type = "SAMPLER2DSHADOW";
             break;

         case OBJDATA:
             type = "OBJDATA";
             break;
         case SCENEDATA:
             type = "SCENEDATA";
             break;
         case GROUPDATA:
             type = "GROUPDATA";
             break;
        default: break;
    }
     _id = getID(type);
}

QDataStream & operator<<(QDataStream &stream, SocketType type)
{
    stream << type.getType() << type.getCustomType();
    return stream;
}

socket_type SocketType::getType() const
{
    return old_type;
}

void SocketType::setType(socket_type value)
{
    old_type = value;
    mapToNew();
}

std::string SocketType::getCustomType() const
{
    return type;
}

void SocketType::setCustomType(std::string value)
{
    old_type = CUSTOMSOCKET;
    type = value;
}

bool SocketType::operator==(const SocketType &other)    
{
    if(other.getType() != old_type) return false;
    if(other.getCustomType() != type) return false;
    return true;
}

bool SocketType::operator==(socket_type t)    
{
    return old_type == t;
}

bool SocketType::operator==(std::string str)    
{
    return type == str;
}

bool SocketType::operator!=(std::string str)    
{
    return !(*this == str);
}

bool SocketType::operator!=(const SocketType &other)    
{
    return !(*this == other);
}

bool SocketType::operator!=(socket_type t)    
{
    return!(*this == t);
}

SocketType& SocketType::operator=(socket_type t)    
{
    setType(t);
    return *this;
}

SocketType& SocketType::operator=(std::string str)    
{
    setCustomType(str);
    return *this;
}

DSocket::DSocket(std::string name, SocketType type, DNode *node)
	: name(name), type(type), ID(++count), variable(false), node(node), is_array(false)
{
    socketIDHash.insert(ID, this);
}

DSocket::DSocket(const DSocket& socket, DNode *node)
: name(socket.getName()), type(socket.getType()), ID(++count),
    QObject(),
    node(node), is_array(socket.getArray()),
    variable(socket.getVariable())
{
    CopySocketMapper::setSocketPair(&socket, this);
    socketIDHash.insert(ID, this);
}

DSocket::~DSocket()
{
    std::cout << "delete DinSocket" << std::endl;
    socketIDHash.remove(ID);
    emit removed();
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
    if(s1 == VARIABLE || s2 == VARIABLE)
        return true;
    if(s1 == s2)
        return true;
    if((s1 == VECTOR && s2 == NORMAL)
        ||(s2 == VECTOR && s1 == NORMAL))
        return true;
    if((s1 == INTEGER && s2 == FLOAT)
        || (s1 == FLOAT && s2 == INTEGER))
        return true;
    if(s1 == GROUPDATA || s2 == GROUPDATA)
        return true;
    return false;
}

DNodeLink DSocket::createLink(DSocket *socket1, DSocket *socket2)    
{
    DinSocket *in = 0;
    DAInSocket *ain = 0;
    DoutSocket *out = 0;

    if(socket1->getDir() == IN)
    {
       if(socket1->getArray())
           ain = (DAInSocket*)socket1;
        else
           in = socket1->toIn();
       out = socket2->toOut(); 
    }
    else
    {
       out = socket1->toOut(); 
       if(socket2->getArray())
           ain = (DAInSocket*)socket2;
        else
           in = socket2->toIn();
    }
    if ( socket1->getType() == VARIABLE ) {
        socket1->setType(socket2->getType());
        socket1->setName(socket2->getName());
    }
    if ( socket2->getType() == VARIABLE ) {
        socket2->setType(socket1->getType());
        socket2->setName(socket1->getName());
    }
    if(in) {
        in->setCntdSocket(out);
        return DNodeLink(in, out);
    }
    else {
        ain->addLink(out);
        return DNodeLink(ain, out);
    }
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

void DSocket::setArray(bool arr)
{
    is_array = arr;
}

bool DSocket::getArray() const
{
    return is_array;
}

DSocket* DSocket::getSocket(unsigned short ID)    
{
    return socketIDHash.value(ID);
}

//void DSocket::nameCB(const DSocket *first, const DSocket *last)    
//{
//    first->connect(first, SIGNAL(nameChanged(QString)), last, SLOT(setName(QString)));
//}

//void DSocket::typeCB(const DSocket *first, const DSocket *last)    
//{
//    //first->connect(first, SIGNAL(typeChanged(socket_type)), last, SLOT(setType(socket_type)));
//}

DAInSocket::DAInSocket(std::string name, SocketType t, DNode *node)
    : DSocket(name, t, node)
{
    setDir(IN);
    node->addSocket(this);
    setArray(true);
}

DAInSocket::~DAInSocket()
{
    //foreach(DoutSocket *cntdSocket, cntdSockets)
    //    cntdSocket->unregisterSocket(this);
}

void DAInSocket::addLink(DoutSocket *socket)    
{
    if(!cntdSockets.contains(socket) && isCompatible(this, socket))
        cntdSockets.append(socket);
}

QList<DoutSocket*> DAInSocket::getLinks()const
{
    return cntdSockets;
}

DinSocket::DinSocket(std::string name, SocketType type, DNode *node)
	: DSocket(name, type, node), cntdSocket(0), tempCntdID(0), Token(false)
{
	setDir(IN);
    getNode()->addSocket(this);
};

DinSocket::DinSocket(const DinSocket& socket, DNode *node)
    : DSocket(socket, node),
    cntdSocket(socket.getCntdSocket()),
    tempCntdID(0), Token(socket.getToken()),
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
	cntdSocket = 0;

    emit disconnected();
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

QDataStream & MindTree::operator<<(QDataStream &stream, DSocket *socket)
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

QDataStream & MindTree::operator>>(QDataStream &stream, DSocket **socket)
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
    newsocket->setName(name.toStdString());
    newsocket->setVariable(isVar);
	if(newsocket->getDir() == IN)
	{
		bool istoken;
		unsigned short tempID;
        bool bprop;
        QString sprop;
        Vector vprop;
        double fprop;
        int iprop;
        QColor cprop;
		stream>>istoken;
		stream>>tempID;
		newsocket->toIn()->setTempCntdID(tempID);
		newsocket->toIn()->setToken(istoken); 
	}
    LoadSocketIDMapper::setID(newsocket, ID);
    return stream;
}

bool DinSocket::operator==(DinSocket &socket)const
{
    if(DSocket::operator!=(socket)
        ||getToken() != socket.getToken())
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
    //bool ok=false;
    //int ar = name.indexOf("[");
    //int l = 0;
    //if(ar >= 0)
    //{
    //    QString arrlengthString = name.right(name.size() - ar);
    //    arrlengthString = arrlengthString.replace("]", "");
    //    arrlengthString = arrlengthString.replace("[", "");
    //    l = arrlengthString.toInt(&ok);
    //}
    //if(ok) 
    //{
    //     arrayLength = l;
    //     is_array = true;
    //}
    //else
    //    is_array = false;

    if(name != "+")
        emit nameChanged(name.c_str());
}

SocketType DSocket::getType() const
{
	return type;
}

void DSocket::setType(SocketType value)
{
    if(value == type) return;
	type = value;

    if(type != VARIABLE)
        emit typeChanged(type);
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

DoutSocket::DoutSocket(std::string name, SocketType type, DNode *node)
	: DSocket(name, type, node)
{
	setDir(OUT);
    getNode()->addSocket(this);
    //setSocketVarName(); 
}

DoutSocket::DoutSocket(const DoutSocket& socket, DNode *node)
    : DSocket(socket, node), attachedData(0)
{
    setDir(OUT);
    getNode()->addSocket(this);
//    setSocketVarName();
}

DoutSocket::~DoutSocket()
{
    std::cout << "delete DinSocket" << std::endl;
    foreach(DinSocket *socket, cntdSockets)
        socket->clearLink();
}

void* DoutSocket::getAttachedData()
{
    return attachedData;
}

void DoutSocket::setAttachedData(void* value)
{
    attachedData = value;
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

QString DSocket::setSocketVarName(QHash<QString, unsigned short> *SocketNameCnt)const
{
    QString name = getName().c_str();
    QString varname_raw = name.replace(" ", "_");
    int ar = varname_raw.indexOf("[");
    if(ar >= 0)
        varname_raw = varname_raw.left(ar);

    switch(getNode()->getNodeType())
    {
        case SURFACEINPUT:
        case DISPLACEMENTINPUT:
        case VOLUMEINPUT:
        case LIGHTINPUT:
        case INSOCKETS:
            varname = name;
            break;
        default:
            if(getNode()->getNodeType() == CONTAINER)
                break;

            if(SocketNameCnt->contains(varname_raw)) {
                unsigned short cnt = SocketNameCnt->value(varname_raw);
                SocketNameCnt->remove(varname_raw);
                SocketNameCnt->insert(varname_raw, ++cnt);
                varname = varname_raw + QString::number(SocketNameCnt->value(varname_raw));
            }
            else {
                SocketNameCnt->insert(varname_raw, 1);
                varname = varname_raw;
            }
            break;
    }
    return varname;
}

QString DSocket::getVarName() const
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
    emit disconnected();
    cntdSockets.removeAll(socket); 
    if(cntdSockets.isEmpty() && getVariable() && decr) getNode()->dec_var_socket(this);
}

void DinSocket::setCntdSocket(DoutSocket *socket)
{
    if(!socket) {
        clearLink();
        return;
    }
    if(cntdSocket == socket)
        return;

//    if(!isCompatible(this, socket))
//        return;

    //here we set the actual link
    if(cntdSocket) cntdSocket->unregisterSocket(this, false);

	cntdSocket = socket;
    cntdSocket->registerSocket(this);
    if (getVariable())
        getNode()->inc_var_socket();
    //std::cout << typeid(this).name() << std::endl;
    MT_SIGNAL_EMITTER(this);
    MT_CUSTOM_SIGNAL_EMITTER("createLink", this);
    //emit linked(socket);
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
        DSocket *ns=0;
        if(!cntdSocket)
            return 0;

        DNode *cntdNode = cntdSocket->getNode();
        if(cntdNode->isContainer())
        {
            return cntdNode->getDerived<ContainerNode>()->getSocketInContainer(cntdSocket)->toIn()->getCntdFunctionalSocket();
        }
        else if(cntdNode->getNodeType() == INSOCKETS)
        {
            DinSocket *ns = cntdNode->getDerived<SocketNode>()->getContainer()->getSocketOnContainer(cntdSocket)->toIn();
            if(ns) return ns->getCntdFunctionalSocket();
            else return 0;
        }
        else if (cntdNode->getNodeType() == LOOPINSOCKETS)
        {
            ns = cntdNode->getDerived<LoopSocketNode>()->getPartnerSocket(cntdSocket);
            if(ns)return ns->toIn()->getCntdFunctionalSocket();
            else
            {
                ns = cntdNode->getDerived<SocketNode>()->getContainer()->getSocketOnContainer(cntdSocket);
                if(ns) return ns->toIn()->getCntdFunctionalSocket();
            }
        }

        return cntdSocket;
}

DoutSocket* DinSocket::getCntdWorkSocket() const
{
        DSocket *ns=0;
        if(!cntdSocket)
            return 0;

        DNode *cntdNode = cntdSocket->getNode();
        if(cntdSocket->getNode()->isContainer())
        {
            if (cntdNode->getNodeType() != CONTAINER)
                return cntdSocket;
            else
                return cntdSocket->getNode()->getDerived<ContainerNode>()->getSocketInContainer(cntdSocket)->toIn()->getCntdWorkSocket();
        }
        else if(cntdSocket->getNode()->getNodeType() == INSOCKETS) {
            //||cntdSocket->getNode()->getNodeType() == LOOPINSOCKETS){
            DSocket *onContainer = cntdSocket->getNode()->getDerived<SocketNode>()->getContainer()->getSocketOnContainer(cntdSocket); 
            if(onContainer)
                return onContainer->toIn()->getCntdWorkSocket();
            else 
                return cntdSocket;
        }
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

