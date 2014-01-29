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
#ifndef NODETYPE_QDRSY1S8

#define NODETYPE_QDRSY1S8

#include "QHash"
namespace MindTree{

enum NType
{
    CONTAINER,          FUNCTION,
    MULTIPLY,           DIVIDE,
    ADD,                SUBTRACT,
    DOTPRODUCT,         MODULO,

    GREATERTHAN,        SMALLERTHAN,
    EQUAL,              AND,OR,
    CONDITIONCONTAINER, NOT,

    FOR, WHILE, GATHER, ILLUMINANCE,
    ILLUMINATE, SOLAR,

    SURFACEINPUT,       SURFACEOUTPUT,
    DISPLACEMENTINPUT,  DISPLACEMENTOUTPUT,
    VOLUMEINPUT,        VOLUMEOUTPUT,
    LIGHTINPUT,         LIGHTOUTPUT,
    ILLUMINANCEINPUT,   ILLUMINATEINPUT,
    SOLARINPUT,

    INSOCKETS,          OUTSOCKETS,
    LOOPINSOCKETS,      LOOPOUTSOCKETS,

    COLORNODE,          FLOATNODE,
    INTNODE, BOOLNODE,
    STRINGNODE,         VECTORNODE,
    PREVIEW, GETARRAY,  SETARRAY,
    COMPOSEARRAY,       VARNAME,
    OBJECTNODE,         POLYGONNODE,
    FLOATTOVECTOR,      VIEWPORTNODE,

    FRAGMENTOUTPUT,     VERTEXOUTPUT,
    GEOMETRYOUTPUT,
    GLFRAGMENTINPUT,    GLVERTEXINPUT,
    GLGEOMETRYINPUT,    TRANSFORM,
    FOREACHNODE,

    READFILE,           WRITEFILE,
    PROCESS, SAVEIMAGE, LOADIMAGE,
    TEXTVIEWER,         IMAGEVIEWER,
    CAMERANODE,         SCENENODE,
    LIGHTNODE,          COMPOSEMESHNODE,
    OBJIMPORTNODE,      SCENEGROUP
};

class NodeType
{
public:
    NodeType(std::string typeStr="");
    NodeType(NType oldType);
    virtual ~NodeType();
    static NodeType byID(int id);
    int id()const;
    std::string toStr()const;
    NType getOldType()const;

    static QList<QString> getTypes();

    static int registerType(std::string name);
    static void unregisterType(std::string name);
    static int getID(std::string name);

    bool operator==(NType t)const;
    bool operator==(std::string t)const;
    bool operator==(const NodeType &other)const;
    bool operator!=(NType t)const;
    bool operator!=(std::string t)const;
    bool operator!=(const NodeType &other)const;

private:
    NType oldType;
    std::string typeString;
    int _id;
    static int id_cnt;
    static std::vector<std::string> id_map;
};

};

#endif /* end of include guard: NODETYPE_QDRSY1S8 */
