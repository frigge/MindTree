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

ShaderCodeGenerator::ShaderCodeGenerator(DNode *start)
    : start(start), tabLevel(1)
{
}

void ShaderCodeGenerator::init()    
{
    setVariables();
    QString outputvar;
    foreach(const DinSocket *socket, start->getInSockets())
    {
        if(!socket->getCntdFunctionalSocket())
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

void ShaderCodeGenerator::setVariables(DNode *node)    
{
    if(!node)
    {
        variables.clear();
        variableCnt.clear();
        node = start;
    }

    foreach(DinSocket *socket, node->getInSockets())
    {
        DoutSocket *cntdWorkSocket = socket->getCntdWorkSocket();
        if(cntdWorkSocket) {
            DNode *cntdWorkNode = cntdWorkSocket->getNode();
            if(cntdWorkSocket->getNode()->isContainer()
                &&!cntdWorkNode->getNodeType() == CONTAINER)
                setVariables(cntdWorkNode);
        }
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

QString ShaderCodeGenerator::getVariable(const DoutSocket* socket)const
{
    return variables.value(socket);
}

DoutSocket* ShaderCodeGenerator::getSimilar(DoutSocket *socket)    
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

void ShaderCodeGenerator::insertVariable(const DoutSocket *socket, QString variable)    
{
    variables.insert(socket, variable);
}

QString ShaderCodeGenerator::newline()
{
    QString nl;
    nl.append("\n");
    for(int i=1; i<=tabLevel; i++)
        nl.append("    ");
    return nl;
}

void ShaderCodeGenerator::incTabLevel()
{
    tabLevel++;
}

void ShaderCodeGenerator::decTabLevel()
{
    tabLevel--;
}

void ShaderCodeGenerator::gotoNextNode(const DinSocket *socket)
{
    if(!socket->getCntdSocket())
        return;

    if(!written_sockets.contains(writeVarName(socket)))
    {
        evalSocketValue(socket->getCntdSocket());
        written_sockets.append(writeVarName(socket));
    }
    return;
}

QString ShaderCodeGenerator::writeVarName(const DinSocket *insocket)
{
    if(!insocket->getCntdFunctionalSocket())
        return "";

    const DoutSocket *prevsocket = insocket->getCntdFunctionalSocket();
    const DNode *node = prevsocket->getNode();

    switch(node->getNodeType())
    {
        case COLORNODE:
            return writeColor(prevsocket);
        case FLOATNODE:
            return writeFloat(prevsocket);
        case STRINGNODE:
            return writeString(prevsocket);
        case VECTORNODE:
            return writeVector(prevsocket);
        case ADD:
        case SUBTRACT:
        case MULTIPLY:
        case DIVIDE:
        case DOTPRODUCT:
            return createMath(prevsocket);
        case GREATERTHAN:
        case SMALLERTHAN:
        case EQUAL:
        case AND:
        case OR:
        case NOT:
            return createCondition(prevsocket);
        default:
            return getVariable(prevsocket);
    }
}

const DinSocket *ShaderCodeGenerator::stepUp(const DoutSocket *socket)
{
    const ContainerNode *node = static_cast<const SocketNode*>(socket->getNode())->getContainer();
    const DinSocket *mapsocket = static_cast<const DinSocket*>(node->getSocketOnContainer(socket));
    return mapsocket;
}

QString ShaderCodeGenerator::createCondition(const DoutSocket *socket)
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

QString ShaderCodeGenerator::createMath(const DoutSocket *socket)
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

void ShaderCodeGenerator::writeMathToVar(const DoutSocket *socket)    
{
    QString output;
    output.append(newline());
    output.append(" = ");
    output.append(getVariable(socket));
    output.append(createMath(socket));
    code.append(output);
}

void ShaderCodeGenerator::evalSocketValue(const DoutSocket *socket)
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
        writeCustom(socket);
        break;
    }
}

void ShaderCodeGenerator::writeGetArray(const DoutSocket *socket)    
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

void ShaderCodeGenerator::writeSetArray(const DoutSocket *socket)    
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

void ShaderCodeGenerator::writeVariable(const DoutSocket *socket)    
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

void ShaderCodeGenerator::writeFunction(const DoutSocket *socket)
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

void ShaderCodeGenerator::writeContainer(const DoutSocket *socket)
{
    const ContainerNode *cnode = socket->getNode()->getDerivedConst<ContainerNode>();
    gotoNextNode(cnode->getSocketInContainer(socket)->toIn());
}

QString ShaderCodeGenerator::writeMath(const DoutSocket *socket, QString mathOperator)
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

QString ShaderCodeGenerator::writeCondition(const DoutSocket *socket, QString conditionOperator)
{
    QString output;
    output.append("(");
    int i = 0;
    foreach(const DinSocket *nsocket, socket->getNode()->getInSockets())
    {
        i++;
		if(nsocket->getCntdFunctionalSocket())
		{
            DNode *nextNode = nsocket->getCntdFunctionalSocket()->getNode();
			if(DNode::isConditionNode(nextNode))
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
            output.append(conditionOperator);
            output.append(" ");
        }
    }
    output.append(")");
    return output;
}

void ShaderCodeGenerator::writeConditionContainer(const DoutSocket *socket)
{
    const DinSocket *mapped_socket = 0, *condition = 0;
    const ConditionContainerNode *node = static_cast<const ConditionContainerNode*>(socket->getNode());
    QString output;
    output.append(newline());
    output.append("if(");
    condition = node->getInSockets().first();
    output.append(writeVarName(condition));
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

QString ShaderCodeGenerator::writeNot(const DoutSocket *socket)
{
    QString output;
    const DinSocket *condition;
    const DNode *node = socket->getNode();
    condition = node->getInSockets().first();
    output.append("!");
    output.append(createCondition(condition->getCntdFunctionalSocket()));
    return output;
}

void ShaderCodeGenerator::writeForLoop(const DoutSocket *socket)
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
    output.append("; step+=");
    output.append(writeVarName(step));
    output.append(")");
    output.append(newline());
    output.append("{");
    incTabLevel();
    code.append(output);
    const ContainerNode *cnode = node->getDerivedConst<ContainerNode>();
    mapped_socket = cnode->getSocketInContainer(socket)->toIn();
    gotoNextNode(mapped_socket);
    decTabLevel();
    code.append(newline());
    code.append("}");
}

void ShaderCodeGenerator::writeWhileLoop(const DoutSocket *socket)
{
    QString output;
    const DNode *node = socket->getNode();
    code.append(output);
}

void ShaderCodeGenerator::writeCustom(const DoutSocket *socket)    
{
}

QString ShaderCodeGenerator::writeString(const DoutSocket *socket)
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
        if(socket->isArray())
            output.append("[" + QString::number(socket->getArrayLength()) + "]");
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return getVariable(socket);
    }
    else
        return value;
}

QString ShaderCodeGenerator::writeFloat(const DoutSocket *socket)
{
    QString output, value;
    FloatValueNode *floatnode = (FloatValueNode*)socket->getNode();
    value.append(QString::number(floatnode->getValue()));
    if(floatnode->isShaderInput())
    {
        output.append("float ");
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

QString ShaderCodeGenerator::getCode()
{
    QString returncode;
    returncode.append(ShaderHeader);
    returncode.append(createShaderParameterCode());
    returncode.append(createOutputVars());
    returncode.append(")\n{\n");
    returncode.append("    /*Variable declarations*/");
    returncode.append(createVarDeclares());
    returncode.append("\n\n    /*Begin Code*/");
    returncode.append(code);
    returncode.append("\n}");
    return returncode;
}

void ShaderCodeGenerator::addToCode(QString c)
{
    code.append(c);
}
void ShaderCodeGenerator::addToShaderHeader(QString s)
{
    ShaderHeader.append(s);
}

void ShaderCodeGenerator::addToShaderParameter(QString s)
{
    if(!ShaderParameter.contains(s))
        ShaderParameter.append(s);
}

QString ShaderCodeGenerator::createShaderParameterCode()
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

void ShaderCodeGenerator::addToOutputVars(QString ov)
{
    if(!OutputVars.contains(ov))
        OutputVars.append(ov);
}

QString ShaderCodeGenerator::createOutputVars()
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

void ShaderCodeGenerator::addToVarDeclares(QString vd)
{

    if(!VarDeclares.contains(vd))
        VarDeclares.append(vd);
}

QString ShaderCodeGenerator::createVarDeclares()
{
    QString vars;
    foreach(QString var, VarDeclares)
    {
        vars.append("\n    ");
        vars.append(var);
    }
    return vars;
}

DNode* ShaderCodeGenerator::getStart()    
{
    return start;
}

QList<QString> ShaderCodeGenerator::getWrittenSockets()    
{
    return written_sockets;
}

QString ShaderCodeGenerator::getVar()    
{
    return var;
}

QList<QString> ShaderCodeGenerator::getSocketNames()    
{
    return socketnames; 
}

QString ShaderCodeGenerator::getCodeCache()    
{
    return code;
}

QStringList ShaderCodeGenerator::getVarDeclares()    
{
    return VarDeclares;
}

QString ShaderCodeGenerator::getShaderHeader()    
{
    return ShaderHeader;
}

QStringList ShaderCodeGenerator::getShaderParameter()    
{
    return ShaderParameter;
}

QStringList ShaderCodeGenerator::getOutputVars()    
{
    return OutputVars;
}
