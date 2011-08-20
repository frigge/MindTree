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
    : start(start)
{
    tabLevel = 1;
    QString outputvar;

    switch(start->getNodeType())
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

    addToShaderHeader(start->getShaderName());
    addToShaderHeader("(");

    foreach(const DinSocket *socket, start->getInSockets())
    {
        if(!socket->getCntdSocket())
            continue;
        if(!start->getInSockets().startsWith(const_cast<DinSocket*>(socket)))
            code.append(newline());

        if(socket->getVariable())
            outputVar(socket);

        outputvar = socket->getName();
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

void ShaderWriter::initVar(const DoutSocket *socket)
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
    VDstr.append(start->getVariable(socket));
    VDstr.append(";");
    addToVarDeclares(VDstr);
}

bool ShaderWriter::isInputVar(const DoutSocket *socket)
{
    QStringList vars;
    vars<<"P"<<"N"<<"I"<<"s"<<"t"<<"u"<<"v"<<"Cl"<<"Cs"<<"Os"<<"L"<<"Oi"<<"Ci"<<"Ps"<<"Ns"<<"du"<<"dv";
    foreach(QString var, vars)
    {
        if(var == start->getVariable(socket))
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

void ShaderWriter::outputVar(const DinSocket *socket)
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

    OVstr.append(socket->getName());
    OVstr.append(" = ");
    OVstr.append(ending);
    OVstr.append(";");
    addToOutputVars(OVstr);
}

void ShaderWriter::gotoNextNode(const DinSocket *socket)
{
    if(!socket->getCntdSocket())
        return;

    if(!written_sockets.contains(writeVarName(socket)))
    {
        evalSocketValue(socket->getCntdSocketConst());
        written_sockets.append(writeVarName(socket));
    }
    return;
}

QString ShaderWriter::writeVarName(const DinSocket *insocket)
{
    if(!insocket->getCntdSocket())
        return "";

    const DoutSocket *prevsocket = insocket->getCntdSocket();
    const DinSocket *mapsocket = 0;
    const DNode *node = prevsocket->getNode();

    ContainerNode *cnode = (ContainerNode*)node;
    if(node->isContainer())
    {
        mapsocket = cnode->getSocketInContainer(prevsocket)->toIn();
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
        return start->getVariable(prevsocket);
    }
}

const DinSocket *ShaderWriter::stepUp(const DoutSocket *socket)
{
    const ContainerNode *node = static_cast<const SocketNode*>(socket->getNode())->getContainer();
    const DinSocket *mapsocket = static_cast<const DinSocket*>(node->getSocketOnContainer(socket));
    return mapsocket;
}

QString ShaderWriter::createCondition(const DoutSocket *socket)
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

QString ShaderWriter::createMath(const DoutSocket *socket)
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

void ShaderWriter::writeMathToVar(const DoutSocket *socket)    
{
    QString output;
    output.append(newline());
    output.append(" = ");
    output.append(start->getVariable(socket));
    output.append(createMath(socket));
    code.append(output);
}

void ShaderWriter::evalSocketValue(const DoutSocket *socket)
{
    if(!socket)return;
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

void ShaderWriter::writeFunction(const DoutSocket *socket)
{
    QString output;
    initVar(socket);
    output.append(newline());
    output.append(start->getVariable(socket));
    output.append(" = ");
    FunctionNode *fnode = (FunctionNode*)socket->getNode();
    output.append(fnode->getFunctionName());
    output.append("(");
    foreach(const DinSocket *nsocket, fnode->getInSockets())
    {
        if(nsocket->getToken())
        {
            output.append("\"");
            output.append(nsocket->getName());
            output.append("\", ");
        }
        output.append(writeVarName(nsocket));
        gotoNextNode(nsocket);
        if(!fnode->getInSockets().endsWith(const_cast<DinSocket*>(nsocket)))
            output.append(", ");
    }

    output.append(");");
    code.append(output);
}

void ShaderWriter::writeContainer(const DoutSocket *socket)
{
    ContainerNode *cnode = (ContainerNode*)socket->getNode();
    const DinSocket *mapped_socket = static_cast<const DinSocket*>(cnode->getSocketInContainer(socket));
    QString output;
//    if(cnode->getInSockets().isEmpty())
//    {
//        if (MathNode::isMathNode(mapped_socket->getCntdSocket()->getNode()))
//        {
//            initVar(socket);
//            output.append(newline());
//            output.append(start->getVariable(socket));
//            output.append(" = ");
//            output.append(writeVarName(mapped_socket));
//            output.append(";");
//        }
//    }
    gotoNextNode(mapped_socket);
    if(output != "")
        code.append(output);
}

QString ShaderWriter::writeMath(const DoutSocket *socket, QString mathOperator)
{
    QString output;
    output.append("(");
    int i = 1;
    foreach(const DinSocket *nsocket, socket->getNode()->getInSockets())
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

QString ShaderWriter::writeCondition(const DoutSocket *socket, QString conditionOperator)
{
    QString output;
    output.append("(");
    int i = 0;
    foreach(const DinSocket *nsocket, socket->getNode()->getInSockets())
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

void ShaderWriter::writeConditionContainer(const DoutSocket *socket)
{
    const DinSocket *mapped_socket = 0, *condition = 0;
    const ConditionContainerNode *node = static_cast<const ConditionContainerNode*>(socket->getNode());
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
    mapped_socket = static_cast<const DinSocket*>(node->getSocketInContainer(socket));
    gotoNextNode(mapped_socket);
    decTabLevel();
    code.append(newline());
    code.append("}");
}

QString ShaderWriter::writeNot(const DoutSocket *socket)
{
    QString output;
    const DinSocket *condition;
    const DNode *node = socket->getNode();
    condition = node->getInSockets().first();
    output.append("!");
    output.append(createCondition(condition->getCntdSocket()));
    return output;
}

void ShaderWriter::writeForLoop(const DoutSocket *socket)
{
    QString output;
    const DNode *node = socket->getNode();
    const DinSocket *start, *end, *step, *mapped_socket;
    start = node->getInSockets().at(0);
    end = node->getInSockets().at(1);
    step = node->getInSockets().at(2);

    output.append(newline());
    output.append("for(");
    output.append(start->getName());
    output.append(" = ");
    output.append(writeVarName(start));
    gotoNextNode(start);
    output.append(";");
    output.append(start->getName());
    output.append(" != ");
    output.append(writeVarName(end));
    gotoNextNode(end);
    output.append(";");
    output.append(start->getName());
    output.append("++");
    output.append(")");
    output.append(newline());
    output.append("{");
    incTabLevel();
    output.append(newline());
    const ContainerNode *cnode = node->getDerivedConst<ContainerNode>();
    mapped_socket = cnode->getSocketInContainer(socket)->toIn();
    gotoNextNode(mapped_socket);
    output.append("}");
    decTabLevel();
    code.append(output);
}

void ShaderWriter::writeWhileLoop(const DoutSocket *socket)
{
    QString output;
    const DNode *node = socket->getNode();
    code.append(output);
}

void ShaderWriter::writeRSLLoop(const DoutSocket *socket)
{
    QString output;
    const DNode *node = socket->getNode();
}

QString ShaderWriter::writeColor(const DoutSocket *socket)
{
    QString output;
    QString value;
    ColorValueNode *colornode = (ColorValueNode*)socket->getNode();
    value.append("color(");
    QColor color = colornode->getValue();
    value.append(QString::number(color.redF()));
    value.append(", ");
    value.append(QString::number(color.greenF()));
    value.append(", ");
    value.append(QString::number(color.blueF()));
    value.append(")");
    if(colornode->isShaderInput())
    {
        output.append("color ");
        output.append(start->getVariable(socket));
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        output = start->getVariable(socket);
        return output;
    }
    else
        return value;
}

QString ShaderWriter::writeString(const DoutSocket *socket)
{
    QString output, value;
    StringValueNode *stringnode = (StringValueNode*)socket->getNode();
    value.append("\"");
    value.append(stringnode->getValue());
    value.append("\"");
    if(stringnode->isShaderInput())
    {
        output.append("string ");
        output.append(start->getVariable(socket));
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return start->getVariable(socket);
    }
    else
        return value;
}

QString ShaderWriter::writeFloat(const DoutSocket *socket)
{
    QString output, value;
    FloatValueNode *floatnode = (FloatValueNode*)socket->getNode();
    value.append(QString::number(floatnode->getValue()));
    if(floatnode->isShaderInput())
    {
        output.append("float ");
        output.append(start->getVariable(socket));
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return start->getVariable(socket);
    }
    else return value;
}

QString ShaderWriter::writeVector(const DoutSocket *socket)
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

