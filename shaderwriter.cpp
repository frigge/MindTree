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
    QString outputvar;
    switch(start->NodeType)
    {
    case SURFACEOUTPUT:
        ShaderHeader.append("surface ");
        break;
    case DISPLACEMENTOUTPUT:
        ShaderHeader.append("displacement ");
        break;
    case VOLUMEOUTPUT:
        ShaderHeader.append("volume ");
        break;
    case LIGHTOUTPUT:
        ShaderHeader.append("light ");
        break;
    }

    ShaderHeader.append(start->ShaderName);
    ShaderHeader.append("(");

    foreach(NSocket *socket, *start->N_inSockets)
    {
        if(socket->Socket.cntdSockets.size()==0)
            continue;

        if(socket->isVariable)
            outputVar(socket);

        outputvar = socket->Socket.varname;
        outputvar.append(" = ");
        outputvar.append(writeVarName(socket));
        NodeLink *nlink = (NodeLink*)socket->Socket.links.first();
        evalSocketValue(nlink->outSocket);
        code.append(outputvar);
        code.append(";\n");
    }
    ShaderHeader.append(")");
}

void ShaderWriter::initVar(NSocket *socket)
{
    switch(socket->Socket.type)
    {
    case COLOR:
        VarDeclares.append("color ");
        break;
    case FLOAT:
         VarDeclares.append("float ");
        break;
    case STRING:
         VarDeclares.append("string ");
        break;
    case VECTOR:
         VarDeclares.append("vector ");
        break;
    case POINT:
         VarDeclares.append("point ");
        break;
    case NORMAL:
         VarDeclares.append("normal ");
        break;
    }
    VarDeclares.append(socket->Socket.varname);
    VarDeclares.append(";\n");
}

void ShaderWriter::outputVar(NSocket *socket)
{
    QString ending;
    OutputVars.append("output ");
    switch(socket->Socket.type)
    {
    case COLOR:
        OutputVars.append("color ");
        ending = "(0, 0, 0)";
        break;
    case FLOAT:
         OutputVars.append("float ");
         ending = "0";
        break;
    case STRING:
         OutputVars.append("string ");
         ending = "";
        break;
    case VECTOR:
         OutputVars.append("vector ");
         ending = "(0, 0, 0)";
        break;
    case POINT:
         OutputVars.append("point ");
         ending = "(0, 0, 0)";
        break;
    case NORMAL:
         OutputVars.append("normal ");
         ending = "(0, 0, 0)";
        break;
    }

    OutputVars.append(socket->Socket.varname);
    OutputVars.append(" = ");
    OutputVars.append(ending);
    OutputVars.append(";\n");
}

void ShaderWriter::gotoNextNode(NSocket *socket)
{
    NodeLink *nlink;
    if(socket->Socket.links.isEmpty())
        return;

    nlink = (NodeLink*)socket->Socket.links.first();
    NSocket *nextsocket = nlink->outSocket;
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

    NodeLink *nlink = (NodeLink*)insocket->Socket.links.first();
    NSocket *prevsocket = nlink->outSocket;
    NSocket *mapsocket;
    Node *node = (Node*)prevsocket->Socket.node;
    ContainerNode *cnode = (ContainerNode*)node;
    if(node->isContainer())

    {
        mapsocket = cnode->socket_map.key(prevsocket);
        return writeVarName(mapsocket);
    }
    else if(node->NodeType == INSOCKETS)
    {
        cnode = (ContainerNode*)cnode_depth.takeLast();
        mapsocket = cnode->socket_map.value(prevsocket);
        return writeVarName(mapsocket);
    }
    else
        return prevsocket->Socket.varname;
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
    case GREATERTHAN:
        writeCondition(socket, ">");
        break;
    case SMALLERTHAN:
        writeCondition(socket, "<");
        break;
    case EQUAL:
        writeCondition(socket, "==");
        break;
    case AND:
        writeCondition(socket, "&&");
        break;
    case OR:
        writeCondition(socket, "||");
        break;
    case CONDITIONCONTAINER:
        writeConditionContainer(socket);
        break;
    case NOT:
        writeNot(socket);
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
        initVar(socket);
        break;
    }
}

void ShaderWriter::writeFunction(NSocket *socket)
{
    QString output;
    initVar(socket);
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
    output.append(");\n");
    code.append(output);
}

void ShaderWriter::writeContainer(NSocket *socket)
{
    ContainerNode *cnode = (ContainerNode*)socket->Socket.node;
    NSocket *mapped_socket = cnode->socket_map.key(socket);
    cnode_depth.append(cnode);
    gotoNextNode(mapped_socket);
    cnode_depth.removeAll(cnode);
}

void ShaderWriter::writeMath(NSocket *socket, QString mathOperator)
{
    QString output;
    int i = 0;
    Node *node = (Node*)socket->Socket.node;
    initVar(socket);
    output.append(socket->Socket.varname);
    output.append(" = ");
    foreach(NSocket *nsocket, *node->N_inSockets)
    {
        i++;
        if(nsocket->Socket.cntdSockets.size()>0)
            output.append(writeVarName(nsocket));
        gotoNextNode(nsocket);
        if(i < node->N_inSockets->size())
        {
            output.append(" ");
            output.append(mathOperator);
            output.append(" ");
        }
    }
    output.append(";\n");
    code.append(output);
}

void ShaderWriter::writeCondition(NSocket *socket, QString conditionOperator)
{
    Node *node = (Node*)socket->Socket.node;
    QString output;
    output.append("(");
    int i = 0;
    foreach(NSocket *nsocket, *node->N_inSockets)
    {
        i++;
        evalSocketValue(nsocket);
        if(i < node->N_inSockets->size())
        {
            output.append(" ");
            output.append(conditionOperator);
            output.append(" ");
        }
    }
    output.append(")");
    code.append(output);
}

void ShaderWriter::writeConditionContainer(NSocket *socket)
{
    QString output;
    NSocket *mapped_socket, *condition;
    ConditionContainerNode *node = (ConditionContainerNode*)socket->Socket.node;
    output.append("if(");
    condition = node->N_inSockets->first();
    evalSocketValue(condition);
    output.append(")\n{");
    mapped_socket = node->socket_map.key(socket);
    gotoNextNode(mapped_socket);
    output.append("}\n");
    code.append(output);
}

void ShaderWriter::writeNot(NSocket *socket)
{
    QString output;
    NSocket *condition;
    Node *node = (Node*)socket->Socket.node;
    condition = node->N_inSockets->first();
    output.append("!");
    gotoNextNode(condition);
    code.append(output);
}

void ShaderWriter::writeForLoop(NSocket *socket)
{
    QString output;
    Node *node = (Node*)socket->Socket.node;
    NSocket *start, *end, *step, *mapped_socket;
    start = node->N_inSockets->at(0);
    end = node->N_inSockets->at(1);
    step = node->N_inSockets->at(2);

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
    output.append(")\n");
    output.append("{\n");
    ContainerNode *cnode = (ContainerNode*)node;
    mapped_socket = cnode->socket_map.key(socket);
    cnode_depth.append(cnode);
    gotoNextNode(mapped_socket);
    cnode_depth.removeAll(cnode);
    output.append("}\n");
    code.append(output);
}

void ShaderWriter::writeWhileLoop(NSocket *socket)
{
    QString output;
    Node *node = (Node*)socket->Socket.node;
    code.append(output);
}

void ShaderWriter::writeColor(NSocket *socket)
{
    QString output;
    ColorValueNode *colornode = (ColorValueNode*)socket->Socket.node;
    output.append("color ");
    output.append(socket->Socket.varname);
    output.append(" = ");
    output.append("(");
    output.append(QString::number(colornode->colorvalue.redF()));
    output.append(" ");
    output.append(QString::number(colornode->colorvalue.greenF()));
    output.append(" ");
    output.append(QString::number(colornode->colorvalue.blueF()));
    output.append(");\n");
    if(colornode->isShaderInput)
    {
        ShaderHeader.append(output);
        output = "";
    }
    code.append(output);
}

void ShaderWriter::writeString(NSocket *socket)
{
    QString output;
    output.append("string ");
    StringValueNode *stringnode = (StringValueNode*)socket->Socket.node;
    output.append(socket->Socket.varname);
    output.append(" = ");
    output.append("\"");
    output.append(stringnode->stringvalue);
    output.append("\";\n");
    if(stringnode->isShaderInput)
    {
        ShaderHeader.append(output);
        output = "";
    }
    code.append(output);
}

void ShaderWriter::writeFloat(NSocket *socket)
{
    QString output;
    FloatValueNode *floatnode = (FloatValueNode*)socket->Socket.node;
    output.append("float ");
    output.append(socket->Socket.varname);
    output.append(" = ");
    output.append(QString::number(floatnode->floatvalue));
    output.append(";\n");
    if(floatnode->isShaderInput)
    {
        ShaderHeader.append(output);
        output = "";
    }
    code.append(output);
}

QString ShaderWriter::getCode()
{
    QString returncode;
    returncode.append(ShaderHeader);
    returncode.append("\n{\n");
    returncode.append("/*Variable declarations*/\n");
    returncode.append(VarDeclares);
    returncode.append("\n/*Begin of the RSL Code*/\n");
    returncode.append(code);
    returncode.append("}");
    return returncode;
}

void ShaderWriter::addToCode(QString c)
{
    QString tab("    ");
    for(int i = 1; i<tabLevel; i++)
        code.append(tab);
    code.append(c);
}
void ShaderWriter::addToShaderHeader(QString s)
{
    ShaderHeader.append(s);
}

void ShaderWriter::addToOutputVars(QString ov)
{
    OutputVars.append(ov);
}

void ShaderWriter::addToVarDeclares(QString vd)
{
    VarDeclares.append(vd);
}

