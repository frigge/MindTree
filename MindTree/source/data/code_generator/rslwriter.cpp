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

#include "rslwriter.h"
#include "source/lib/data/nodes/data_node.h"
#include "outputs.h"

using namespace MindTree;

ShaderWriter::ShaderWriter(DNode *start)
    : ShaderCodeGenerator(start)
{
    createHeader();
}

void ShaderWriter::createHeader()    
{
    switch(getStart()->getNodeType())
    {
    case SURFACEOUTPUT:
    case PREVIEW:
        addToShaderHeader("surface ");
        break;
    case DISPLACEMENTOUTPUT:
        addToShaderHeader("displacement ");
        break;
    case VOLUMEOUTPUT:
        addToShaderHeader("volume ");
        break;
    case LIGHTOUTPUT:
        addToShaderHeader("light ");
        break;
    default:
        break;
    }
    addToShaderHeader(getStart()->getDerivedConst<RSLOutputNode>()->getShaderName());
    addToShaderHeader("(");
    init();
}

void ShaderWriter::initVar(const DSocket *socket)
{
    if(socket->getDir() == OUT && isInputVar(const_cast<DSocket*>(socket)->toOut()))return;

    QString VDstr;

    switch(socket->getType().getType())
    {
	case VARIABLE:
	case CONDITION:
	break;
    case COLOR:
        VDstr.append("color ");
        break;
    case FLOAT:
         VDstr.append("float ");
        break;
    case STRING:
         VDstr.append("string ");
        break;
    case VECTOR:
         VDstr.append("vector ");
        break;
    case POINT:
         VDstr.append("point ");
        break;
    case NORMAL:
         VDstr.append("normal ");
        break;
    default:
        break;
    }
    VDstr.append(getVariable(socket));
    //if(socket->isArray())
    //    VDstr.append("[" + QString::number(socket->getArrayLength()) + "]");
    VDstr.append(";");
    addToVarDeclares(VDstr);
}

bool ShaderWriter::isInputVar(const DoutSocket *socket)
{
    QStringList vars;
    vars<<"P"<<"N"<<"I"<<"s"<<"t"<<"u"<<"v"<<"Cl"<<"Cs"<<"Os"<<"L"<<"Oi"<<"Ci"<<"Ps"<<"Ns"<<"du"<<"dv";
    for(QString var : vars)
    {
        if(var == getVariable(socket))
            return true;
    }
    return false;
}

void ShaderWriter::outputVar(const DinSocket *socket)
{
    QString ending, OVstr;
    OVstr.append("output ");
    switch(socket->getType().getType())
    {
	case VARIABLE:
	break;
    case COLOR:
        OVstr.append("color ");
        ending = "(0, 0, 0)";
        break;
    case FLOAT:
	case CONDITION:
    case INTEGER:
         OVstr.append("float ");
         ending = "0";
    case STRING:
         OVstr.append("string ");
         ending = "";
        break;
    case VECTOR:
         OVstr.append("vector ");
         ending = "(0, 0, 0)";
        break;
    case POINT:
         OVstr.append("point ");
         ending = "(0, 0, 0)";
        break;
    case NORMAL:
         OVstr.append("normal ");
         ending = "(0, 0, 0)";
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

void ShaderWriter::writeCustom(const DoutSocket *socket)
{
    switch(socket->getNode()->getNodeType()){
    case GATHER:
        writeGather(socket);
        break;
    case ILLUMINANCE:
        writeIlluminance(socket);
        break;
    case ILLUMINATE:
        writeIlluminate(socket);
        break;
    case SOLAR:
        writeSolar(socket);
        break;
    default:
        break;
    }
}

void ShaderWriter::writeGather(const DoutSocket *socket)    
{
    
}

void ShaderWriter::writeIlluminance(const DoutSocket *socket)    
{
    
}

void ShaderWriter::writeIlluminate(const DoutSocket *socket)    
{
    
}

void ShaderWriter::writeSolar(const DoutSocket *socket)    
{
    
}

QString ShaderWriter::writeColor(const DoutSocket *socket)
{
    QString output;
    QString value;
    ColorValueNode *colornode = (ColorValueNode*)socket->getNode();
    value.append("color(");
    QColor color = colornode->getInSockets().first()->getSocketProperty().getData<QColor>();
    value.append(QString::number(color.redF()));
    value.append(", ");
    value.append(QString::number(color.greenF()));
    value.append(", ");
    value.append(QString::number(color.blueF()));
    value.append(")");
    if(colornode->isShaderInput())
    {
        output.append("color ");
        output.append(getVariable(socket));
        //if(socket->isArray())
        //    output.append("[" + QString::number(socket->getArrayLength()) + "]");
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

QString ShaderWriter::writeVector(const DoutSocket *socket)
{
    QString output, value;
    const VectorValueNode *node = socket->getNode()->getDerivedConst<VectorValueNode>();
    value.append("vector(");
    Vector vec = node->getInSockets().first()->getProperty().getData<Vector>();
    value.append(QString::number(vec.x));
    value.append(", ");
    value.append(QString::number(vec.y));
    value.append(", ");
    value.append(QString::number(vec.z));
    value.append(")");
    if(node->isShaderInput())
    {
        output.append("vector ");
        output.append(getVariable(socket));
        //if(socket->isArray())
        //    output.append("[" + QString::number(socket->getArrayLength()) + "]");
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return getVariable(socket);
    }
    else return value;
}

