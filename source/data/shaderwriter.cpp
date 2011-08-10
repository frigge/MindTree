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


#include "shaderwriter.h"

ShaderWriter::ShaderWriter(OutputNode *start)
{
    tabLevel = 1;
    QString outputvar;

    DSocket::SocketNameCnt.clear();

    switch(start->getNodeType())
    {
    case SURFACEOUTPUT:
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

    addToShaderHeader(start->getShaderName());
    addToShaderHeader("(");

    foreach(DinSocket *socket, start->getInSockets())
    {
        if(!socket->getCntdSocket())
            continue;
        if(!start->getInSockets().startsWith(socket))
            code.append(newline());

        if(socket->getVariable())
            outputVar(socket);

        outputvar = socket->getVarName();
        outputvar.append(" = ");
        outputvar.append(writeVarName(socket));
        gotoNextNode(socket);
        code.append("\n    ");
        code.append(outputvar);
        code.append(";");
    }
}

QString ShaderWriter::newline()
{
    QString nl;
    nl.append("\n");
    for(int i=1; i<=tabLevel; i++)
        nl.append("    ");
    return nl;
}

void ShaderWriter::initVar(DoutSocket *socket)
{
    if(isInputVar(socket))return;
    QString VDstr;
    switch(socket->getType())
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
    }
    VDstr.append(socket->getVarName());
    VDstr.append(";");
    addToVarDeclares(VDstr);
}

bool ShaderWriter::isInputVar(DoutSocket *socket)
{
    QStringList vars;
    vars<<"P"<<"N"<<"I"<<"s"<<"t"<<"u"<<"v"<<"Cl"<<"Cs"<<"Os"<<"L"<<"Oi"<<"Ci"<<"Ps"<<"Ns"<<"du"<<"dv";
    foreach(QString var, vars)
    {
        if(var == socket->getVarName())
            return true;
    }
    return false;
}

void ShaderWriter::incTabLevel()
{
    tabLevel++;
}

void ShaderWriter::decTabLevel()
{
    tabLevel--;
}

void ShaderWriter::outputVar(DinSocket *socket)
{
    QString ending, OVstr;
    OVstr.append("output ");
    switch(socket->getType())
    {
	case CONDITION:
	case VARIABLE:
	break;
    case COLOR:
        OVstr.append("color ");
        ending = "(0, 0, 0)";
        break;
    case FLOAT:
         OVstr.append("float ");
         ending = "0";
        break;
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
    }

    OVstr.append(socket->getVarName());
    OVstr.append(" = ");
    OVstr.append(ending);
    OVstr.append(";");
    addToOutputVars(OVstr);
}

void ShaderWriter::gotoNextNode(DinSocket *socket)
{
    if(!socket->getCntdSocket())
        return;

    DoutSocket *nextsocket = socket->getCntdSocket();
    if(!written_sockets.contains(nextsocket->getVarName()))
    {
        evalSocketValue(nextsocket);
        written_sockets.append(nextsocket->getVarName());
    }
    return;
}

QString ShaderWriter::writeVarName(DinSocket *insocket)
{
    if(!insocket->getCntdSocket())
        return "";

    DoutSocket *prevsocket = insocket->getCntdSocket();
    DinSocket *mapsocket = 0;
    DNode *node = prevsocket->getNode();

    ContainerNode *cnode = (ContainerNode*)node;
    if(node->isContainer()
        &&!node->getInSockets().isEmpty())
    {
        mapsocket = (DinSocket*)cnode->getSocketInContainer(prevsocket);
        return writeVarName(mapsocket);
    }
    else if(node->getNodeType() == INSOCKETS
            ||node->getNodeType() == LOOPINSOCKETS)
    {
        return writeVarName(stepUp(prevsocket));
    }
    else if(node->getNodeType() == COLORNODE)
    {
        return writeColor(prevsocket);
    }
    else if(node->getNodeType() == FLOATNODE)
    {
        return writeFloat(prevsocket);
    }
    else if(node->getNodeType() == STRINGNODE)
    {
        return writeString(prevsocket);
    }
    else if(node->getNodeType() == VECTORNODE)
    {
        return writeVector(prevsocket);
    }
    else if(DNode::isMathNode(node))
    {
        return createMath(prevsocket);
    }
    else
    {
        node->setSocketVarName(prevsocket);
        return prevsocket->getVarName();
    }
}

DinSocket *ShaderWriter::stepUp(DoutSocket *socket)
{
    ContainerNode *node = static_cast<SocketNode*>(socket->getNode())->getContainer();
    DinSocket *mapsocket = static_cast<DinSocket*>(node->getSocketOnContainer(socket));
    return mapsocket;
}

QString ShaderWriter::createCondition(DoutSocket *socket)
{
    switch(socket->getNode()->getNodeType())
    {
        case NOT:
            return writeNot(socket);
            break;
        case EQUAL:
            return writeCondition(socket, "==");
            break;
        case GREATERTHAN:
            return writeCondition(socket, ">");
            break;
        case SMALLERTHAN:
            return writeCondition(socket, "<");
            break;
        default:
            break;
    }
	return "";
}

QString ShaderWriter::createMath(DoutSocket *socket)
{
    switch(socket->getNode()->getNodeType())
    {
    case MULTIPLY:
        return writeMath(socket, "*");
        break;
    case DIVIDE:
        return writeMath(socket, "/");
        break;
    case ADD:
        return writeMath(socket, "+");
        break;
    case SUBTRACT:
        return writeMath(socket, "-");
        break;
    case DOTPRODUCT:
        return writeMath(socket, ".");
        break;
    default:
        break;
    }
	return "";
}

void ShaderWriter::writeMathToVar(DoutSocket *socket)    
{
    QString output;
    output.append(newline());
    output.append(" = ");
    output.append(socket->getVarName());
    output.append(createMath(socket));
    code.append(output);
}

void ShaderWriter::evalSocketValue(DoutSocket *socket)
{
    switch(socket->getNode()->getNodeType())
    {
    case CONTAINER:
        writeContainer(socket);
        break;
    case FUNCTION:
        writeFunction(socket);
        break;
    case CONDITIONCONTAINER:
        writeConditionContainer(socket);
        break;
    case FOR:
        writeForLoop(socket);
        break;
    case WHILE:
        writeWhileLoop(socket);
        break;
    case GATHER:
        writeRSLLoop(socket);
        break;
    case ILLUMINANCE:
        writeRSLLoop(socket);
        break;
    case ILLUMINATE:
        writeRSLLoop(socket);
        break;
    case SOLAR:
        writeRSLLoop(socket);
        break;
    case COLORNODE:
        writeColor(socket);
        break;
    case STRINGNODE:
        writeString(socket);
        break;
    case FLOATNODE:
        writeFloat(socket);
        break;
    case VECTORNODE:
        writeVector(socket);
        break;
    case INSOCKETS:
        evalSocketValue(stepUp(socket)->getCntdSocket());
        break;
    default:
        break;
    }
}

void ShaderWriter::writeFunction(DoutSocket *socket)
{
    QString output;
    initVar(socket);
    output.append(newline());
    output.append(socket->getVarName());
    output.append(" = ");
    FunctionNode *fnode = (FunctionNode*)socket->getNode();
    output.append(fnode->getFunctionName());
    output.append("(");
    foreach(DinSocket *nsocket, fnode->getInSockets())
    {
        if(nsocket->getToken())
        {
            output.append("\"");
            output.append(nsocket->getName());
            output.append("\", ");
        }
        output.append(writeVarName(nsocket));
        gotoNextNode(nsocket);
        if(!fnode->getInSockets().endsWith(nsocket))
            output.append(", ");
    }

    output.append(");");
    code.append(output);
}

void ShaderWriter::writeContainer(DoutSocket *socket)
{
    ContainerNode *cnode = (ContainerNode*)socket->getNode();
    DinSocket *mapped_socket = (DinSocket*)cnode->getSocketInContainer(socket);
    QString output;
    if(cnode->getInSockets().isEmpty())
    {
        initVar(socket);
        if (MathNode::isMathNode(mapped_socket->getCntdSocket()->getNode()))
        {
            output.append(newline());
            output.append(socket->getVarName());
            output.append(" = ");
            output.append(writeVarName(mapped_socket));
            output.append(";");
        }
    }
    gotoNextNode(mapped_socket);
    if(output != "")
        code.append(output);
}

QString ShaderWriter::writeMath(DoutSocket *socket, QString mathOperator)
{
    QString output;
    output.append("(");
    int i = 1;
    foreach(DinSocket *nsocket, socket->getNode()->getInSockets())
    {
        i++;
		if(nsocket->getCntdSocket())
		{

			if(DNode::isMathNode(nsocket->getCntdSocket()->getNode()))
				output.append(createMath(nsocket->getCntdSocket()));
			else
			{
				output.append(writeVarName(nsocket));
				gotoNextNode(nsocket);
			}

		}
        if(i < socket->getNode()->getInSockets().size())
        {
            output.append(" ");
            output.append(mathOperator);
            output.append(" ");
        }
    }
    output.append(")");
    return output;
}

QString ShaderWriter::writeCondition(DoutSocket *socket, QString conditionOperator)
{
    QString output;
    output.append("(");
    int i = 0;
    foreach(DinSocket *nsocket, socket->getNode()->getInSockets())
    {
        i++;
        if(nsocket->getCntdSocket())
        {
            output.append(writeVarName(nsocket));
            gotoNextNode(nsocket);
        }
        if(i < socket->getNode()->getInSockets().size())
        {
            output.append(" ");
            output.append(conditionOperator);
            output.append(" ");
        }
    }
    output.append(")");
    return output;
}

void ShaderWriter::writeConditionContainer(DoutSocket *socket)
{
    DinSocket *mapped_socket = 0, *condition = 0;
    ConditionContainerNode *node = (ConditionContainerNode*)socket->getNode();
    QString output;
    output.append(newline());
    output.append("if(");
    condition = node->getInSockets().first();
    output.append(createCondition(condition->getCntdSocket()));
    output.append(")");
    output.append(newline());
    output.append("{");
    code.append(output);
    incTabLevel();
    output.append(newline());
    mapped_socket = (DinSocket*)node->getSocketInContainer(socket);
    gotoNextNode(mapped_socket);
    decTabLevel();
    code.append(newline());
    code.append("}");
}

QString ShaderWriter::writeNot(DoutSocket *socket)
{
    QString output;
    DinSocket *condition;
    DNode *node = socket->getNode();
    condition = node->getInSockets().first();
    output.append("!");
    output.append(createCondition(condition->getCntdSocket()));
    return output;
}

void ShaderWriter::writeForLoop(DoutSocket *socket)
{
    QString output;
    DNode *node = socket->getNode();
    DinSocket *start, *end, *step, *mapped_socket;
    start = node->getInSockets().at(0);
    end = node->getInSockets().at(1);
    step = node->getInSockets().at(2);

    output.append(newline());
    output.append("for(");
    output.append(start->getVarName());
    output.append(" = ");
    output.append(writeVarName(start));
    gotoNextNode(start);
    output.append(";");
    output.append(start->getVarName());
    output.append(" != ");
    output.append(writeVarName(end));
    gotoNextNode(end);
    output.append(";");
    output.append(start->getVarName());
    output.append("++");
    output.append(")");
    output.append(newline());
    output.append("{");
    incTabLevel();
    output.append(newline());
    ContainerNode *cnode = (ContainerNode*)node;
    mapped_socket = (DinSocket*)cnode->getSocketInContainer(socket);
    gotoNextNode(mapped_socket);
    output.append("}");
    decTabLevel();
    code.append(output);
}

void ShaderWriter::writeWhileLoop(DoutSocket *socket)
{
    QString output;
    DNode *node = socket->getNode();
    code.append(output);
}

void ShaderWriter::writeRSLLoop(DoutSocket *socket)
{
    QString output;
    DNode *node = socket->getNode();
}

QString ShaderWriter::writeColor(DoutSocket *socket)
{
    QString output;
    QString value;
    ColorValueNode *colornode = (ColorValueNode*)socket->getNode();
    value.append("(");
    QColor color = colornode->getValue();
    value.append(QString::number(color.redF()));
    value.append(" ");
    value.append(QString::number(color.greenF()));
    value.append(" ");
    value.append(QString::number(color.blueF()));
    value.append(")");
    if(colornode->isShaderInput())
    {
        colornode->setSocketVarName(socket);
        output.append("color ");
        output.append(socket->getVarName());
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        output = socket->getVarName();
        return output;
    }
    else
        return value;
}

QString ShaderWriter::writeString(DoutSocket *socket)
{
    QString output, value;
    StringValueNode *stringnode = (StringValueNode*)socket->getNode();
    value.append("\"");
    value.append(stringnode->getValue());
    value.append("\"");
    if(stringnode->isShaderInput())
    {
        stringnode->setSocketVarName(socket);
        output.append("string ");
        output.append(socket->getVarName());
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return socket->getVarName();
    }
    else
        return value;
}

QString ShaderWriter::writeFloat(DoutSocket *socket)
{
    QString output, value;
    FloatValueNode *floatnode = (FloatValueNode*)socket->getNode();
    value.append(QString::number(floatnode->getValue()));
    if(floatnode->isShaderInput())
    {
        floatnode->setSocketVarName(socket);
        output.append("float ");
        output.append(socket->getVarName());
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return socket->getVarName();
    }
    else return value;
}

QString ShaderWriter::writeVector(DoutSocket *socket)
{
    return QString();
}

QString ShaderWriter::getCode()
{
    QString returncode;
    returncode.append(ShaderHeader);
    returncode.append(createShaderParameterCode());
    returncode.append(createOutputVars());
    returncode.append(")\n{\n");
    returncode.append("    /*Variable declarations*/");
    returncode.append(createVarDeclares());
    returncode.append("\n\n    /*Begin of the RSL Code*/");
    returncode.append(code);
    returncode.append("\n}");
    return returncode;
}

void ShaderWriter::addToCode(QString c)
{
    code.append(c);
}
void ShaderWriter::addToShaderHeader(QString s)
{
    ShaderHeader.append(s);
}

void ShaderWriter::addToShaderParameter(QString s)
{
    if(!ShaderParameter.contains(s))
        ShaderParameter.append(s);
}

QString ShaderWriter::createShaderParameterCode()
{
    QString parameters;
    foreach(QString parameter, ShaderParameter)
    {
        if(parameter != ShaderParameter.first()
                &&(!(parameter == ShaderParameter.last())
                   && OutputVars.isEmpty()))
        {
            QString space(ShaderHeader);
            space.fill(' ');
            parameters.append("\n");
            parameters.append(space);
        }
        parameters.append(parameter);
    }
    return parameters;
}

void ShaderWriter::addToOutputVars(QString ov)
{
    if(!OutputVars.contains(ov))
        OutputVars.append(ov);
}

QString ShaderWriter::createOutputVars()
{
    QString vars;
    foreach(QString var, OutputVars)
    {
        if(var != OutputVars.last())
        {
            QString space(ShaderHeader);
            space.fill(' ');
            vars.append("\n");
            vars.append(space);
        }
        vars.append(var);
    }
    return vars;
}

void ShaderWriter::addToVarDeclares(QString vd)
{

    if(!VarDeclares.contains(vd))
        VarDeclares.append(vd);
}

QString ShaderWriter::createVarDeclares()
{
    QString vars;
    foreach(QString var, VarDeclares)
    {
        vars.append("\n    ");
        vars.append(var);
    }
    return vars;
}

