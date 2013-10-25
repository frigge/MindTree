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
#include "QObject"
#include "QDataStream"

#include "data/callbacks.h"
#include "data/datatypes.h"
#include "data/python/pyexposable.h"
#include "data/data_nodelink.h"
#include "data/properties.h"

namespace MindTree
{
class DSocket;
class DinSocket;
class DoutSocket;
typedef QList<DinSocket*> DinSocketList;
typedef QList<DoutSocket*> DoutSocketList;


class LLsocket : public PyExposable
{
public:
    LLsocket() : prev(0), next(0), socket(0) {}
    LLsocket& operator++();
    DSocket* operator*();
    bool operator!=(const LLsocket& other);
    LLsocket *prev, *next;
    DSocket *socket;
};

class DSocketList : public PyExposable
{
public:
    DSocketList();
    DSocketList(const DSocketList &other);
    void add(DSocket *socket);
    void move(unsigned short oldpos, unsigned short newpos);
    DinSocketList returnAsInSocketList()const;
    DoutSocketList returnAsOutSocketList()const;
    unsigned short len();
    void rm(DSocket *socket);
    LLsocket* getFirst()const;
    LLsocket* getLLsocketAt(unsigned short pos);
    DSocket* at(uint pos);
    const LLsocket& begin();
    const LLsocket& end();

protected:
    void initList(DSocket *socket);
    LLsocket* getLLlastSocket()const;

private:
    unsigned int length;
    LLsocket *first;
};

class LoadSocketIDMapper
{
public:
    static unsigned short getID(const DSocket *socket);
    static void setID(const DSocket *socket, unsigned short ID);
    static const DSocket * getSocket(unsigned short ID);
    static void remap();

private:
    static QHash<unsigned short, const DSocket*>loadIDMapper;
};

class CopySocketMapper
{
public:
    static void setSocketPair(const DSocket *original, const DSocket *copy);
    static const DSocket * getCopy(const DSocket *original);
    static void remap();

private:
    static QHash<const DSocket*, const DSocket*> socketMap;
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
    SCENEOBJECT,
    POLYGON,
    INTEGER,

    VEC2,
    MAT2,
    MAT3,

    SAMPLER1D,
    SAMPLER2D,
    IMAGE = SAMPLER2D,
    SAMPLER3D,
    SAMPLERCUBE,
    SAMPLER1DSHADOW,
    SAMPLER2DSHADOW,

    OBJDATA,
    SCENEDATA,
    GROUPDATA,
    CUSTOMSOCKET
}socket_type;

class SocketType
{
public:
    SocketType(socket_type t);
    SocketType(std::string str="");
    virtual ~SocketType();

    void mapToNew();

    socket_type getType()const;
    void setType(socket_type value);
    std::string getCustomType()const;
    void setCustomType(std::string value);
    static int getID(std::string str);
    static SocketType byID(int id);
    int getID(QString type);
    static int registerType(std::string type);
    static std::vector<std::string> getTypes();
    int id();

    bool operator==(const SocketType &other);
    bool operator!=(const SocketType &other);
    bool operator==(socket_type t);
    bool operator!=(socket_type t);
    bool operator==(std::string str);
    bool operator!=(std::string str);

    SocketType& operator=(socket_type t);
    SocketType& operator=(std::string str);

private:
    std::string type;
    int _id;
    static int id_cnt;
    static std::vector<std::string> id_map;
    socket_type old_type;
};

typedef enum
{
    IN,
    OUT
} socket_dir;

class DNode;
//class VNode;
class DinSocket;
class DoutSocket;

class DSocket : public QObject, public PyExposable
{
    Q_OBJECT
public:
	DSocket(std::string, SocketType, DNode *node);
    DSocket(const DSocket& socket, DNode *node=0);
    virtual ~DSocket();

    DinSocket* toIn();
    DoutSocket* toOut();
    const DinSocket* toIn()const;
    const DoutSocket* toOut()const;

    QString setSocketVarName(QHash<QString, unsigned short> *SocketNameCnt)const;
	QString getVarName() const;

	void  setNode(DNode*);
    bool operator==(DSocket &socket)const;
    bool operator!=(DSocket &socket)const;
    std::string getName() const;
	SocketType getType() const;
	socket_dir getDir() const;
	void setDir(socket_dir value);
	DNode* getNode() const;
	bool getVariable() const;
	void setVariable(bool value);
	unsigned short getID() const;

    void setIDName(std::string name);
    std::string getIDName();

    void setArray(bool);
    bool getArray()const;

    static unsigned short count;
    static DNodeLink createLink(DSocket *socket1, DSocket *socket2);
    static void removeLink(DinSocket *in, DoutSocket *out);
    static bool isCompatible(DSocket *s1, DSocket *s2);
    static bool isCompatible(SocketType s1, SocketType s2);
    static DSocket* getSocket(unsigned short ID);
    //static void typeCB(const DSocket *first, const DSocket *last);
    //static void nameCB(const DSocket *first, const DSocket *last);

public slots:
	virtual void setType(SocketType value);
	void setName(std::string value);

signals:
    void nameChanged(QString);
    void typeChanged(SocketType);
    void disconnected();
    void removed();

private:
    std::string idName;
    std::string name;
    SocketType type;
    socket_dir dir;
    DNode *node;
    bool variable, is_array;
    unsigned short ID;

    static QHash<unsigned short, DSocket*>socketIDHash;
	mutable QString varname;
};

QDataStream& operator<<(QDataStream &stream, DSocket *socket);
QDataStream& operator>>(QDataStream &stream, DSocket **socket);

class DoutSocket;

class DAInSocket : public DSocket
{
public:
    DAInSocket(std::string, SocketType, DNode *node);
    virtual ~DAInSocket();
    void addLink(DoutSocket *socket);
    QList<DoutSocket*> getLinks()const;

private:
    QList<DoutSocket*> cntdSockets;
};

class DinSocket : public DSocket
{
    Q_OBJECT
public:
    DinSocket(std::string, SocketType, DNode *node);
    DinSocket(const DinSocket& socket, DNode *node=0);
    ~DinSocket();

    void setNode(DNode*);
    void addLink(DoutSocket*);

    static void createLink(DinSocket *in, DinSocket *out);
	const DoutSocket* getCntdSocketConst() const;
    DoutSocket* getCntdSocket() const;
    DoutSocket* getCntdFunctionalSocket() const;
    DoutSocket* getCntdWorkSocket() const;
	void setCntdSocket(DoutSocket *socket);
    void cntdSocketFromID();
	bool getToken() const;
	void setToken(bool value);
    bool operator==(DinSocket &socket)const;
    bool operator!=(DinSocket &socket)const;
	void setTempCntdID(unsigned short value);
	unsigned short getTempCntdID() const;

    Property getProperty()const;
    void setProperty(Property property);

public slots:
    void clearLink();

signals:
    void linked(DoutSocket *);

private:
	unsigned short tempCntdID;
    DoutSocket* cntdSocket;
    bool Token;

    mutable Property prop;
};


class DoutSocket: public DSocket
{
    Q_OBJECT
public:
	DoutSocket(std::string, SocketType, DNode *node);
    DoutSocket(const DoutSocket& socket, DNode *node=0);
    ~DoutSocket();
    void* getAttachedData();
    void setAttachedData(void* value);
    bool operator==(DoutSocket &socket)const;
    bool operator!=(DoutSocket &socket)const;
    QList<DNodeLink> getLinks() const;
    void registerSocket(DSocket *socket);
    void unregisterSocket(DinSocket *socket, bool decr=true);

private:
    void* attachedData;
    QList<DinSocket*> cntdSockets;
};

} /* MindTree */
QDataStream & operator<<(QDataStream &stream, MindTree::SocketType type);
QDataStream& operator<<(QDataStream &stream, MindTree::DinSocket *socket);
QDataStream& operator>>(QDataStream &stream, MindTree::DinSocket **socket);

#endif // DATA_NODE_SOCKET_H
