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
#include "nodelink.h"

ShaderWriter::ShaderWriter(OutputNode *start)
{
    setCNodeDepthCnt();
    tabLevel = 1;
    QString outputvar;
    switch(start->NodeType)
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
    }

    addToShaderHeader(start->ShaderName);
    addToShaderHeader("(");

    foreach(NSocket *socket, *start->N_inSockets)
    {
        if(socket->Socket.cntdSockets.size()==0)
            continue;
        if(!start->N_inSockets->startsWith(socket))
            code.append(newline());

        if(socket->isVariable)
            outputVar(socket);

        outputvar = socket->Socket.varname;
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

void ShaderWriter::initVar(NSocket *socket)
{
    QString VDstr;
    switch(socket->Socket.type)
    {
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
    VDstr.append(socket->Socket.varname);
    VDstr.append(";");
    addToVarDeclares(VDstr);
}

void ShaderWriter::incTabLevel()
{
    tabLevel++;
}

void ShaderWriter::decTabLevel()
{
    tabLevel--;
}

void ShaderWriter::outputVar(NSocket *socket)
{
    QString ending, OVstr;
    OVstr.append("output ");
    switch(socket->Socket.type)
    {
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

    OVstr.append(socket->Socket.varname);
    OVstr.append(" = ");
    OVstr.append(ending);
    OVstr.append(";");
    addToOutputVars(OVstr);
}

void ShaderWriter::gotoNextNode(NSocket *socket)
{
    NodeLink *nlink;
    if(socket->Socket.links.isEmpty())
        return;

    NSocket *nextsocket = getPreviousSocket(socket);
    if(!written_sockets.contains(nextsocket->Socket.varname))
    {
        evalSocketValue(nextsocket);
        written_sockets.append(nextsocket->Socket.varname);
    }
}

QString ShaderWriter::writeVarName(NSocket *insocket)
{
    if(insocket->Socket.cntdSockets.isEmpty())
        return "";

    NSocket *prevsocket = getPreviousSocket(insocket);
    NSocket *mapsocket = 0;
    Node *node = (Node*)prevsocket->Socket.node;
    ContainerNode *cnode = (ContainerNode*)node;
    if(node->isContainer())
    {
        mapsocket = cnode->socket_map.key(prevsocket);
        return writeVarName(mapsocket);
    }
    else if(node->NodeType == INSOCKETS)
    {
        return writeVarName(stepUp(prevsocket));
    }
    else if(node->NodeType == COLORNODE)
    {
        /*
          writeVarName() is recursive when it's used on parts of a sub network due to ignoring these in
          code creation hence we need to reset the depth level here.
          */
        setCNodeDepthCnt();
        return writeColor(prevsocket);
    }
    else if(node->NodeType == FLOATNODE)
    {
        setCNodeDepthCnt();
        return writeFloat(prevsocket);
    }
    else if(node->NodeType == STRINGNODE)
    {
        setCNodeDepthCnt();
        return writeString(prevsocket);
    }
    else if(node->NodeType == VECTORNODE)
    {
        setCNodeDepthCnt();
        return writeVector(prevsocket);
    }
    else
    {
        setCNodeDepthCnt();
        return prevsocket->Socket.varname;
    }
}

void ShaderWriter::setCNodeDepthCnt()
{
    cnode_depth_cnt = cnode_depth.size();
}

NSocket *ShaderWriter::stepUp(NSocket *socket)
{
    NSocket *mapsocket = getCNodeDepthbyCnt()->socket_map.value(socket);
    cnode_depth_cnt--;
    return getPreviousSocket(mapsocket);
}

NSocket *ShaderWriter::getPreviousSocket(NSocket *socket)
{
    NodeLink *nlink = (NodeLink*)socket->Socket.links.first();
    if(nlink)
        return nlink->outSocket;
    else
        return 0;
}

void ShaderWriter::incCNodeDepth(ContainerNode *cnode)
{
    cnode_depth.append(cnode);
    cnode_depth_cnt++;
}

ContainerNode *ShaderWriter::takeCNodeDepth()
{
    cnode_depth_cnt--;
    return cnode_depth.takeLast();
}

ContainerNode *ShaderWriter::getCNodeDepthbyCnt()
{
    return cnode_depth.at(cnode_depth_cnt-1);
}

QString ShaderWriter::createCondition(NSocket *socket)
{
    Node *node = (Node*)socket->Socket.node;
    switch(node->NodeType)
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
    }
}

void ShaderWriter::evalSocketValue(NSocket *socket)
{
    Node *node = (Node*)socket->Socket.node;
    switch(node->NodeType)
    {
    case CONTAINER:
        writeContainer(socket);
        break;
    case FUNCTION:
        writeFunction(socket);
        break;
    case MULTIPLY:
        writeMath(socket, "*");
        break;
    case DIVIDE:
        writeMath(socket, "/");
        break;
    case ADD:
        writeMath(socket, "+");
        break;
    case SUBTRACT:
        writeMath(socket, "-");
        break;
    case DOTPRODUCT:
        writeMath(socket, ".");
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
        evalSocketValue(stepUp(socket));
        setCNodeDepthCnt();
        break;
    }
}

void ShaderWriter::writeFunction(NSocket *socket)
{
    QString output;
    initVar(socket);
    output.append(newline());
    output.append(socket->Socket.varname);
    output.append(" = ");
    FunctionNode *fnode = (FunctionNode*)socket->Socket.node;
    output.append(fnode->function_name);
    output.append("(");
    foreach(NSocket *nsocket, *fnode->N_inSockets)
    {
        if(nsocket->Socket.isToken)
        {
            output.append("\"");
            output.append(nsocket->Socket.name);
            output.append("\", ");
        }
        output.append(writeVarName(nsocket));
        gotoNextNode(nsocket);
        if(!fnode->N_inSockets->endsWith(nsocket))
            output.append(", ");
    }

    if(fnode->N_outSockets->size() > 1)
    {
        foreach(NSocket *nsocket, *fnode->N_outSockets)
        {
            if(nsocket->Socket.isToken)
            {
                output.append("\"");
                output.append(nsocket->Socket.name);
                output.append("\", ");
            }
            output.append(writeVarName(nsocket));
            if(!fnode->N_inSockets->endsWith(nsocket))
                output.append(", ");
        }
    }
    output.append(");");
    code.append(output);
}

void ShaderWriter::writeContainer(NSocket *socket)
{
    ContainerNode *cnode = (ContainerNode*)socket->Socket.node;
    NSocket *mapped_socket = cnode->socket_map.key(socket);
    incCNodeDepth(cnode);
    gotoNextNode(mapped_socket);
    takeCNodeDepth();
}

void ShaderWriter::writeMath(NSocket *socket, QString mathOperator)
{
    QString output;
    output.append(newline());
    Node *node = (Node*)socket->Socket.node;
    initVar(socket);
    output.append(socket->Socket.varname);
    output.append(" = ");
    int i = 1;
    foreach(NSocket *nsocket, *node->N_inSockets)
    {
        i++;
        if(nsocket->Socket.cntdSockets.size()>0)
        {
            output.append(writeVarName(nsocket));
            gotoNextNode(nsocket);
        }
        if(i < node->N_inSockets->size())
        {
            output.append(" ");
            output.append(mathOperator);
            output.append(" ");
        }
    }
    output.append(";");
    code.append(output);
}

QString ShaderWriter::writeCondition(NSocket *socket, QString conditionOperator)
{
    Node *node = (Node*)socket->Socket.node;
    QString output;
    output.append("(");
    int i = 0;
    foreach(NSocket *nsocket, *node->N_inSockets)
    {
        i++;
        if(nsocket->Socket.cntdSockets.size()>0)
        {
            output.append(writeVarName(nsocket));
            gotoNextNode(nsocket);
        }
        if(i < node->N_inSockets->size())
        {
            output.append(" ");
            output.append(conditionOperator);
            output.append(" ");
        }
    }
    output.append(")");
    return output;
}

void ShaderWriter::writeConditionContainer(NSocket *socket)
{
    NSocket *mapped_socket = 0, *condition = 0;
    ConditionContainerNode *node = (ConditionContainerNode*)socket->Socket.node;
    QString output;
    output.append(newline());
    output.append("if(");
    condition = node->N_inSockets->first();
    output.append(createCondition(getPreviousSocket(condition)));
    output.append(")");
    output.append(newline());
    output.append("{");
    code.append(output);
    incTabLevel();
    output.append(newline());
    incCNodeDepth(node);
    mapped_socket = node->socket_map.key(socket);
    gotoNextNode(mapped_socket);
    takeCNodeDepth();
    decTabLevel();
    code.append(newline());
    code.append("}");
}

QString ShaderWriter::writeNot(NSocket *socket)
{
    QString output;
    NSocket *condition;
    Node *node = (Node*)socket->Socket.node;
    condition = node->N_inSockets->first();
    output.append("!");
    output.append(createCondition(getPreviousSocket(condition)));
    return output;
}

void ShaderWriter::writeForLoop(NSocket *socket)
{
    QString output;
    Node *node = (Node*)socket->Socket.node;
    NSocket *start, *end, *step, *mapped_socket;
    start = node->N_inSockets->at(0);
    end = node->N_inSockets->at(1);
    step = node->N_inSockets->at(2);

    output.append(newline());
    output.append("for(");
    output.append(start->Socket.varname);
    output.append(" = ");
    output.append(writeVarName(start));
    gotoNextNode(start);
    output.append(";");
    output.append(start->Socket.varname);
    output.append(" != ");
    output.append(writeVarName(end));
    gotoNextNode(end);
    output.append(";");
    output.append(start->Socket.varname);
    output.append("++");
    output.append(")");
    output.append(newline());
    output.append("{");
    incTabLevel();
    output.append(newline());
    ContainerNode *cnode = (ContainerNode*)node;
    mapped_socket = cnode->socket_map.key(socket);
    incCNodeDepth(cnode);
    gotoNextNode(mapped_socket);
    takeCNodeDepth();
    output.append("}");
    decTabLevel();
    code.append(output);
}

void ShaderWriter::writeWhileLoop(NSocket *socket)
{
    QString output;
    Node *node = (Node*)socket->Socket.node;
    code.append(output);
}

void ShaderWriter::writeRSLLoop(NSocket *socket)
{
    QString output;
    Node *node = (Node*)socket->Socket.node;
}

QString ShaderWriter::writeColor(NSocket *socket)
{
    QString output;
    QString value;
    ColorValueNode *colornode = (ColorValueNode*)socket->Socket.node;
    value.append("(");
    value.append(QString::number(colornode->colorvalue.redF()));
    value.append(" ");
    value.append(QString::number(colornode->colorvalue.greenF()));
    value.append(" ");
    value.append(QString::number(colornode->colorvalue.blueF()));
    value.append(")");
    if(colornode->isShaderInput)
    {
        output.append("color ");
        output.append(socket->Socket.varname);
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        output = socket->Socket.varname;
        return output;
    }
    else
        return value;
}

QString ShaderWriter::writeString(NSocket *socket)
{
    QString output, value;
    StringValueNode *stringnode = (StringValueNode*)socket->Socket.node;
    value.append("\"");
    value.append(stringnode->stringvalue);
    value.append("\"");
    if(stringnode->isShaderInput)
    {
        output.append("string ");
        output.append(socket->Socket.varname);
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return socket->Socket.varname;
    }
    else
        return value;
}

QString ShaderWriter::writeFloat(NSocket *socket)
{
    QString output, value;
    FloatValueNode *floatnode = (FloatValueNode*)socket->Socket.node;
    value.append(QString::number(floatnode->floatvalue));
    if(floatnode->isShaderInput)
    {
        output.append("float ");
        output.append(socket->Socket.varname);
        output.append(" = ");
        output.append(value);
        output.append(";");
        addToShaderParameter(output);
        return socket->Socket.varname;
    }
    else return value;
}

QString ShaderWriter::writeVector(NSocket *socket)
{

}

QString ShaderWriter::getCode()
{
    QString returncode;
    returncode.append(ShaderHeader);
    returncode.append(createShaderParameterCode());
    returncode.append(OutputVars);
    returncode.append(")\n{\n");
    returncode.append("    /*Variable declarations*/");
    returncode.append(VarDeclares);
    returncode.append("\n\n    /*Begin of the RSL Code*/");
    returncode.append(code);
    returncode.append("\n}");
    return returncode;
}

void ShaderWriter::addToCode(QString c)
{
    code.append(c);
}
void ShaderWriter::addToShaderHeader(QString s, bool newline)
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
        if(parameter != ShaderParameter.first())
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

void ShaderWriter::addToOutputVars(QString ov, bool newline)
{
    if(newline)
    {
        QString space(ShaderHeader);
        space.fill(' ');
        OutputVars.append("\n");
        OutputVars.append(space);
    }
    OutputVars.append(ov);
}

void ShaderWriter::addToVarDeclares(QString vd, bool newline)
{
    if(newline)VarDeclares.append("\n    ");
    VarDeclares.append(vd);
}

