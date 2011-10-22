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

#include "source/data/nodes/data_node.h"

ShaderWriter::ShaderWriter(DNode *start)
    : start(start)
{
    setVariables();
    tabLevel = 1;
    QString outputvar;

    switch(start->getNodeType())
    {
    case SURFACEOUTPUT:
    case PREVIEW:
        addToShaderHeader("surface ");
        addToShaderHeader(start->getDerived<OutputNode>()->getShaderName());
        addToShaderHeader("(");
        break;
    case DISPLACEMENTOUTPUT:
        addToShaderHeader("displacement ");
        addToShaderHeader(start->getDerived<OutputNode>()->getShaderName());
        addToShaderHeader("(");
        break;
    case VOLUMEOUTPUT:
        addToShaderHeader("volume ");
        addToShaderHeader(start->getDerived<OutputNode>()->getShaderName());
        addToShaderHeader("(");
        break;
    case LIGHTOUTPUT:
        addToShaderHeader("light ");
        addToShaderHeader(start->getDerived<OutputNode>()->getShaderName());
        addToShaderHeader("(");
        break;
    default:
        break;
    }


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

void ShaderWriter::setVariables(DNode *node)    
{
    if(!node)
    {
        variables.clear();
        variableCnt.clear();
        node = start;
    }

    foreach(DinSocket *socket, node->getInSockets())
    {
        DoutSocket *cntdSocket = socket->getCntdFunctionalSocket();
        if(!cntdSocket)
            continue;

        if(variables.contains(cntdSocket))
            continue;
            
        DNode *cntdNode = cntdSocket->getNode();
        QString varname_raw = cntdSocket->getName().replace(" ", "_");
        int ar = varname_raw.indexOf("[");
        if(ar >= 0)
            varname_raw = varname_raw.left(ar);

        DoutSocket *simSocket = getSimilar(cntdSocket);
        if(variableCnt.contains(varname_raw)
            && simSocket)
            insertVariable(cntdSocket, getVariable(simSocket));
        else
            insertVariable(cntdSocket, cntdSocket->setSocketVarName(&variableCnt));

        setVariables(cntdNode);
    }
}

QString ShaderWriter::getVariable(const DoutSocket* socket)const
{
    return variables.value(socket);
}

DoutSocket* ShaderWriter::getSimilar(DoutSocket *socket)    
{
    DNode *node = socket->getNode();
    DNode *simnode = 0;
    foreach(DNode *n, start->getAllInNodes())
    {
        if(n != node
            && *n == *node)
            {
                simnode = n;
                foreach(DNode *inn, n->getAllInNodes())
                {
                    bool hasSim = false;
                    foreach(DNode *inNode, node->getAllInNodes())
                        if(*inn == *inNode)
                        {
                            hasSim = true;
                            for(int i=0; i<inn->getInSockets().size(); i++)
                                if(*inn->getInSockets().at(i) != *inNode->getInSockets().at(i))
                                    hasSim = false;
                        }
                    if(!hasSim) simnode = 0;
                }
            }
        if(simnode)
            break;
    }
    if(!simnode) return 0;

    foreach(DoutSocket *out, simnode->getOutSockets())
        if(*out == *socket)
            return out;
}

void ShaderWriter::insertVariable(const DoutSocket *socket, QString variable)    
{
    variables.insert(socket, variable);
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
    VDstr.append(getVariable(socket));
    VDstr.append(";");
    addToVarDeclares(VDstr);
}

bool ShaderWriter::isInputVar(const DoutSocket *socket)
{
    QStringList vars;
    vars<<"P"<<"N"<<"I"<<"s"<<"t"<<"u"<<"v"<<"Cl"<<"Cs"<<"Os"<<"L"<<"Oi"<<"Ci"<<"Ps"<<"Ns"<<"du"<<"dv";
    foreach(QString var, vars)
    {
        if(var == getVariable(socket))
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
        return getVariable(prevsocket);
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
    output.append(getVariable(socket));
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
    case GETARRAY:
        writeGetArray(socket);
        break;
    case SETARRAY:
        writeSetArray(socket);
        break;
    case VARNAME:
        writeVariable(socket);
        break;
    default:
        break;
    }
}

void ShaderWriter::writeGetArray(const DoutSocket *socket)    
{
    const DinSocket *array = socket->getNode()->getInSockets().at(0);
    const DinSocket *index = socket->getNode()->getInSockets().at(1);
    QString output;
    output.append(newline());
    initVar(socket);
    output.append(getVariable(socket));
    output.append(" = ");
    output.append(writeVarName(array));
    gotoNextNode(array);
    output.append("[");
    output.append(writeVarName(index));
    gotoNextNode(index);
    output.append("];");
    code.append(output);
}

void ShaderWriter::writeSetArray(const DoutSocket *socket)    
{
    const DinSocket *array = socket->getNode()->getInSockets().at(0);
    const DinSocket *index = socket->getNode()->getInSockets().at(1);
    QString output;
    output.append(newline());
    initVar(socket);
    output.append(writeVarName(array));
    gotoNextNode(array);
    output.append("[");
    output.append(writeVarName(index));
    gotoNextNode(index);
    output.append("]");
    output.append(" = ");
    output.append(getVariable(socket));
    output.append(";");
    code.append(output);
}

void ShaderWriter::writeVariable(const DoutSocket *socket)    
{
    QString output;
    output.append(newline());
    initVar(socket);
    output.append(getVariable(socket));
    output.append(" = ");
    output.append(writeVarName(socket->getNode()->getInSockets().first()));
    output.append(";");
    code.append(output);
}

void ShaderWriter::writeFunction(const DoutSocket *socket)
{
    QString output;
    initVar(socket);
    output.append(newline());
    output.append(getVariable(socket));
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
//            output.append(getVariable(socket));
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
		if(nsocket->getCntdFunctionalSocket())
		{

            DNode *nextNode = nsocket->getCntdFunctionalSocket()->getNode();
			if(DNode::isMathNode(nextNode))
				output.append(createMath(nsocket->getCntdFunctionalSocket()));
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
    output.append("float step=");
    output.append(writeVarName(start));
    gotoNextNode(start);
    output.append("; step < ");
    output.append(writeVarName(end));
    gotoNextNode(end);
    output.append(";");
    output.append("; step++)");
    output.append(newline());
    output.append("{");
    incTabLevel();
    output.append(newline());
    code.append(output);
    const ContainerNode *cnode = node->getDerivedConst<ContainerNode>();
    mapped_socket = cnode->getSocketInContainer(socket)->toIn();
    gotoNextNode(mapped_socket);
    decTabLevel();
    code.append("}");
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
        output.append(getVariable(socket));
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
        output.append(getVariable(socket));
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return getVariable(socket);
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
        output.append(getVariable(socket));
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return getVariable(socket);
    }
    else return value;
}

QString ShaderWriter::writeVector(const DoutSocket *socket)
{
    QString output, value;
    const VectorValueNode *node = socket->getNode()->getDerivedConst<VectorValueNode>();
    value.append("vector(");
    Vector vec = node->getValue();
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
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return getVariable(socket);
    }
    else return value;
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

