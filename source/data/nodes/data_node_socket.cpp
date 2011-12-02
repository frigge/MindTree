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

DSocketList::DSocketList()
    : first(0)
{
}

void DSocketList::initList(DSocket *socket)    
{
    first = new LLsocket;
    first->socket = socket;
    first->prev = 0;
    first->next = 0;
}

LLsocket* DSocketList::getLLlastSocket()const
{
    LLsocket *l=first;
    while(l->next){
        l = l->next;
    }
    return l;
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
}

LLsocket* DSocketList::getLLsocketAt(unsigned short pos)    
{
    int i;
    LLsocket *f = first;
    for(i=0; i != pos; i++)
       f = f->next; 
    return f;
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
    return i;
}

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
    blockAllCB();
    CopySocketMapper::setSocketPair(socket, this);
    socketIDHash.insert(ID, this);
    unblockAllCB();
}

DSocket::~DSocket()
{
    if(!is_array || (is_array && arrayID == ID))
        killSocketVis();
    socketIDHash.remove(ID);
}

void DSocket::blockAllCB()    
{
    linkCallbacks.setBlock(true); 
    removeLinkCallbacks.setBlock(true); 
    renameCallbacks.setBlock(true); 
    changeTypeCallbacks.setBlock(true); 
}

void DSocket::unblockAllCB()    
{
    linkCallbacks.setBlock(false); 
    removeLinkCallbacks.setBlock(false); 
    renameCallbacks.setBlock(false); 
    changeTypeCallbacks.setBlock(false); 
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
        ||(s2->getType() == VECTOR && s1->getType() == NORMAL))
        return true;
    if((s1->getType() == INTEGER && s2->getType() == FLOAT)
        || (s1->getType() == FLOAT && s2->getType() == INTEGER))
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

void DSocket::addLinkCB(Callback *cb)    
{
    linkCallbacks.add(cb);
}

void DSocket::remLinkCB(Callback *cb)    
{
    linkCallbacks.remove(cb);
}

void DSocket::addRmLinkCB(Callback *cb)    
{
    removeLinkCallbacks.add(cb);
}

void DSocket::remRmLinkCB(Callback *cb)    
{
    removeLinkCallbacks.remove(cb);
}

bool DSocket::isArray() const
{
    return is_array; 
}

int DSocket::getArrayLength() const
{
    return arrayLength; 
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

ArrayContainer* DSocket::getArray() const
{
    return array;
}

int DSocket::getIndex() const
{
    return index;
}

void DSocket::setIndex(int ind)    
{
    index = ind;
}

unsigned short DSocket::getArrayID() const
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
	: DSocket(name, type, node), cntdSocket(0), tempCntdID(0), Token(false), linkedNameCB(0), linkedTypeCB(0),
    prop(0)
{
	setDir(IN);
    getNode()->addSocket(this);

    setProperty();
};

DinSocket::DinSocket(DinSocket* socket, DNode *node)
    : DSocket(socket, node), linkedNameCB(0), linkedTypeCB(0), 
    cntdSocket(socket->getCntdSocket()),
    tempCntdID(0), Token(socket->getToken()),
    prop(0)
{
    setDir(IN);
    getNode()->addSocket(this);

    setProperty();
}

DinSocket::~DinSocket()
{
    if(cntdSocket)
        cntdSocket->unregisterSocket(this);
    delete prop;
}

void DinSocket::setType(socket_type value)
{
    DSocket::setType(value);
    setProperty();
}

Property* DinSocket::getProperty()const
{
    return prop;
}

void DinSocket::setProperty()    
{
    if(prop) delete prop;
    switch(getType())
    {
        case NORMAL:
        case VECTOR:
        case POINT:
            prop = new VectorProperty(this);
            break;
        case COLOR:
            prop = new ColorProperty(this);
            break;
        case STRING:
            prop = new StringProperty(this);
            break;
        case FLOAT:
            prop = new FloatProperty(this);
            break;
        case INTEGER:
            prop = new IntProperty(this);
            break;
        case CONDITION:
            prop = new BoolProperty(this);
            break;
        default:
            prop = 0;
            break;
    }
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
    removeLinkCallbacks();
    removeLinkCallbacks.clear();

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
        Vector vec;
        switch(socket->getType())
        {
            case FLOAT:
                stream<<((FloatProperty*)socket->toIn()->getProperty())->getValue();
                break;
            case INTEGER:
                stream<<((IntProperty*)socket->toIn()->getProperty())->getValue();
                break;
            case STRING:
                stream<<((StringProperty*)socket->toIn()->getProperty())->getValue();
                break;
            case CONDITION:
                stream<<((BoolProperty*)socket->toIn()->getProperty())->getValue();
                break;
            case POINT:
            case NORMAL:
            case VECTOR:
                vec = ((VectorProperty*)socket->toIn()->getProperty())->getValue();
                stream<<vec.x<<vec.y<<vec.z;
                break;
            case COLOR:
                stream<<((ColorProperty*)socket->toIn()->getProperty())->getValue();
                break;
            default:
                break;
        }
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
        switch(type)
        {
            case FLOAT:
                stream>>fprop;
                ((FloatProperty*)newsocket->toIn()->getProperty())->setValue(fprop);
                break;
            case INTEGER:
                stream>>iprop;
                ((IntProperty*)newsocket->toIn()->getProperty())->setValue(iprop);
                break;
            case STRING:
                stream>>sprop;
                ((StringProperty*)newsocket->toIn()->getProperty())->setValue(sprop);
                break;
            case CONDITION:
                stream>>bprop;
                ((BoolProperty*)newsocket->toIn()->getProperty())->setValue(bprop);
                break;
            case POINT:
            case NORMAL:
            case VECTOR:
                stream>>vprop.x>>vprop.y>>vprop.z;
                ((VectorProperty*)newsocket->toIn()->getProperty())->setValue(vprop);
                break;
            case COLOR:
                stream>>cprop;
                ((ColorProperty*)newsocket->toIn()->getProperty())->setValue(cprop);
                break;
            default:
                break;
        }
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
    bool ok=false;
    int ar = name.indexOf("[");
    int l = 0;
    if(ar >= 0)
    {
        QString arrlengthString = name.right(name.size() - ar);
        arrlengthString = arrlengthString.replace("]", "");
        arrlengthString = arrlengthString.replace("[", "");
        l = arrlengthString.toInt(&ok);
    }
    if(ok) 
    {
         arrayLength = l;
         is_array = true;
    }
    else
        is_array = false;
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

void DoutSocket::setName(QString name)    
{
    DSocket::setName(name);
}

QString DSocket::setSocketVarName(QHash<QString, unsigned short> *SocketNameCnt)const
{
    QString name = getName();
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

QString DSocket::getVarName() const
{
    return varname;
}

void DoutSocket::registerSocket(DSocket *socket)    
{
    linkCallbacks();
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
    removeLinkCallbacks();
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
    {
        cntdSocket->unregisterSocket(this, false);
        if(linkedNameCB) 
        {
            cntdSocket->remRenameCB(linkedNameCB);
        }
        if(linkedTypeCB) {
            cntdSocket->remTypeCB(linkedTypeCB);
        }
    }

	cntdSocket = socket;
    linkCallbacks();
    if(socket->getType() == VARIABLE)
    {
        socket->setType(getType());
        socket->setName(getName());
    }
    if(getType() == VARIABLE)
    {
        linkedNameCB = new ScpNameCB(socket, this);
        linkedTypeCB = new ScpTypeCB(socket, this);
        socket->addRenameCB(linkedNameCB);
        socket->addTypeCB(linkedTypeCB);
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

QString DSocket::getIDName()
{
    return idName;
}

void DSocket::setIDName(QString value)
{
    idName = value;
}

