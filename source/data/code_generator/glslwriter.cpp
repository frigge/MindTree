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

#include "glslwriter.h"
#include "source/data/nodes/data_node.h"

GLSLWriter::GLSLWriter(const DNode *start)
    : ShaderCodeGenerator(start)
{
    createHeader();
}

void GLSLWriter::createHeader()    
{
    init();
}

QString GLSLWriter::getCode()
{
    QString returncode;
    returncode.append(createOutputVars());
    returncode.append(createShaderParameterCode());
    returncode.append("void main()\n{");
    returncode.append("    /*Variable declarations*/");
    returncode.append(createVarDeclares());
    returncode.append("\n\n    /*Begin Code*/");
    returncode.append(getCache()->get());
    returncode.append("\n}");
    return returncode;
}

void GLSLWriter::initVar(const DSocket *socket)
{
    QString VDstr;

    switch(socket->getType())
    {
	case VARIABLE:
	case CONDITION:
	break;
    case COLOR:
        VDstr.append("vec4 ");
        break;
    case FLOAT:
         VDstr.append("float ");
        break;
    case VECTOR:
    case POINT:
    case NORMAL:
         VDstr.append("vec3 ");
        break;
    default:
        break;
    }
    VDstr.append(getVariable(socket));
    if(socket->isArray())
        VDstr.append("[" + QString::number(socket->getArrayLength()) + "]");
    VDstr.append(";");
    addToVarDeclares(VDstr);
}

bool GLSLWriter::isInputVar(const DoutSocket *socket)
{
    QStringList vars;
    foreach(QString var, vars)
    {
        if(var == getVariable(socket))
            return true;
    }
    return false;
}

void GLSLWriter::outputVar(const DinSocket *socket)
{
    QString ending, OVstr;
    switch(socket->getType())
    {
    case COLOR:
        OVstr.append("vec4 ");
        ending = "(0.0, 0.0, 0.0, 0.0)";
        break;
    case FLOAT:
         OVstr.append("float ");
         ending = "0";
	case CONDITION:
         OVstr.append("bool ");
         ending = "0";
    case INTEGER:
         OVstr.append("int ");
         ending = "0";
    case VECTOR:
    case POINT:
    case NORMAL:
         OVstr.append("vec3 ");
         ending = "(0.0, 0.0, 0.0)";
        break;
    default:
        break;
    }

    OVstr.append(socket->getName());
    OVstr.append(" = ");
    OVstr.append(ending);
    OVstr.append(";");
    addToOutputVars(OVstr);
}

QString GLSLWriter::writeColor(const DoutSocket *socket)
{
    QString output;
    QString value;
    ColorValueNode *colornode = (ColorValueNode*)socket->getNode();
    value.append("vec4(");
    QColor color = ((ColorProperty*)colornode->getInSockets().first()->getProperty())->getValue();
    value.append(QString::number(color.redF()));
    value.append(", ");
    value.append(QString::number(color.greenF()));
    value.append(", ");
    value.append(QString::number(color.blueF()));
    value.append(", ");
    value.append(QString::number(color.alphaF(), 'f', 1));
    value.append(")");
    if(colornode->isShaderInput())
    {
        output.append("vec4 ");
        output.append(getVariable(socket));
        if(socket->isArray())
            output.append("[" + QString::number(socket->getArrayLength()) + "]");
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        output = getVariable(socket);
        return output;
    }
    else
        return value;
}

QString GLSLWriter::writeVector(const DoutSocket *socket)
{
    QString output, value;
    const VectorValueNode *node = socket->getNode()->getDerivedConst<VectorValueNode>();
    value.append("vec3(");
    Vector vec = ((VectorProperty*)node->getInSockets().first()->getProperty())->getValue();
    value.append(QString::number(vec.x));
    value.append(", ");
    value.append(QString::number(vec.y));
    value.append(", ");
    value.append(QString::number(vec.z));
    value.append(")");
    if(node->isShaderInput())
    {
        output.append("vec3 ");
        output.append(getVariable(socket));
        if(socket->isArray())
            output.append("[" + QString::number(socket->getArrayLength()) + "]");
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return getVariable(socket);
    }
    else return value;
}

