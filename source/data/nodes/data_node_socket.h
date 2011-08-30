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

#include "source/data/callbacks.h"

class  DSocket;

class LoadSocketIDMapper
{
public:
    static unsigned short getID(DSocket *socket);
    static void setID(DSocket *socket, unsigned short ID);
    static DSocket * getSocket(unsigned short ID);
    static void remap();

private:
    static QHash<unsigned short, DSocket*>loadIDMapper;
};

class CopySocketMapper
{
public:
    static void setSocketPair(DSocket *original, DSocket *copy);
    static DSocket * getCopy(DSocket *original);
    static void remap();

private:
    static QHash<DSocket*, DSocket*> socketMap;
};

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

class DNode;
class VNode;
class DinSocket;
class DoutSocket;
class DNodeLink;

class DSocket
{
public:
	DSocket(QString, socket_type, DNode *node);
    DSocket(DSocket* socket, DNode *node);
    virtual ~DSocket();

    DinSocket* toIn();
    DoutSocket* toOut();

    void setSocketVis(VNSocket *vis);
	void  setNode(DNode*);
    void killSocketVis();
	void createSocketVis(VNode *parent);
    bool operator==(DSocket &socket);
    bool operator!=(DSocket &socket);
	QString getName() const;
	virtual void setName(QString value);
	socket_type getType() const;
	void setType(socket_type value);
	socket_dir getDir() const;
	void setDir(socket_dir value);
	DNode* getNode() const;
	bool getVariable() const;
	void setVariable(bool value);
	VNSocket* getSocketVis();
	unsigned short getID() const;

    void remRenameCB(Callback *cb);
    void addRenameCB(Callback *cb);
    void addTypeCB(Callback *cb);
    void remTypeCB(Callback *cb);

    static unsigned short count;
    static DNodeLink createLink(DSocket *socket1, DSocket *socket2);
    static void removeLink(DinSocket *in, DoutSocket *out);
    static bool isCompatible(DSocket *s1, DSocket *s2);

private:
    QString name;
    socket_type type;
    socket_dir dir;
    DNode *node;
    bool Variable;
    VNSocket *socketVis;
    unsigned short ID;
    CallbackList renameCallbacks;
    CallbackList changeTypeCallbacks;
};

class QDataStream;

QDataStream& operator<<(QDataStream &stream, DSocket *socket);
QDataStream& operator>>(QDataStream &stream, DSocket **socket);

class DoutSocket;

class DinSocket : public DSocket
{
public:
    DinSocket(QString, socket_type, DNode *node);
    DinSocket(DinSocket* socket, DNode *node);
    ~DinSocket();

    void setNode(DNode*);
    void addLink(DoutSocket*);
    void clearLink(bool unregister=true);

    static void createLink(DinSocket *in, DinSocket *out);
	const DoutSocket* getCntdSocketConst() const;
    DoutSocket* getCntdSocket() const;
    DoutSocket* getCntdFunctionalSocket() const;
	void setCntdSocket(DoutSocket *socket);
    void cntdSocketFromID();
	bool getToken() const;
	void setToken(bool value);
    bool operator==(DinSocket &socket);
    bool operator!=(DinSocket &socket);
	void setTempCntdID(unsigned short value);
	unsigned short getTempCntdID() const;

private:
	unsigned short tempCntdID;
    DoutSocket* cntdSocket;
    bool Token;
};

QDataStream& operator<<(QDataStream &stream, DinSocket *socket);
QDataStream& operator>>(QDataStream &stream, DinSocket **socket);

class DoutSocket: public DSocket
{
public:
	DoutSocket(QString, socket_type, DNode *node);
    DoutSocket(DoutSocket* socket, DNode *node);
    ~DoutSocket();
    bool operator==(DoutSocket &socket);
    bool operator!=(DoutSocket &socket);
    QList<DNodeLink> getLinks() const;
    void registerSocket(DSocket *socket);
    void unregisterSocket(DinSocket *socket, bool decr=true);
	virtual void setName(QString value);
    QString setSocketVarName(QHash<QString, unsigned short> *SocketNameCnt);
	QString getVarName() const;

private:
    QList<DinSocket*> cntdSockets;
	QString varname;
};

typedef QList<DinSocket*> DinSocketList;
typedef QList<DoutSocket*> DoutSocketList;

#endif // DATA_NODE_SOCKET_H
