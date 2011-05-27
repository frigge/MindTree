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

#include "data_node_socket.h"

#include "source/graphics/nodes/graphics_node_socket.h"
#include "data_node.h"
#include "source/graphics/nodes/graphics_node.h"

QHash<int, DNSocket*> DNSocket::loadIDMapper;
QHash<QString, int> DNSocket::SocketNameCnt;
int DNSocket::count = 0;



DNSocket::DNSocket(QString name, socket_type type)
{
    ID = count;
    count++;
    this->name = name;
    this->type = type;
    isVariable = false;
    socketVis = new VNSocket(this);
};

void DNSocket::createLink(DNSocket *in, DNSocket *out)
{
    in->addLink(out);
    out->addLink(in);
}

void DNSocket::addLink(DNSocket *socket)
{
    if(cntdSockets.contains(socket))
        return;
    cntdSockets.append(socket);
    if (type == VARIABLE)
    {
        type = socket->type;
        if(dir == IN)name = socket->name;
    }
    if (isVariable && cntdSockets.size() == 1)
        node->inc_var_socket();

}

QDataStream & operator<<(QDataStream &stream, DNSocket *socket)
{
    stream<<(qint16)socket->ID<<socket->isVariable<<socket->dir;
    stream<<socket->isToken<<socket->name<<socket->type;
    return stream;
}

QDataStream & operator>>(QDataStream &stream, DNSocket **socket)
{
    int dir, type;
    qint16 ID;
    QString name;
    bool isVar, isToken;
    stream>>ID;
    stream>>isVar;
    stream>>dir;
    stream>>isToken;
    stream>>name;
    stream>>type;
    DNSocket *newsocket = new DNSocket(name, (socket_type)type);
    newsocket->dir = (socket_dir)dir;
    newsocket->isVariable = isVar;
    newsocket->isToken = isToken;
    *socket = newsocket;
    DNSocket::loadIDMapper.insert(ID, *socket);
    return stream;
}

bool DNSocket::operator==(DNSocket &socket)
{
    if(isVariable != socket.isVariable
            ||name != socket.name
            ||isToken != socket.isToken
            ||type != socket.type)
        return false;
    return true;
}

bool DNSocket::operator !=(DNSocket &socket)
{
    return(!(*this == socket));
}

void DNSocket::removeLink(DNSocket *socket)
{
    if(dir == IN)
        cntdSockets.clear();
    else
        cntdSockets.removeAll(socket);

    if(isVariable && cntdSockets.isEmpty())
        node->dec_var_socket(this);
    if((DNode::isMathNode(node))
        && dir == OUT
        && node->varcnt == 0)
    {
        node->varsocket->type = VARIABLE;
        type = VARIABLE;
    }
}

void DNSocket::clearLinks()
{
    cntdSockets.clear();
}

void DNSocket::setNode(DNode *node)
{
    if(!node->isGhost())
        socketVis->setParentItem(node->nodeVis);
    this->node = node;
}

