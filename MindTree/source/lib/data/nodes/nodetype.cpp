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

NodeType::NodeType(std::string typeStr)
: typeString(typeStr), _id(getID(typeStr))
{
}

NodeType::NodeType(NType oldType)
    : oldType(oldType)
{
    switch(oldType)
    {
    case CONTAINER: typeString = "CONTAINER"; break;
    case FUNCTION: typeString = "FUNCTION"; break;
    case MULTIPLY: typeString = "MULTIPLY"; break;
    case DIVIDE: typeString = "DIVIDE"; break;
    case ADD: typeString = "ADD"; break;
    case SUBTRACT: typeString = "SUBTRACT"; break;
    case DOTPRODUCT: typeString = "DOTPRODUCT"; break;
    case MODULO: typeString = "MODULO"; break;
    case GREATERTHAN: typeString = "GREATERTHAN"; break;
    case SMALLERTHAN: typeString = "SMALLERTHAN"; break;
    case EQUAL: typeString = "EQUAL"; break;
    case AND: typeString = "AND"; break;
    case OR: typeString = "OR"; break;
    case CONDITIONCONTAINER: typeString = "CONDITIONCONTAINER"; break;
    case NOT: typeString = "NOT"; break;
    case FOR: typeString = "FOR"; break;
    case WHILE: typeString = "WHILE"; break;
    case GATHER: typeString = "GATHER"; break;
    case ILLUMINANCE: typeString = "ILLUMINANCE"; break;
    case ILLUMINATE: typeString = "ILLUMINATE"; break;
    case SOLAR: typeString = "SOLAR"; break;
    case SURFACEINPUT: typeString = "SURFACEINPUT"; break;
    case SURFACEOUTPUT: typeString = "SURFACEOUTPUT"; break;
    case DISPLACEMENTINPUT: typeString = "DISPLACEMENTINPUT"; break;
    case DISPLACEMENTOUTPUT: typeString = "DISPLACEMENTOUTPUT"; break;
    case VOLUMEINPUT: typeString = "VOLUMEINPUT"; break;
    case VOLUMEOUTPUT: typeString = "VOLUMEOUTPUT"; break;
    case LIGHTINPUT: typeString = "LIGHTINPUT"; break;
    case LIGHTOUTPUT: typeString = "LIGHTOUTPUT"; break;
    case ILLUMINANCEINPUT: typeString = "ILLUMINANCEINPUT"; break;
    case ILLUMINATEINPUT: typeString = "ILLUMINATEINPUT"; break;
    case SOLARINPUT: typeString = "SOLARINPUT"; break;
    case INSOCKETS: typeString = "INSOCKETS"; break;
    case OUTSOCKETS: typeString = "OUTSOCKETS"; break;
    case LOOPINSOCKETS: typeString = "LOOPINSOCKETS"; break;
    case LOOPOUTSOCKETS: typeString = "LOOPOUTSOCKETS"; break;
    case COLORNODE: typeString = "COLORNODE"; break;
    case FLOATNODE: typeString = "FLOATNODE"; break;
    case INTNODE: typeString = "INTNODE"; break;
    case BOOLNODE: typeString = "BOOLNODE"; break;
    case STRINGNODE: typeString = "STRINGNODE"; break;
    case VECTORNODE: typeString = "VECTORNODE"; break;
    case PREVIEW: typeString = "PREVIEW"; break;
    case GETARRAY: typeString = "GETARRAY"; break;
    case SETARRAY: typeString = "SETARRAY"; break;
    case COMPOSEARRAY: typeString = "COMPOSEARRAY"; break;
    case VARNAME: typeString = "VARNAME"; break;
    case OBJECTNODE: typeString = "OBJECTNODE"; break;
    case POLYGONNODE: typeString = "POLYGONNODE"; break;
    case FLOATTOVECTOR: typeString = "FLOATTOVECTOR"; break;
    case VIEWPORTNODE: typeString = "VIEWPORTNODE"; break;
    case FRAGMENTOUTPUT: typeString = "FRAGMENTOUTPUT"; break;
    case VERTEXOUTPUT: typeString = "VERTEXOUTPUT"; break;
    case GEOMETRYOUTPUT: typeString = "GEOMETRYOUTPUT"; break;
    case GLFRAGMENTINPUT: typeString = "GLFRAGMENTINPUT"; break;
    case GLVERTEXINPUT: typeString = "GLVERTEXINPUT"; break;
    case GLGEOMETRYINPUT: typeString = "GLGEOMETRYINPUT"; break;
    case TRANSFORM: typeString = "TRANSFORM"; break;
    case FOREACHNODE: typeString = "FOREACHNODE"; break;
    case READFILE: typeString = "READFILE"; break;
    case WRITEFILE: typeString = "WRITEFILE"; break;
    case PROCESS: typeString = "PROCESS"; break;
    case SAVEIMAGE: typeString = "SAVEIMAGE"; break;
    case LOADIMAGE: typeString = "LOADIMAGE"; break;
    case TEXTVIEWER: typeString = "TEXTVIEWER"; break;
    case IMAGEVIEWER: typeString = "IMAGEVIEWER"; break;
    case CAMERANODE: typeString = "CAMERANODE"; break;
    case SCENENODE: typeString = "SCENENODE"; break;
    case LIGHTNODE: typeString = "LIGHTNODE"; break;
    case COMPOSEMESHNODE: typeString = "COMPOSEMESHNODE"; break;
    case OBJIMPORTNODE: typeString = "OBJIMPORTNODE"; break;
    case SCENEGROUP: typeString = "SCENEGROUP"; break;
    }
    _id = getID(typeString);
}

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

QList<QString> NodeType::getTypes()    
{
    QList<QString> list;
    for(auto s : id_map)
        list << s.c_str();
    return list;
}

NType NodeType::getOldType()    const
{
    return oldType;
}
    
std::string NodeType::toStr()const
{
    return typeString;
}

int NodeType::id()const 
{
    return _id;
}

bool NodeType::operator==(NType t)  const
{
    return t == oldType;
}

bool NodeType::operator==(std::string t)  const  
{
    return t == typeString;
}


bool NodeType::operator==(const NodeType &other)    const
{
    return other._id == _id;
}

bool NodeType::operator!=(NType t)   const 
{
    return !((*this)==t); 
}

bool NodeType::operator!=(std::string t)   const 
{
    return !((*this)==t); 
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
