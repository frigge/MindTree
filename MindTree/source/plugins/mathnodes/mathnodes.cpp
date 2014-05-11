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

#include "data/nodes/node_db.h"
#include "data/nodes/nodetype.h"
#include "boost/python.hpp"

#include "mathnodes.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(mathnodes){
    NodeDataBase::registerNodeType(new BuildInDecorator( "ADD", "Math.Add",
                                    []{
                                        return new MathNode(NodeType("ADD"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInDecorator( "SUBTRACT", "Math.Subtract",
                                    []{
                                        return new MathNode(NodeType("SUBTRACT"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInDecorator( "MULTIPLY", "Math.Multiply",
                                    []{
                                        return new MathNode(NodeType("MULTIPLY"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInDecorator( "DIVIDE", "Math.Divide", 
                                    []{
                                        return new MathNode(NodeType("DIVIDE"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInDecorator( "DOTPRODUCT", "Math.Dot Product",
                                    []{
                                        return new MathNode(NodeType("DOTPRODUCT"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInDecorator( "MODULO", "Math.Modulo",
                                    []{
                                        return new MathNode(NodeType("MODULO"));
                                    }));

}

MathNode::MathNode(NodeType t, bool raw)
{
    setNodeType(t);
    if(t == "ADD") {
        setNodeName("Add");
    }
    else if(t == "SUBTRACT") {
        setNodeName("Subtract");
    }
    else if(t == "MULTIPLY") {
        setNodeName("Multiply");
    }
    else if(t == "DIVIDE") {
        setNodeName("Divide");
    }
    else if(t == "DOTPRODUCT") {
        setNodeName("Dot Product");
    }
    else if(t == "MODULO") {
        setNodeName("Modulo");
    }

    if(!raw) {
        auto *out = new DoutSocket("Result", "VARIABLE", this);

        if(t != "ADD" && t != "MULTIPLY") {
            new DinSocket("value1", "VARIABLE", this);
            new DinSocket("value2", "VARIABLE", this);
        }
        else {
            setDynamicSocketsNode(DSocket::IN);
            auto *varsocket = getVarSocket();
            Signal::getBoundHandler<DoutSocket*>(varsocket)
                .connect("linkChanged", [varsocket, out](DoutSocket *newConnection){
                             if(newConnection) {
                                 varsocket->setName(newConnection->getName());
                                 varsocket->setType(newConnection->getType());
                             } else {
                                 if(!newConnection && out->getNode()->getVarcnt() == 1)
                                     out->setType("VARIABLE");
                             }
                         }).detach();

            Signal::getBoundHandler<SocketType>(varsocket)
                .connect("typeChanged", [out](SocketType newtype){
                             out->setType(newtype);
                         }).detach();
        }

    }
}

MathNode::MathNode(const MathNode &node)
    : DNode(node)
{
}

void MathNode::incVarSocket()
{
    DNode::incVarSocket();
    auto *varsocket = getVarSocket();
    auto *out = getOutSockets().at(0);

    Signal::getBoundHandler<DoutSocket*>(varsocket)
        .connect("linkChanged", [varsocket, out](DoutSocket *newConnection){
                    if(newConnection) {
                        varsocket->setName(newConnection->getName());
                        varsocket->setType(newConnection->getType());
                        if(out->getType() == "VARIABLE")
                            out->setType(newConnection->getType()); 
                    } else {
                        if(!newConnection && out->getNode()->getVarcnt() == 1)
                            out->setType("VARIABLE");
                    }
                 }).detach();

    Signal::getBoundHandler<SocketType>(varsocket)
        .connect("typeChanged", [out](SocketType newtype){
                     out->setType(newtype);
                 }).detach();
}

void MathNode::decVarSocket(DSocket *socket)
{
    DNode::decVarSocket(socket);
    DoutSocket *outsocket = getOutSockets().at(0);
    if(getVarcnt() == 0)
    {
        outsocket->setType("VARIABLE");
        getVarSocket()->setType("VARIABLE");
    }
}

