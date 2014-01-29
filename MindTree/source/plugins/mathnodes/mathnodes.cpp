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
    NodeDataBase::registerNodeType(new BuildInFactory( "ADD", "Math.Add",
                                    []{
                                        return new MathNode(NodeType("ADD"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInFactory( "SUBTRACT", "Math.Subtract",
                                    []{
                                        return new MathNode(NodeType("SUBTRACT"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInFactory( "MULTIPLY", "Math.Multiply",
                                    []{
                                        return new MathNode(NodeType("MULTIPLY"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInFactory( "DIVIDE", "Math.Divide", 
                                    []{
                                        return new MathNode(NodeType("DIVIDE"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInFactory( "DOTPRODUCT", "Math.Dot Product",
                                    []{
                                        return new MathNode(NodeType("DOTPRODUCT"));
                                    }));

    NodeDataBase::registerNodeType(new BuildInFactory( "MODULO", "Math.Modulo",
                                    []{
                                        return new MathNode(NodeType("MODULO"));
                                    }));

}

MathNode::MathNode(NodeType t, bool raw)
{
    setNodeType(t);
    if(t == "ADD") setNodeName("Add");
    else if(t == "SUBTRACT") setNodeName("Subtract");
    else if(t == "MULTIPLY") setNodeName("Multiply");
    else if(t == "DIVIDE") setNodeName("Divide");
    else if(t == "DOTPRODUCT") setNodeName("Dot Product");
    else if(t == "MODULO") setNodeName("Modulo");

    if(!raw)
    {
        setDynamicSocketsNode(IN);
        DoutSocket *out = new DoutSocket("Result", VARIABLE, this);
        DinSocket *in = getInSockets().first();
        //DSocket::nameCB(out, in);
        //DSocket::nameCB(in, out);
        //DSocket::typeCB(out, in);
        //DSocket::typeCB(in, out);
    }
}

MathNode::MathNode(const MathNode* node)
    : DNode(node)
{
    DoutSocket *out = getOutSockets().first();
    DinSocket *in = getInSockets().first();
    //DSocket::nameCB(out, in);
    //DSocket::nameCB(in, out);
    //DSocket::typeCB(out, in);
    //DSocket::typeCB(in, out);
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

