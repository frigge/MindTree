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

#include "data_node.h"

#include "QCoreApplication"
#include "QTextStream"
#include "QDir"
#include "QDebug"

#include "source/data/base/frg.h"
#include "source/graphics/nodes/graphics_node.h"
#include "source/data/shaderwriter.h"
#include "source/data/base/dnspace.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/base/project.h"

unsigned short DNode::count = 1;
QHash<unsigned short, DNode*>LoadNodeIDMapper::loadIDMapper;

unsigned short LoadNodeIDMapper::getID(DNode *node)    
{
    return loadIDMapper.key(node);
}

void LoadNodeIDMapper::setID(DNode *node, unsigned short ID)    
{
    loadIDMapper.insert(ID, node);
}

DNode * LoadNodeIDMapper::getNode(unsigned short ID)    
{
    return loadIDMapper.value(ID);
}

void LoadNodeIDMapper::clear()    
{
    loadIDMapper.clear();
}

DNode::DNode(QString name)
    : space(0), varcnt(0), ID(count++), nodeVis(0), nodeName(name), varsocket(0), lastsocket(0)
{};

DNode::DNode(DNode* node)
    : space(0), varcnt(0), ID(count++), nodeVis(0),
    nodeName(node->getNodeName()), NodeType(node->getNodeType())
{
    foreach(DinSocket *socket, node->inSockets)
        addSocket(new DinSocket(socket));
    foreach(DoutSocket *socket, node->outSockets)
        addSocket(new DoutSocket(socket));
    varsocket = CopySocketMapper::getCopy(node->getVarSocket());
    lastsocket = CopySocketMapper::getCopy(node->getLastSocket());
}

DNode* DNode::copy(DNode *original)    
{
    DNode *newNode;
    switch(original->getNodeType())
    {
    case CONTAINER:
        newNode = new ContainerNode(static_cast<ContainerNode*>(original));
        break;
    case FUNCTION:
        newNode = new FunctionNode(static_cast<FunctionNode*>(original));
        break;
    case MULTIPLY:
    case DIVIDE:
    case ADD:
    case SUBTRACT:
    case DOTPRODUCT:
        newNode = new MathNode(static_cast<MathNode*>(original));
        break;
    case GREATERTHAN:
    case SMALLERTHAN:
    case EQUAL:
    case AND:
    case OR:
    case NOT:
        newNode = new ConditionNode(static_cast<ConditionNode*>(original));
        break;
    case CONDITIONCONTAINER:
        newNode = new ConditionContainerNode(static_cast<ConditionContainerNode*>(original));
        break;
    case FOR:
        newNode = new ForNode(static_cast<ForNode*>(original));
        break;
    case WHILE: 
        newNode = new WhileNode(static_cast<WhileNode*>(original));
        break;
    case GATHER:
        newNode = new GatherNode(static_cast<GatherNode*>(original));
        break;
    case ILLUMINANCE:
        newNode = new IlluminanceNode(static_cast<IlluminanceNode*>(original));
        break;
    case ILLUMINATE:
        newNode = new IlluminateNode(static_cast<IlluminateNode*>(original));
        break;
    case SOLAR:
        newNode = new SolarNode(static_cast<SolarNode*>(original));
        break;
    case SURFACEINPUT:
    case DISPLACEMENTINPUT:
    case VOLUMEINPUT:
    case LIGHTINPUT:
    case ILLUMINANCEINPUT:
    case SOLARINPUT:
    case ILLUMINATEINPUT:
        newNode = new InputNode(static_cast<InputNode*>(original));
        break;
    case SURFACEOUTPUT:
    case DISPLACEMENTOUTPUT:
    case VOLUMEOUTPUT:
    case LIGHTOUTPUT:
        newNode = new OutputNode(static_cast<OutputNode*>(original));
        break;
    case INSOCKETS:
    case OUTSOCKETS:
        newNode = new SocketNode(static_cast<SocketNode*>(original));
        break;
    case LOOPINSOCKETS:
    case LOOPOUTSOCKETS:
        newNode = new LoopSocketNode(static_cast<LoopSocketNode*>(original));
        break;
    case COLORNODE:
        newNode = new ColorValueNode(static_cast<ColorValueNode*>(original));
        break;
    case FLOATNODE:
        newNode = new FloatValueNode(static_cast<FloatValueNode*>(original));
        break;
    case STRINGNODE:
        newNode = new StringValueNode(static_cast<StringValueNode*>(original));
        break;
    case VECTORNODE:
        newNode = new VectorValueNode(static_cast<VectorValueNode*>(original));
        break;
    }
    //FRG::CurrentProject->setNodePosition(newNode, FRG::CurrentProject->getNodePosition(original));
    return newNode;
}

QList<DNode*> DNode::copy(QList<DNode*>nodes)    
{
   QList<DNode*>nodeCopies;
   foreach(DNode *node, nodes)
   {
        DNode *newNode = copy(node);
        foreach(DinSocket *socket, newNode->getInSockets())
            if(socket->getCntdSocket()
                &&nodes.contains(socket->getCntdSocket()->getNode()))
                socket->setCntdSocket(static_cast<DoutSocket*>(CopySocketMapper::getCopy(socket->getCntdSocket())));
        nodeCopies.append(newNode);
   }
   return nodeCopies;
}

DNode::~DNode()
{
    foreach(DinSocket *socket, inSockets)
        delete socket;
    foreach(DoutSocket *socket, outSockets)
        delete socket;

    if(nodeVis) deleteNodeVis();
    FRG::CurrentProject->clearNodePosition(this);
}

VNode* DNode::createNodeVis()
{
    nodeVis = new VNode(this);
    return nodeVis;
}

void DNode::deleteNodeVis()    
{
    FRG::Space->removeItem(nodeVis);
    delete nodeVis;
    nodeVis = 0;
}

QPointF DNode::getPos()    
{
    return FRG::CurrentProject->getNodePosition(this);
}

bool DNode::isContainer()
{
    if(NodeType == CONTAINER
            ||NodeType == CONDITIONCONTAINER
            ||NodeType == FOR
            ||NodeType == WHILE
            ||NodeType == GATHER
            ||NodeType == ILLUMINANCE
            ||NodeType == ILLUMINATE
            ||NodeType == SOLAR)
        return true;
    else return false;
}

QDataStream &operator <<(QDataStream &stream, DNode  *node)
{
    stream<<node->getNodeName()<<node->getID()<<node->getNodeType();
    stream<<FRG::CurrentProject->getNodePosition(node);
    stream<<(qint16)node->getInSockets().size()<<(qint16)node->getOutSockets().size();

    foreach(DinSocket *socket, node->getInSockets())
        stream<<(DSocket*)socket;
    foreach(DoutSocket *socket, node->getOutSockets())
        stream<<(DSocket*)socket;

    if(node->getNodeType() == FUNCTION)
    {
        FunctionNode *fnode = (FunctionNode*) node;
        stream<<fnode->getFunctionName();
    }

    if(node->isContainer())
    {
        ContainerNode *cnode =(ContainerNode*) node;
        stream<<cnode->getInputs()->getID()<<cnode->getOutputs()->getID();
        stream<<cnode->getSocketMapSize();
        foreach(DSocket *socket, cnode->getMappedSocketsOnContainer())
            stream<<socket->getID()<<cnode->getSocketInContainer(socket)->getID();
        stream<<cnode->getContainerData();
    }

    if(     node->getNodeType() == LOOPINSOCKETS
          ||node->getNodeType() == LOOPOUTSOCKETS)
    {

        LoopSocketNode *lsnode = (LoopSocketNode*)node;
        stream<<lsnode->getLoopedSocketsCount();
        foreach(DSocket *socket, lsnode->getLoopedSockets())
        {
            stream<<socket->getID()<<lsnode->getPartnerSocket(socket)->getID();
        }

    }

    if(node->getNodeType() == COLORNODE)
    {
        ColorValueNode *colornode = (ColorValueNode*)node;
        stream<<colornode->getValue();
    }

    if(node->getNodeType() == FLOATNODE)
    {
        FloatValueNode *floatnode = (FloatValueNode*)node;
        stream<<floatnode->getValue();
    }

    if(node->getNodeType() == STRINGNODE)
    {
        StringValueNode *stringnode = (StringValueNode*)node;
        stream<<stringnode->getValue();
    }

    if(node->getNodeType() == VECTORNODE)
    {
        VectorValueNode *vectornode = (VectorValueNode*)node;
        stream<<vectornode->getValue().x<<vectornode->getValue().y<<vectornode->getValue().z;
    }

    return stream;
}

DNode *DNode::newNode(QString name, NType t, int insize, int outsize)
{
    DNode *node;
    if(t==CONTAINER)
        node = new ContainerNode(name, true);
    else if (t == FOR)
        node = new ForNode(name, true);
    else if (t == WHILE)
        node = new WhileNode(name, true);
    else if (t == GATHER)
        node = new GatherNode(name, true);
    else if (t == SOLAR)
        node = new SolarNode(name, true);
    else if (t == ILLUMINATE)
        node = new IlluminateNode(name, true);
    else if (t == ILLUMINANCE)
        node = new IlluminanceNode(name, true);
    else if (t == CONDITIONCONTAINER)
        node = new ConditionContainerNode(name, true);
    else if(t == FUNCTION)
        node = new FunctionNode();
    else if(t == MULTIPLY
            ||t == DIVIDE
            ||t == ADD
            ||t == SUBTRACT
            ||t == DOTPRODUCT)
        node = new MathNode(t, true);
    else if(t == GREATERTHAN
            ||t == SMALLERTHAN
            ||t == EQUAL
            ||t == AND
            ||t == OR
            ||t == NOT)
        node = new ConditionNode(t, true);
    else if(t == COLORNODE)
        node = new ColorValueNode(name, true);
    else if(t == FLOATNODE)
        node = new FloatValueNode(name, true);
    else if(t == STRINGNODE)
        node = new StringValueNode(name, true);
    else if(t == VECTORNODE)
        node = new VectorValueNode(name, true);
    else if(t == INSOCKETS
            ||t == OUTSOCKETS)
        node = new SocketNode(insize == 0 ? OUT : IN, 0, true);
    else if(t == LOOPINSOCKETS
            ||t == LOOPOUTSOCKETS)
        node = new LoopSocketNode(insize == 0 ? OUT : IN, 0, true);
    else if(t == SURFACEOUTPUT
            ||t == DISPLACEMENTOUTPUT
            ||t == VOLUMEOUTPUT
            ||t == LIGHTOUTPUT)
        node = new OutputNode();
    else if(t == SURFACEINPUT
            ||t == DISPLACEMENTINPUT
            ||t == VOLUMEINPUT
            ||t == LIGHTINPUT)
        node = new InputNode();
    else
        node = new DNode();

    node->setNodeName(name);
    node->setNodeType(t);
    return node;
}

QDataStream &operator >>(QDataStream &stream, DNode  **node)
{
    QString name;
    unsigned short ID;
    qint16 insocketsize, outsocketsize;
    int nodetype;
    QPointF nodepos;
    stream>>name>>ID>>nodetype>>nodepos;
    stream>>insocketsize>>outsocketsize;

    DNode *newnode;
    newnode = DNode::newNode(name, (NType)nodetype, insocketsize, outsocketsize);
    LoadNodeIDMapper::setID(newnode, ID);
    //FRG::CurrentProject->setNodePosition(newnode, nodepos);
    *node = newnode;

    DSocket *socket;
    for(int i=0; i<insocketsize; i++)
    {
        stream>>&socket;
        newnode->addSocket(socket);
    }
    for(int j=0; j<outsocketsize; j++)
    {
        stream>>&socket;
        newnode->addSocket(socket);
    }

    if(newnode->getNodeType() == FUNCTION)
    {
        FunctionNode *fnode = (FunctionNode*) newnode;
        QString fname;
        stream>>fname;
        fnode->setFunctionName(fname);
    }

    unsigned short inSocketID, outSocketID, keyID, valueID;
    int smapsize;
    if(newnode->isContainer())
    {
        ContainerNode *contnode = (ContainerNode*)newnode;
        stream>>inSocketID>>outSocketID;
        stream>>smapsize;
        QPair<unsigned short, unsigned short>cont_socket_map_ID_mapper[smapsize];
        for(int i = 0; i < smapsize; i++)
        {
            stream>>keyID>>valueID;
            cont_socket_map_ID_mapper[i].first = keyID;
            cont_socket_map_ID_mapper[i].second = valueID;
        }
        DNSpace *space = new ContainerSpace();
        stream>>&space;
        contnode->setContainerData(static_cast<ContainerSpace*>(space));
        SocketNode *innode = static_cast<SocketNode*>(LoadNodeIDMapper::getNode(inSocketID));
        contnode->setInputs(innode);
        SocketNode *outnode = static_cast<SocketNode*>(LoadNodeIDMapper::getNode(outSocketID));
        contnode->setOutputs(outnode);
        for(int j = 0; j < smapsize; j++)
        {
            keyID = cont_socket_map_ID_mapper[j].first;
            valueID = cont_socket_map_ID_mapper[j].second;
            contnode->mapOnToIn(LoadSocketIDMapper::getSocket(keyID), LoadSocketIDMapper::getSocket(valueID));
        }
        if(LoopNode::isLoopNode(newnode))
        {
            LoopSocketNode *loutnode = (LoopSocketNode*) outnode;
            LoopSocketNode *linnode = (LoopSocketNode*) innode;
            loutnode->setPartner(linnode);
        }
    }

    if(newnode->getNodeType() == COLORNODE)
    {
        ColorValueNode *colornode = (ColorValueNode*)*node;
        QColor color;
        stream>>color;
        colornode->setValue(color);
    }

    if(newnode->getNodeType() == FLOATNODE)
    {
        FloatValueNode *floatnode = (FloatValueNode*)*node;
        float fval;
        stream>>fval;
        floatnode->setValue(fval);
    }

    if(newnode->getNodeType() == STRINGNODE)
    {
        StringValueNode *stringnode = (StringValueNode*)*node;
        QString string;
        stream>>string;
        stringnode->setValue(string);
    }

    if(newnode->getNodeType() == VECTORNODE)
    {
        VectorValueNode *vectornode = (VectorValueNode*)newnode;
        vector vec = vectornode->getValue();
        stream>>vec.x>>vec.y>>vec.z;
    }


    if(newnode->getNodeType() == LOOPINSOCKETS
            ||newnode->getNodeType() == LOOPOUTSOCKETS)
    {
        LoopSocketNode *lsnode = (LoopSocketNode*)newnode;
        int partnerSockets, socketID, partnerID;
        stream>>partnerSockets;
        for(int i = 0; i < partnerSockets; i++)
        {
            stream>>socketID>>partnerID;
            DSocket *socket = LoadSocketIDMapper::getSocket(socketID);
            DSocket *partner = LoadSocketIDMapper::getSocket(partnerID);
            lsnode->mapPartner(socket, partner);
        }
    }

    return stream;
}

bool DNode::operator==(DNode &node)
{
    if(getNodeType() != node.getNodeType())
        return false;

    if(nodeName != node.nodeName)
        return false;

    if(inSockets.size() != node.inSockets.size()
            ||outSockets.size() != node.outSockets.size())
        return false;

    for(int i=0; i<inSockets.size(); i++)
        if(inSockets.at(i) != node.inSockets.at(i))
            return false;

    for(int i=0; i<outSockets.size(); i++)
        if(outSockets.at(i) != outSockets.at(i))
            return false;
    return true;
}

bool DNode::operator!=(DNode &node)
{
    return (!(*this == node));
}

void DNode::setNodeType(NType t)
{
    NodeType = t;
}

void DNode::setNodeName(QString name)
{
    nodeName = name;
}

void DNode::addSocket(DSocket *socket)
{
    socket->setNode(this);
    if(socket->getDir()==IN) inSockets.append((DinSocket*)socket);
    else 
	{
		outSockets.append((DoutSocket*)socket);
		setSocketVarName((DoutSocket*)socket);
	}
    if(nodeVis)
        nodeVis->recalcNodeVis();
}

void DNode::removeSocket(DSocket *socket)
{
    if(socket->getDir() == IN)
        inSockets.removeAll((DinSocket*)socket);
    else
        outSockets.removeAll((DoutSocket*)socket);
    socket->killSocketVis();
    delete socket;
}

void DNode::dec_var_socket(DSocket *socket)
{
    removeSocket(socket);
    varcnt -= 1;
}

void DNode::inc_var_socket()
{
    lastsocket = varsocket;
    if(lastsocket->getDir() == IN)
        varsocket = new DinSocket("Add Socket", VARIABLE);
    else
        varsocket = new DoutSocket("Add Socket", VARIABLE);
    varsocket->setVariable(true);
    addSocket(varsocket);
    varcnt +=1;
}

void DNode::setSocketVarName(DoutSocket *socket)
{
    if(socket->getDir() == IN)
        return;

    if(NodeType == SURFACEINPUT
       ||NodeType == DISPLACEMENTINPUT
       ||NodeType == VOLUMEINPUT
       ||NodeType == LIGHTINPUT)
    {
        socket->setVarName(socket->getName());
        return;
    }
    if((isContainer() && inSockets.isEmpty()))
    {
        if(socket->getName().startsWith('-'))
            socket->getVarName() = socket->getName().replace(0, 1, "neg");
        socket->setVarName(socket->getName());
        return;
    }

    if((NodeType == INSOCKETS
        ||NodeType == LOOPINSOCKETS))
    {
        socket->setVarName(socket->getName());
        return;
    }

    if(NodeType == FUNCTION)
    {
        socket->setVarName(socket->getName());
        return;
    }

    QString varname_raw = socket->getName().replace(" ", "_");
    if(DinSocket::SocketNameCnt.contains(varname_raw))
    {
        DinSocket::SocketNameCnt[varname_raw]++;
        socket->setVarName(varname_raw + QString::number(DinSocket::SocketNameCnt[varname_raw]));
    }
    else
    {
        DinSocket::SocketNameCnt.insert(varname_raw, 1);
        socket->setVarName(varname_raw);
    }
}

void DNode::clearSocketLinks()
{
    foreach(DinSocket *socket, inSockets)
       socket->clearLink();
}

bool DNode::isGhost()
{
    return ghost;
}

QString DNode::getNodeName()
{
    return nodeName;
}

unsigned short DNode::getID()
{
    return ID;
}

void DNode::setID(unsigned short value)
{
    ID = value;
}

VNode* DNode::getNodeVis()
{
    return nodeVis;
}

void DNode::setNodeVis(VNode* value)
{
    nodeVis = value;
}

DoutSocketList DNode::getOutSockets()
{
    return outSockets;
}

void DNode::setOutSockets(DoutSocketList value)
{
    outSockets = value;
}

DinSocketList DNode::getInSockets()
{
    return inSockets;
}

void DNode::setInSockets(DinSocketList value)
{
    inSockets = value;
}

NType DNode::getNodeType()
{
    return NodeType;
}

DSocket* DNode::getVarSocket()
{
    return varsocket;
}

void DNode::setVarSocket(DSocket* value)
{
    varsocket = value;
}

DSocket* DNode::getLastSocket()
{
    return lastsocket;
}

void DNode::setLastSocket(DSocket* value)
{
    lastsocket = value;
}

int DNode::getVarcnt()
{
    return varcnt;
}

void DNode::setVarcnt(int value)
{
    varcnt = value;
}

DNSpace* DNode::getSpace()
{
    return space;
}

void DNode::setSpace(DNSpace* value)
{
    space = value;
}

void DNode::setsurfaceInput(DNode *node)
{
    node->addSocket(new DoutSocket("P", POINT));
    node->addSocket(new DoutSocket("N", NORMAL));
    node->addSocket(new DoutSocket("Cs", COLOR));
    node->addSocket(new DoutSocket("Os", COLOR));
    node->addSocket(new DoutSocket("u", FLOAT));
    node->addSocket(new DoutSocket("v", FLOAT));
    node->addSocket(new DoutSocket("du", FLOAT));
    node->addSocket(new DoutSocket("dv", FLOAT));
    node->addSocket(new DoutSocket("s", FLOAT));
    node->addSocket(new DoutSocket("t", FLOAT));
    node->addSocket(new DoutSocket("I", VECTOR));
    node->setNodeName("Surface Input");
    node->setNodeType(SURFACEINPUT);
}

void DNode::setdisplacementInput(DNode *node)
{
    node->addSocket(new DoutSocket("P", POINT));
    node->addSocket(new DoutSocket("N", NORMAL));
    node->addSocket(new DoutSocket("u", FLOAT));
    node->addSocket(new DoutSocket("v", FLOAT));
    node->addSocket(new DoutSocket("du", FLOAT));
    node->addSocket(new DoutSocket("dv", FLOAT));
    node->addSocket(new DoutSocket("s", FLOAT));
    node->addSocket(new DoutSocket("t", FLOAT));
    node->setNodeName("Displacement Input");
    node->setNodeType(DISPLACEMENTINPUT);
}

void DNode::setvolumeInput(DNode *node)
{
    node->addSocket(new DoutSocket("P", POINT));
    node->addSocket(new DoutSocket("I", VECTOR));
    node->addSocket(new DoutSocket("Ci", COLOR));
    node->addSocket(new DoutSocket("Oi", COLOR));
    node->addSocket(new DoutSocket("Cs", COLOR));
    node->addSocket(new DoutSocket("Os", COLOR));
    node->addSocket(new DoutSocket("L", VECTOR));
    node->addSocket(new DoutSocket("Cl", COLOR));
    node->setNodeName("Volume Input");
    node->setNodeType(VOLUMEINPUT);
}

void DNode::setlightInput(DNode *node)
{
    node->addSocket(new DoutSocket("P", POINT));
    node->addSocket(new DoutSocket("Ps", POINT));
    node->addSocket(new DoutSocket("L", VECTOR));
    node->setNodeName("Light Input");
    node->setNodeType(LIGHTINPUT);
}

void DNode::setsurfaceOutput(DNode *node)
{
    node->addSocket(new DinSocket("Ci", COLOR));
    node->addSocket(new DinSocket("Oi", COLOR));
    node->setNodeName("Surface Output");
    node->setNodeType(SURFACEOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setdisplacementOutput(DNode *node)
{
    node->addSocket(new DinSocket("P", POINT));
    node->addSocket(new DinSocket("N", NORMAL));
    node->setNodeName("Displacement Output");
    node->setNodeType(DISPLACEMENTOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setvolumeOutput(DNode *node)
{
    node->addSocket(new DinSocket("Ci", COLOR));
    node->addSocket(new DinSocket("Oi", COLOR));
    node->setNodeName("Volume Output");
    node->setNodeType(VOLUMEOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setlightOutput(DNode *node)
{
    node->addSocket(new DinSocket("Cl", COLOR));
    node->setNodeName("Light Output");
    node->setNodeType(LIGHTOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setDynamicSocketsNode(socket_dir dir, socket_type t)
{
    varsocket = dir == IN ? (DSocket*)new DinSocket("Add Socket", t) : (DSocket*)new DoutSocket("Add Socket", t);
    varsocket->setVariable(true);
    addSocket(varsocket);
}

bool DNode::isInput(DNode *node)
{
    if(node->getNodeType() == SURFACEINPUT
       ||node->getNodeType() == DISPLACEMENTINPUT
       ||node->getNodeType() == VOLUMEINPUT
       ||node->getNodeType() == LIGHTINPUT)
        return true;
    else
        return false;
}

DNode *DNode::dropNode(QString filepath)
{
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    DNode *node;
    stream>>&node;
    
    file.close();

    LoadNodeIDMapper::clear();
    LoadSocketIDMapper::clear();

	FRG::SpaceDataInFocus->addNode(node);
    return node;
}

ContainerNode *DNode::buildContainerNode(QList<DNode*>nodes)
{
    int SPACING = 200;

    if(nodes.isEmpty()) return 0;
    ContainerNode *contnode = new ContainerNode("New Node", false);

    QList<DNodeLink*>ins = FRG::CurrentProject->getInLinks(FRG::Space->selectedNodes());
    QList<DNodeLink*>outs = FRG::CurrentProject->getOutLinks(FRG::Space->selectedNodes());

    FRG::CurrentProject->setNodePosition(contnode, FRG::Space->getCenter(nodes));
    FRG::Space->centerNodes(nodes);
    float minX = 0, maxX = 0;
    foreach(DNode *node, nodes)
    {
        int nodeWidth = node->getNodeVis()->getNodeWidth();
        float nodeX = FRG::CurrentProject->getNodePosition(node).x();
        float nodeMinX = nodeX - nodeWidth/2;
        float nodeMaxX = nodeX + nodeWidth/2;
        node->getSpace()->unregisterNode(node);
        contnode->getContainerData()->addNode(node);
        if(minX > nodeMinX) minX = nodeMinX;
        if(maxX < nodeMaxX) maxX = nodeMaxX;
    }

    FRG::CurrentProject->setNodePosition(contnode->getInputs(), QPointF(minX - SPACING, 0));
    FRG::CurrentProject->setNodePosition(contnode->getOutputs(), QPointF(maxX + SPACING, 0));
    foreach(DNodeLink *nld, ins)
    {
        //relink the first input of the slected nodes to the container input node
        DoutSocket *entryInContainer = static_cast<DoutSocket*>(contnode->getInputs()->getVarSocket());
        nld->in->addLink(entryInContainer);

        //link the generated mapped socket of the container input to the output previously conntected
        DinSocket *containerInput = static_cast<DinSocket*>(contnode->getSocketOnContainer(entryInContainer));
        containerInput->addLink(nld->out);
    }

    foreach(DNodeLink *nld, outs)
    {
        //link the container output node to the last output of the selected nodes
        DinSocket *exitInContainer = static_cast<DinSocket*>(contnode->getOutputs()->getVarSocket());
        exitInContainer->addLink(nld->out);

        //relink the input previously connected to the generated mapped socket of the output of the container
        nld->in->addLink(static_cast<DoutSocket*>(contnode->getSocketOnContainer(exitInContainer)));
    }

    FRG::SpaceDataInFocus->addNode(contnode);
    return contnode;
}

void DNode::unpackContainerNode(DNode *node)
{
    ContainerNode *contnode = static_cast<ContainerNode*>(node);
    QList<DNodeLink*>ins = FRG::CurrentProject->getInLinks(contnode);
    QList<DNodeLink*>outs = FRG::CurrentProject->getOutLinks(contnode);

    QList<DNodeLink> newInLinks;
    QList<DNodeLink> newOutLinks;

    foreach(DNodeLink *in, ins)
    {
        DoutSocket *inContainer = static_cast<DoutSocket*>(contnode->getSocketInContainer(in->in));
        QList<DinSocket*> newInSockets = contnode->getContainerData()->getConnected(inContainer);
        foreach(DinSocket *newInSocket, newInSockets)
            newInLinks.append(DNodeLink(newInSocket, in->out));
    }

    foreach(DNodeLink *out, outs)
    {
        DinSocket *inContainer = static_cast<DinSocket*>(contnode->getSocketInContainer(out->out));
        DoutSocket *newOutSocket = inContainer->getCntdSocket();
        newOutLinks.append(DNodeLink(out->in, newOutSocket));
    }

    QList<DNode*>nodes(contnode->getContainerData()->getNodes());
    QPointF contNodePos = FRG::CurrentProject->getNodePosition(contnode);
    foreach(DNode *node, nodes)
    {
        if(node == contnode->getInputs()
            ||node == contnode->getOutputs())
            continue;

        node->getSpace()->unregisterNode(node);
        FRG::SpaceDataInFocus->addNode(node);
        VNode *nodeVis = node->createNodeVis();
        FRG::Space->addItem(nodeVis);
        QPointF oldPos = FRG::CurrentProject->getNodePosition(node);
        QPointF newPos = contNodePos + oldPos;
        FRG::CurrentProject->setNodePosition(node, newPos);
        nodeVis->setPos(newPos);
        nodeVis->setSelected(true);
    }

    foreach(DNodeLink in, newInLinks)
        in.in->setCntdSocket(in.out);

    foreach(DNodeLink out, newOutLinks)
        out.in->setCntdSocket(out.out);

    FRG::SpaceDataInFocus->unregisterNode(node);
}

FunctionNode::FunctionNode(QString name)
    : DNode(name)
{}

FunctionNode::FunctionNode(FunctionNode* node)
    : DNode(node), function_name(node->getFunctionName())
{
}

QString FunctionNode::getFunctionName()
{
    return function_name;
}

void FunctionNode::setFunctionName(QString value)
{
    function_name = value;
}

bool FunctionNode::operator==(DNode &node)
{
    FunctionNode &fnode = dynamic_cast<FunctionNode&>(node);
    if(function_name != fnode.function_name)
        return false;
    return true;
}

ContainerNode::ContainerNode(QString name, bool raw)
    : DNode(name)
{
    setNodeType(CONTAINER);
    if(!raw)
    {
        setContainerData(new ContainerSpace);
        containerData->setName(name);
        new SocketNode(IN, this);
        new SocketNode(OUT, this);
    }
}

ContainerNode::ContainerNode(ContainerNode* node)
    : DNode(node)
{
    setNodeType(CONTAINER);
    setContainerData(new ContainerSpace(node->getContainerData()));
    foreach(DNode *node, containerData->getNodes())
    {
        if(node->getNodeType() == INSOCKETS
                    ||node->getNodeType() == LOOPINSOCKETS)
                setInputs(node);
        if(node->getNodeType() == OUTSOCKETS
                ||node->getNodeType() == LOOPOUTSOCKETS)
            setOutputs(node);
    }
    foreach(DSocket *socket, node->getMappedSocketsOnContainer())
        mapOnToIn(CopySocketMapper::getCopy(socket), CopySocketMapper::getCopy(node->getSocketInContainer(socket)));
}

ContainerNode::~ContainerNode()
{
    socket_map.clear();
    if(containerData) delete containerData;
}


VNode* ContainerNode::createNodeVis()
{
    setNodeVis(new VContainerNode(this));
    //setNodeVis(new VNode(this));
    return getNodeVis();
}

void ContainerNode::addMappedSocket(DSocket *socket)
{
    addSocket(socket);
    DSocket *mapped_socket = 0;
    if(socket->getDir() == IN)
    {
        mapped_socket = (DSocket*)new DoutSocket(socket->getName(), socket->getType());
        inSocketNode->addSocket(mapped_socket);
    }
    else
    {
        mapped_socket = (DSocket*)new DinSocket(socket->getName(), socket->getType());
        outSocketNode->addSocket(mapped_socket);
    }
    mapOnToIn(socket, mapped_socket);     
}

void ContainerNode::addtolib()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QString filename;
    filename.append("nodes/");
    filename.append(getNodeName());
    filename.append(".node");
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out<<this;
}

void ContainerNode::C_addItems(QList<DNode *> nodes)
{
    foreach(DNode *node, nodes)
        containerData->addNode(node);
}

void ContainerNode::setInputs(SocketNode *inputNode)
{
    inSocketNode = inputNode;
    if(!containerData->getNodes().contains(inputNode))
        containerData->addNode(inputNode);
    inputNode->connectToContainer(this);
}

void ContainerNode::setInputs(DNode *inputNode)    
{
    setInputs(static_cast<SocketNode*>(inputNode));
}

void ContainerNode::setOutputs(SocketNode *outputNode)
{
    outSocketNode = outputNode;
    containerData->addNode(outputNode);
    outputNode->connectToContainer(this);
}

void ContainerNode::setOutputs(DNode *outputNode)    
{
    setOutputs(static_cast<SocketNode*>(outputNode));
}

SocketNode* ContainerNode::getInputs()
{
    return inSocketNode;
}

SocketNode* ContainerNode::getOutputs()
{
    return outSocketNode;
}

void ContainerNode::newSocket(DSocket *socket)
{
    DSocket *newsocket = 0;
    if(socket->getDir() == IN)
        newsocket = new DoutSocket(socket->getName(), socket->getType());
    else
        newsocket = new DinSocket(socket->getName(), socket->getType());
    newsocket->setVariable(false);
    mapOnToIn(socket, newsocket);
    addSocket(newsocket);
}

void ContainerNode::killSocket(DSocket *socket)
{
    DSocket *contsocket = socket_map.value(socket);
    socket_map.remove(socket);
    removeSocket(contsocket);
}

DSocket *ContainerNode::getSocketInContainer(DSocket *socket)
{
    return socket_map.value(socket);
}

void ContainerNode::mapOnToIn(DSocket *on, DSocket *in)    
{
    socket_map.insert(on, in);
}

DSocket *ContainerNode::getSocketOnContainer(DSocket *socket)
{
    return socket_map.key(socket);
}

QList<DSocket*> ContainerNode::getMappedSocketsOnContainer()    
{
    return socket_map.keys();
}

int ContainerNode::getSocketMapSize()    
{
    return socket_map.size();
}

void ContainerNode::setNodeName(QString name)
{
    DNode::setNodeName(name);
    containerData->setName(name);
}

ContainerSpace* ContainerNode::getContainerData()
{
    return containerData;
}

void ContainerNode::setContainerData(ContainerSpace* value)
{
    containerData = value;
    containerData->setContainer(this);
}

//void ContainerNode::setSocketVarName(NSocket *socket)
//{
//    socket->varname = ""; //is ignored in code creation4
//}

bool ContainerNode::operator==(DNode &node)
{
    ContainerNode &cnode = dynamic_cast<ContainerNode&>(node);
    if(*containerData != *cnode.containerData)
        return false;
    return true;
}

ConditionContainerNode::ConditionContainerNode(QString name, bool raw)
    : ContainerNode(name, true)
{
    setNodeType(CONDITIONCONTAINER);
    if(!raw)
    {
        new SocketNode(IN, this);
        new SocketNode(OUT, this);
        addSocket(new DinSocket("Condition", CONDITION));
    }
}

ConditionContainerNode::ConditionContainerNode(ConditionContainerNode* node)
    : ContainerNode(node)
{
    setNodeType(CONDITIONCONTAINER);
}

SocketNode::SocketNode(socket_dir dir, ContainerNode *contnode, bool raw)
{
    if (dir == IN)
    {
        setNodeType(INSOCKETS);
        if(!raw && contnode)setInSocketNode(contnode);
    }
    else
    {
        setNodeType(OUTSOCKETS);
        if(!raw && contnode)setOutSocketNode(contnode);
    }
}

SocketNode::SocketNode(SocketNode* node)
    : DNode(node)
{
}

void SocketNode::setInSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(OUT);
    setNodeName("Input");
    contnode->setInputs(this);
}

void SocketNode::setOutSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(IN);
    setNodeName("Output");
    contnode->setOutputs(this);
}

void SocketNode::connectToContainer(ContainerNode *contnode)
{
    container = contnode;
}

void SocketNode::inc_var_socket()
{
    DNode::inc_var_socket();
	DSocket *newsocket;
	if(getLastSocket()->getDir() == IN)
		newsocket = new DoutSocket(getLastSocket()->getName(), getLastSocket()->getType());
	else
		newsocket = new DinSocket(getLastSocket()->getName(), getLastSocket()->getType());
    container->mapOnToIn(newsocket, getLastSocket());
    container->addSocket(newsocket);
}

void SocketNode::dec_var_socket(DSocket *socket)
{
    DNode::dec_var_socket(socket);
    container->removeSocket(container->getSocketOnContainer(socket));
}

//void SocketNode::setSocketVarName(NSocket *socket)
//{
//    socket->varname = ""; //Is ignored in conde creation
//}


LoopSocketNode::LoopSocketNode(socket_dir dir, ContainerNode *contnode, bool raw)
    :SocketNode(dir, contnode, raw)
{
    if (dir == IN)
    {
        setNodeType(LOOPINSOCKETS);
    }
    else
    {
        setNodeType(LOOPOUTSOCKETS);
    }

}

LoopSocketNode::LoopSocketNode(LoopSocketNode* node)
    : SocketNode(node)
{
    foreach(DSocket *original, node->getLoopedSockets())
        loopSocketMap.insert(CopySocketMapper::getCopy(original), CopySocketMapper::getCopy(node->getPartnerSocket(original)));        
}

void LoopSocketNode::dec_var_socket(DinSocket *socket)
{
    SocketNode::dec_var_socket(socket);
    if(partner != 0)
    {
        deletePartnerSocket(socket);
    }
}

QList<DSocket*> LoopSocketNode::getLoopedSockets()    
{
    return loopSocketMap.keys();
}

qint16 LoopSocketNode::getLoopedSocketsCount()    
{
    return loopSocketMap.size();
}

void LoopSocketNode::createPartnerSocket(DSocket *socket)
{
	DSocket *partnerSocket;
	if(socket->getDir() == IN)
		partnerSocket = new DoutSocket(socket->getName(), socket->getType());
	else
		partnerSocket = new DinSocket(socket->getName(), socket->getType());
    partner->addSocket(partnerSocket);
    loopSocketMap.insert(socket, partnerSocket);
}

void LoopSocketNode::deletePartnerSocket(DSocket *socket)
{
    DinSocket *partnerSocket = (DinSocket*)getPartnerSocket(socket);
    partner->removeSocket(partnerSocket);
    loopSocketMap.remove(socket);
}

void LoopSocketNode::mapPartner(DSocket* here, DSocket *partner)    
{
   loopSocketMap.insert(here, partner); 
}

void LoopSocketNode::setPartner(LoopSocketNode *p)
{
    partner = p;
}

DSocket *LoopSocketNode::getPartnerSocket(DSocket *socket)
{
    return loopSocketMap.value(socket);
}

void LoopSocketNode::addSocket(DSocket *socket)
{
    DNode::addSocket(socket);
}

void LoopSocketNode::inc_var_socket()
{
    SocketNode::inc_var_socket();
    if(partner) createPartnerSocket(getLastSocket());
}

ConditionNode::ConditionNode(NType t, bool raw)
{
    setNodeType(t);

    if(!raw)
    {
        addSocket(new DoutSocket("Output", CONDITION));
        if(getNodeType() != AND && getNodeType() != OR)
        {
            addSocket(new DinSocket("Input", getNodeType() == NOT ? CONDITION : VARIABLE));
            if(getNodeType() != NOT)
                addSocket(new DinSocket("Input", VARIABLE));
        }
        switch(getNodeType())
        {
        case GREATERTHAN:
            setNodeName("Greater Than");
            break;
        case SMALLERTHAN:
            setNodeName("Smaller Than");
            break;
        case EQUAL:
            setNodeName("Equal");
            break;
        case NOT:
            setNodeName("Not");
            break;
        case AND:
            setNodeName("And");
            setDynamicSocketsNode(IN, CONDITION);
            break;
        case OR:
            setNodeName("Or");
            setDynamicSocketsNode(IN, CONDITION);
            break;
        }
    }
}

ConditionNode::ConditionNode(ConditionNode* node)
    : DNode(node)
{
}

MathNode::MathNode(NType t, bool raw)
{
    setNodeType(t);
    switch(t)
    {
    case ADD:
        setNodeName("Add");
        break;
    case SUBTRACT:
        setNodeName("Subtract");
        break;
    case MULTIPLY:
        setNodeName("Multiply");
        break;
    case DIVIDE:
        setNodeName("Divide");
        break;
    case DOTPRODUCT:
        setNodeName("Dot Product");
        break;
    }

    if(!raw)
    {
        setDynamicSocketsNode(IN);
        DoutSocket *result = new DoutSocket("Result", VARIABLE);
        addSocket(result);
    }
}

MathNode::MathNode(MathNode* node)
    : DNode(node)
{
}

void MathNode::inc_var_socket()
{
    DNode::inc_var_socket();
//    varsocket->type = lastsocket->type;
    getOutSockets().first()->setType(getInSockets().first()->getType());
}

void MathNode::dec_var_socket(DSocket *socket)
{
    DNode::dec_var_socket(socket);
    DoutSocket *outsocket = getOutSockets().first();
    if(getVarcnt() == 0)
    {
        outsocket->setType(VARIABLE);
        getVarSocket()->setType(VARIABLE);
    }
}

bool DNode::isMathNode(DNode *node)
{
    return node->getNodeType() == ADD
       || node->getNodeType() == SUBTRACT
       || node->getNodeType() == MULTIPLY
       || node->getNodeType() == DIVIDE;
}

ValueNode::ValueNode(QString name)
    : DNode(name), shaderInput(false)
{
}

ValueNode::ValueNode(ValueNode* node)
    : DNode(node), shaderInput(node->isShaderInput())
{
}

VNode* ValueNode::createNodeVis()
{
    setNodeVis(new VValueNode(this));
    return getNodeVis();
}

void ValueNode::setShaderInput(bool si)
{
    shaderInput = si;
}

bool ValueNode::isShaderInput()
{
    return shaderInput;
}

ColorValueNode::ColorValueNode(QString name, bool raw)
    : ValueNode(name)
{
    setNodeType(COLORNODE);
    setNodeName("Color Value");
    if(!raw)
    {
        addSocket(new DoutSocket("Color", COLOR));
    }
}

ColorValueNode::ColorValueNode(ColorValueNode* node)
    : ValueNode(node), colorvalue(node->getValue())
{
}

VNode* ColorValueNode::createNodeVis()
{
    setNodeVis(new VColorValueNode(this));
    return getNodeVis();
}

void ColorValueNode::setValue(QColor newvalue)
{
    colorvalue = newvalue;
}

QColor ColorValueNode::getValue()
{
    return colorvalue;
}

bool ColorValueNode::operator ==(DNode &node)
{
    ColorValueNode &clnode = dynamic_cast<ColorValueNode&>(node);
    if(colorvalue != clnode.colorvalue)
        return false;
    return true;
}

StringValueNode::StringValueNode(QString name, bool raw)
    : ValueNode(name)
{
    setNodeType(STRINGNODE);
    setNodeName("String Value");
    if(!raw)
    {
        addSocket(new DoutSocket("String", STRING));
    }
}

StringValueNode::StringValueNode(StringValueNode* node)
    : ValueNode(node), stringvalue(node->getValue())
{
}

VNode* StringValueNode::createNodeVis()
{
    setNodeVis(new VStringValueNode(this));
    return getNodeVis();
}

void StringValueNode::setValue(QString newstring)
{
    stringvalue = newstring;
}

QString StringValueNode::getValue()
{
    return stringvalue;
}

bool StringValueNode::operator ==(DNode &node)
{
    StringValueNode &snode = dynamic_cast<StringValueNode&>(node);
    if(stringvalue != snode.stringvalue)
        return false;
    return true;
}

FloatValueNode::FloatValueNode(QString name, bool raw)
    : ValueNode(name)
{
    setNodeType(FLOATNODE);
    setNodeName("Float Value");
    if(!raw)
    {
        addSocket(new DoutSocket("Float", FLOAT));
    }
}

FloatValueNode::FloatValueNode(FloatValueNode* node)
    : ValueNode(node), floatvalue(node->getValue())
{
}

VNode* FloatValueNode::createNodeVis()
{
    setNodeVis(new VFloatValueNode(this));
    return getNodeVis();
}

void FloatValueNode::setValue(double newval)
{
    floatvalue = newval;
}

float FloatValueNode::getValue()
{
    return floatvalue;
}

bool FloatValueNode::operator==(DNode &node)
{
    FloatValueNode &flnode = dynamic_cast<FloatValueNode&>(node);
    if(floatvalue != flnode.floatvalue)
        return false;
    return true;
}

VectorValueNode::VectorValueNode(QString name, bool raw)
    :ValueNode(name)
{
    setNodeType(VECTORNODE);
    setNodeName("Vector Value");
}

VectorValueNode::VectorValueNode(VectorValueNode* node)
    : ValueNode(node), vectorvalue(node->getValue())
{
}

void VectorValueNode::setValue(vector newvalue)
{
    vectorvalue = newvalue;
}

vector VectorValueNode::getValue()    
{
    return vectorvalue;
}

bool VectorValueNode::operator==(DNode &node)
{
    return true;
}

LoopNode::LoopNode(QString name, bool raw)
    : ContainerNode(name, true)
{
    if(!raw)
    {
        LoopSocketNode *loutNode, *linNode;
        setContainerData(new ContainerSpace);
        getContainerData()->setName(name);

        linNode = new LoopSocketNode(IN, this);
        loutNode = new LoopSocketNode(OUT, this);
        loutNode->setPartner(linNode);
    }
}

LoopNode::LoopNode(LoopNode* node)
    : ContainerNode(node)
{
}

bool LoopNode::isLoopNode(DNode *node)
{
    return (node->getNodeType() == FOR
            ||node->getNodeType() == WHILE
            ||node->getNodeType() == ILLUMINANCE
            ||node->getNodeType() == ILLUMINATE
            ||node->getNodeType() == SOLAR
            ||node->getNodeType() == GATHER);
}
WhileNode::WhileNode(QString name, bool raw)
    : LoopNode(name, raw)
{
    setNodeType(WHILE);
    if(!raw)
        getOutputs()->addSocket(new DinSocket("Condition", CONDITION));
}

WhileNode::WhileNode(WhileNode* node)
    : LoopNode(node)
{
}

ForNode::ForNode(QString name, bool raw)
    : LoopNode(name, raw)
{
    setNodeType(FOR);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Start", FLOAT));
        addMappedSocket(new DinSocket("End", FLOAT));
        addMappedSocket(new DinSocket("Step", FLOAT));
    }
}

ForNode::ForNode(ForNode* node)
    : LoopNode(node)
{
}

IlluminanceNode::IlluminanceNode(QString name, bool raw)
    : LoopNode(name, raw)
{
    setNodeType(ILLUMINANCE);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Category", STRING));
        addMappedSocket(new DinSocket("Point", POINT));
        addMappedSocket(new DinSocket("Direction", VECTOR));
        addMappedSocket(new DinSocket("Angle", FLOAT));
        addMappedSocket(new DinSocket("Message Passing", STRING));
    }
}

IlluminanceNode::IlluminanceNode(IlluminanceNode* node)
    : LoopNode(node)
{
}

IlluminateNode::IlluminateNode(QString name, bool raw)
    : LoopNode(name, raw)
{
    setNodeType(ILLUMINATE);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Point", POINT));
        addMappedSocket(new DinSocket("Direction", VECTOR));
        addMappedSocket(new DinSocket("Angle", FLOAT));
    }
}

IlluminateNode::IlluminateNode(IlluminateNode* node)
    : LoopNode(node)
{
}

GatherNode::GatherNode(QString name, bool raw)
    : LoopNode(name, raw)
{
    setNodeType(GATHER);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Category", STRING));
        addMappedSocket(new DinSocket("Point", POINT));
        addMappedSocket(new DinSocket("Direction", VECTOR));
        addMappedSocket(new DinSocket("Angle", FLOAT));
        addMappedSocket(new DinSocket("Message Passing", STRING));
        addMappedSocket(new DinSocket("Samples", FLOAT));
    }
}

GatherNode::GatherNode(GatherNode* node)
    : LoopNode(node)
{
}

SolarNode::SolarNode(QString name, bool raw)
    : LoopNode(name, raw)
{
    setNodeType(SOLAR);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Axis", VECTOR));
        addMappedSocket(new DinSocket("Angle", FLOAT));
    }
}

SolarNode::SolarNode(SolarNode* node)
    : LoopNode(node)
{
}

OutputNode::OutputNode()
    : filename("")
{
}

OutputNode::OutputNode(OutputNode* node)
    : DNode(node), filename(node->getFileName()), ShaderName(node->getShaderName())
{
}

VNode* OutputNode::createNodeVis()
{
    setNodeVis(new VOutputNode(this));
    return getNodeVis();
}

QString OutputNode::getShaderName()
{
    return ShaderName;
}

void OutputNode::writeCode()
{

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    ShaderWriter sw(this);
    QString code = sw.getCode();
    stream<<code;
    file.close();
}

QString OutputNode::getFileName()
{
    return filename;
}

void OutputNode::setFileName(QString name)
{
    filename = name;
}

void OutputNode::changeName(QString newname)
{
    QString shadertype, newnodename;
    switch(getNodeType())
    {
    case SURFACEOUTPUT:
        shadertype = "Surface Output";
        break;
    case DISPLACEMENTOUTPUT:
        shadertype = "Displacement Output";
        break;
    case VOLUMEOUTPUT:
        shadertype = "Volume Output";
        break;
    case LIGHTOUTPUT:
        shadertype = "Light Output";
        break;
	case CONTAINER:
	case FUNCTION:
	case MULTIPLY:
	case DIVIDE:
	case ADD:
	case SUBTRACT:
	case DOTPRODUCT:
	case GREATERTHAN:
	case SMALLERTHAN:
	case EQUAL:
	case AND:
	case OR:
	case CONDITIONCONTAINER:
	case NOT:
	case FOR:
	case WHILE:
	case GATHER:
	case ILLUMINANCE:
	case ILLUMINATE:
	case SOLAR:
	case SURFACEINPUT:
	case DISPLACEMENTINPUT:
	case VOLUMEINPUT:
	case LIGHTINPUT:
	case ILLUMINANCEINPUT:
	case ILLUMINATEINPUT:
	case SOLARINPUT:
	case INSOCKETS:
	case OUTSOCKETS:
	case LOOPINSOCKETS:
	case LOOPOUTSOCKETS:
	case COLORNODE:
	case FLOATNODE:
	case STRINGNODE:
	case VECTORNODE:
	break;	
    }

    ShaderName = newname;
    newnodename = shadertype;
    newnodename += " (";
    newnodename += ShaderName;
    newnodename += " )";
    setNodeName(newnodename);
}

InputNode::InputNode()
{
}

InputNode::InputNode(InputNode* node)
    : DNode(node)
{
}
