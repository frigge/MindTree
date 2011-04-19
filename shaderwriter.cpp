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
        NodeLink *nlink = (NodeLink*)socket->Socket.links.first();
        outputvar.append(nlink->outSocket->Socket.varname);
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
    if(node->NodeType == CONTAINER
       ||node->NodeType == FOR
       ||node->NodeType == WHILE
       ||node->NodeType == RSLLOOP
       ||node->NodeType == CONDITIONCONTAINER)

    {
        mapsocket = cnode->socket_map.key(prevsocket);
        return mapsocket->Socket.cntdSockets.first()->varname;
    }
    else if(node->NodeType == INSOCKETS)
    {
        cnode = (ContainerNode*)cnode_depth.takeLast();
        mapsocket = cnode->socket_map.value(prevsocket);
        return mapsocket->Socket.cntdSockets.first()->varname;
    }
    else
        return insocket->Socket.cntdSockets.first()->varname;
}

void ShaderWriter::evalSocketValue(NSocket *socket)
{
    NodeLink *nlink;
    QString output;
    Node *node = (Node*)socket->Socket.node;
    NSocket *nsocket = 0, *mapped_socket = 0, *condition = 0;
    ContainerNode *cnode;
    FunctionNode *fnode;
    SocketNode *snode;
    ColorValueNode *colornode;
    StringValueNode *stringnode;
    FloatValueNode *floatnode;

    int i = 1;
    switch(node->NodeType)
    {
    case CONTAINER:
        cnode = (ContainerNode*)node;
        mapped_socket = cnode->socket_map.key(socket);
        cnode_depth.append(cnode);
        gotoNextNode(mapped_socket);
        cnode_depth.removeAll(cnode);
        break;

    case FUNCTION:
        initVar(socket);
        output.append(socket->Socket.varname);
        output.append(" = ");
        fnode = (FunctionNode*)node;
        output.append(fnode->function_name);
        output.append("(");
        foreach(nsocket, *node->N_inSockets)
        {
            if(nsocket->Socket.isToken)
            {
                output.append("\"");
                output.append(nsocket->Socket.varname);
                output.append("\", ");
            }
            output.append(writeVarName(nsocket));
            gotoNextNode(nsocket);
            if(!node->N_inSockets->endsWith(nsocket))
                output.append(", ");
        }
        output.append(");\n");
        break;

    case MULTIPLY:
        initVar(socket);
        output.append(socket->Socket.varname);
        output.append(" = ");
        foreach(nsocket, *node->N_inSockets)
        {
            i++;
            if(nsocket->Socket.cntdSockets.size()>0)
                output.append(nsocket->Socket.cntdSockets.first()->varname);
            gotoNextNode(nsocket);
            if(i < node->N_inSockets->size())
                output.append(" * ");
        }
        output.append(";\n");
        break;

    case DIVIDE:
        initVar(socket);
        output.append(socket->Socket.varname);
        output.append(" = ");
        foreach(nsocket, *node->N_inSockets)
        {
            i++;
            if(nsocket->Socket.cntdSockets.size()>0)
                output.append(nsocket->Socket.cntdSockets.first()->varname);
            gotoNextNode(nsocket);
            if(i < node->N_inSockets->size())
                output.append(" / ");
        }
        output.append(";\n");
        break;

    case ADD:
        initVar(socket);
        output.append(socket->Socket.varname);
        output.append(" = ");
        foreach(nsocket, *node->N_inSockets)
        {
            i++;
            if(nsocket->Socket.cntdSockets.size()>0)
                output.append(nsocket->Socket.cntdSockets.first()->varname);
            gotoNextNode(nsocket);
            if(i < node->N_inSockets->size())
                output.append(" + ");
        }
        output.append(";\n");
        break;

    case SUBTRACT:
        initVar(socket);
        output.append(socket->Socket.varname);
        output.append(" = ");
        foreach(nsocket, *node->N_inSockets)
        {
            i++;
            if(nsocket->Socket.cntdSockets.size()>0)
                output.append(nsocket->Socket.cntdSockets.first()->varname);
            gotoNextNode(nsocket);
            if(i < node->N_inSockets->size())
                output.append(" - ");
        }
        output.append(";\n");
        break;

    case DOTPRODUCT:
        initVar(socket);
        output.append(socket->Socket.varname);
        output.append(" = ");
        foreach(nsocket, *node->N_inSockets)
        {
            i++;
            if(nsocket->Socket.cntdSockets.size()>0)
                output.append(nsocket->Socket.cntdSockets.first()->varname);
            gotoNextNode(nsocket);
            if(i < node->N_inSockets->size())
                output.append(" . ");
        }
        output.append(";\n");
        break;

    case GREATERTHAN:
        output.append("(");
        foreach(nsocket, *node->N_inSockets)
        {
            nlink = (NodeLink*)nsocket->Socket.links.first();
            evalSocketValue(nlink->outSocket);
            if(!node->N_inSockets->endsWith(nsocket))
                output.append(">");
        }
        output.append(")");
        break;

    case SMALLERTHAN:
        output.append("(");
        foreach(nsocket, *node->N_inSockets)
        {
            nlink = (NodeLink*)nsocket->Socket.links.first();
            evalSocketValue(nlink->outSocket);
            if(!node->N_inSockets->endsWith(nsocket))
                output.append("<");
        }
        output.append(")");
        break;

    case EQUAL:
        output.append("(");
        foreach(nsocket, *node->N_inSockets)
        {
            nlink = (NodeLink*)nsocket->Socket.links.first();
            evalSocketValue(nlink->outSocket);
            if(!node->N_inSockets->endsWith(nsocket))
                output.append("==");
        }
        output.append(")");
        break;

    case AND:

        output.append("(");
        foreach(nsocket, *node->N_inSockets)
        {
            nlink = (NodeLink*)nsocket->Socket.links.first();
            evalSocketValue(nlink->outSocket);
            if(!node->N_inSockets->endsWith(nsocket))
                output.append("&&");
        }
        output.append(")");
        break;

    case OR:
        output.append("(");
        foreach(nsocket, *node->N_inSockets)
        {
            nlink = (NodeLink*)nsocket->Socket.links.first();
            evalSocketValue(nlink->outSocket);
            if(!node->N_inSockets->endsWith(nsocket))
                output.append("||");
        }
        output.append(")");
        break;

    case CONDITIONCONTAINER:
        output.append("if(");
        condition = node->N_inSockets->first();
        nlink = (NodeLink*)condition->Socket.links.first();
        evalSocketValue(nlink->outSocket);
        output.append(")\n{");
        cnode = (ContainerNode*)node;
        mapped_socket = cnode->socket_map.key(socket);
        gotoNextNode(mapped_socket);
        output.append("}\n");
        break;

    case NOT:
        condition = node->N_inSockets->first();
        nlink = (NodeLink*)condition->Socket.links.first();
        output.append("!");
        evalSocketValue(nlink->outSocket);
        break;

    case FOR:
        break;
    case WHILE:
        break;
    case RSLLOOP:
        break;
    case SURFACEINPUT:
        break;
    case DISPLACEMENTINPUT:
        break;
    case VOLUMEINPUT:
        break;
    case LIGHTINPUT:
        break;
    case INSOCKETS:
        initVar(socket);
        output.append(socket->Socket.varname);
        output.append(" = ");
        writeVarName(socket);
        output.append(";\n");
        gotoNextNode(mapped_socket);
        break;

    case COLORNODE:
        colornode = (ColorValueNode*)node;
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
        break;

    case STRINGNODE:
        output.append("string ");
        stringnode = (StringValueNode*)node;
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
        break;

    case FLOATNODE:
        floatnode = (FloatValueNode*)node;
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
        break;

    case VECTORNODE:
        initVar(socket);
        break;
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
