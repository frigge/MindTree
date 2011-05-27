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

#ifndef DATA_NODE_SOCKET_H
#define DATA_NODE_SOCKET_H

#include "QHash"

class VNSocket;

typedef enum
{
    NORMAL,
    VECTOR,
    FLOAT,
    COLOR,
    POINT,
    STRING,
    VARIABLE,
    CONDITION
}socket_type;

typedef enum
{
    IN,
    OUT
} socket_dir;

class  DNode;

class DNSocket
{
public:
    DNSocket(QString, socket_type);
    QString name;
    socket_type type;
    socket_dir dir;
    DNode *node;
    QList<DNSocket*> cntdSockets;
    bool isToken;
    QString varname;
    bool isVariable;
    static int count;
    VNSocket *socketVis;
    int ID;

    void setNode(DNode*);
    void addLink(DNSocket*);
    void clearLinks();
    void removeLink(DNSocket*);

    static QHash<int, DNSocket*>loadIDMapper;
    static QHash<QString, int>SocketNameCnt;
    static void createLink(DNSocket *in, DNSocket *out);
    bool operator==(DNSocket &socket);
    bool operator!=(DNSocket &socket);
};

QDataStream & operator<<(QDataStream &stream, DNSocket *socket);
QDataStream & operator>>(QDataStream &stream, DNSocket **socket);

#endif // DATA_NODE_SOCKET_H
