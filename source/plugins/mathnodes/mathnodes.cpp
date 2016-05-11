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
using namespace std;

BOOST_PYTHON_MODULE(mathnodes){
    using ndb = NodeDataBase;
    ndb::registerNodeType(make_unique<BuildInDecorator>("ADD",
                                                        "Math.Add",
                            [] (bool raw){
                                return make_shared<MathNode>(NodeType("ADD"),
                                                                raw);
                            }));

    ndb::registerNodeType(make_unique<BuildInDecorator>("SUBTRACT",
                                                        "Math.Subtract",
                        [] (bool raw){
                            return make_shared<MathNode>(NodeType("SUBTRACT"),
                                                            raw);
                        }));

    ndb::registerNodeType(make_unique<BuildInDecorator>("MULTIPLY",
                                                        "Math.Multiply",
                        [](bool raw){
                            return make_shared<MathNode>(NodeType("MULTIPLY"),
                                                            raw);
                        }));

    ndb::registerNodeType(make_unique<BuildInDecorator>("DIVIDE",
                                                        "Math.Divide",
                            [] (bool raw){
                                return make_shared<MathNode>(NodeType("DIVIDE"),
                                                                raw);
                            }));

    ndb::registerNodeType(make_unique<BuildInDecorator>("DOTPRODUCT",
                                                        "Math.Dot Product",
                        [] (bool raw){
                            return make_shared<MathNode>(NodeType("DOTPRODUCT"),
                                                            raw);
                        }));

    ndb::registerNodeType(make_unique<BuildInDecorator>("MODULO",
                                                        "Math.Modulo",
                            [] (bool raw){
                                return make_shared<MathNode>(NodeType("MODULO"),
                                                                raw);
                            }));

}

MathNode::MathNode(NodeType t, bool raw)
{
    std::map<std::string, std::string> names = {
        { "ADD", "Add" },
        { "SUBTRACT", "Substract" },
        { "MULTIPLY", "Multiply" },
        { "DIVIDE", "Divide" },
        { "DOTPRODUCT", "Dot Product" },
        { "MODULO", "Modulo" }
    };

    setType(t);
    setName(names[t.toStr()]);

    if(!raw) {
        auto *out = new DoutSocket("Result", "VARIABLE", this);

        if(t != "ADD" && t != "MULTIPLY") {
            auto in1 = new DinSocket("value1", "VARIABLE", this);
            auto in2 = new DinSocket("value2", "VARIABLE", this);
            out->listenToChange(in1);
        }
        else {
            setDynamicSocketsNode(DSocket::IN);
            auto *varsocket = getVarSocket();
            varsocket->toIn()->listenToLinked();
            out->listenToChange(varsocket);
        }
    }
}

MathNode::MathNode(const MathNode &node)
    : DNode(node)
{
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

