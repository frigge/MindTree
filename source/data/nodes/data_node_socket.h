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
class DinSocket;
class DoutSocket;
typedef QList<DinSocket*> DinSocketList;
typedef QList<DoutSocket*> DoutSocketList;


typedef struct LLsocket
{
    LLsocket() : prev(0), next(0), socket(0) {}
    struct LLsocket *prev, *next;
    DSocket *socket;
} LLsocket;

class DSocketList
{
public:
    DSocketList();
    void add(DSocket *socket);
    void move(unsigned short oldpos, unsigned short newpos);
    DinSocketList returnAsInSocketList()const;
    DoutSocketList returnAsOutSocketList()const;
    unsigned short len();
    void rm(DSocket *socket);

protected:
    void initList(DSocket *socket);
    LLsocket* getLLlastSocket()const;
    LLsocket* getLLsocketAt(unsigned short pos);

private:
    LLsocket *first;
};

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
    CONDITION,
    MATRIX,
    POLYGON,
    OBJECT,
    INTEGER
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
class ArrayContainer;

class DSocket
{
public:
	DSocket(QString, socket_type, DNode *node);
    DSocket(DSocket* socket, DNode *node);
    virtual ~DSocket();

    DinSocket* toIn();
    DoutSocket* toOut();

    QString setSocketVarName(QHash<QString, unsigned short> *SocketNameCnt)const;
	QString getVarName() const;

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

    void setIDName(QString name);
    QString getIDName();

    void remRenameCB(Callback *cb);
    void addRenameCB(Callback *cb);
    void addTypeCB(Callback *cb);
    void remTypeCB(Callback *cb);
    void addLinkCB(Callback *cb);
    void remLinkCB(Callback *cb);
    void addRmLinkCB(Callback *cb);
    void remRmLinkCB(Callback *cb);
    virtual void unblockAllCB();
    virtual void blockAllCB();

    bool isArray() const;
    void setArray(unsigned short);
    int getIndex()const;
    void setIndex(int ind);
    void setArrayID(unsigned short value);
    unsigned short getArrayID()const;
    ArrayContainer* getArray()const;  
    int getArrayLength() const;

    static unsigned short count;
    static DNodeLink createLink(DSocket *socket1, DSocket *socket2);
    static void removeLink(DinSocket *in, DoutSocket *out);
    static bool isCompatible(DSocket *s1, DSocket *s2);
    static DSocket* getSocket(unsigned short ID);

protected:
    CallbackList linkCallbacks;
    CallbackList removeLinkCallbacks;

private:
    QString idName;
    QString name;
    socket_type type;
    socket_dir dir;
    DNode *node;
    bool Variable, is_array;
    int index;
    VNSocket *socketVis;
    unsigned short ID;
    unsigned short arrayID;
    int arrayLength;
    ArrayContainer *array;
    CallbackList renameCallbacks;
    CallbackList changeTypeCallbacks;

    static QHash<unsigned short, DSocket*>socketIDHash;
	mutable QString varname;
};

class QDataStream;

QDataStream& operator<<(QDataStream &stream, DSocket *socket);
QDataStream& operator>>(QDataStream &stream, DSocket **socket);

class ArrayContainer
{
public:
    ArrayContainer(DSocket *frist);
    void addSocket(DSocket *socket);

private:
    QList<DSocket*>array;
    unsigned short ID;
};

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
    DoutSocket* getCntdWorkSocket() const;
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
    ScpNameCB *linkedNameCB;
    ScpTypeCB *linkedTypeCB;
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

private:
    QList<DinSocket*> cntdSockets;
};

#endif // DATA_NODE_SOCKET_H
