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
#include "QString"
#include "iostream"
#include "nodetype.h"

namespace MindTree {

std::vector<std::string> NodeType::id_map;
int NodeType::id_cnt = -1;

NodeType::NodeType(const std::string &typeStr)
: typeString(typeStr), _id(getID(typeStr))
{
}

NodeType::NodeType(const char *typeStr)
    : typeString(typeStr), _id(getID(typeStr))
{}

NodeType::~NodeType()
{
}

NodeType NodeType::byID(int id)    
{
    if(id >= id_map.size()) {
        std::cout<<"NodeType not registered: "<<id<<std::endl;
        return NodeType();
    }
    return NodeType(id_map[id]);
}

std::vector<std::string> NodeType::getTypes()    
{
    return id_map;
}

std::string NodeType::toStr()const
{
    return typeString;
}

int NodeType::id()const 
{
    return _id;
}

bool NodeType::operator==(const NodeType &other)    const
{
    return other._id == _id;
}

bool NodeType::operator!=(const NodeType &other)    const
{
    return !((*this)==other);
}

int NodeType::registerType(std::string name)    
{
    id_map.push_back(name);
    return ++id_cnt;
}

void NodeType::unregisterType(std::string name)    
{
}

int NodeType::getID(std::string name)    
{
    if(name == "") return -1;
    int i = 0;
    for(auto s = id_map.begin(); s != id_map.end(); s++, i++){
        if(name == *s) return i;
    }
    return registerType(name);
}
}
