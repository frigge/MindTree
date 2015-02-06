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

#include "data/python/pyexposable.h"
#include "data/properties.h"
#include "data/signal.h"
#include "data/type.h"
#include "mutex"

namespace MindTree
{
class DSocket;
class DinSocket;
class DoutSocket;

typedef std::vector<DinSocket*> DinSocketList;
typedef std::vector<DoutSocket*> DoutSocketList;
typedef std::vector<DSocket*> DSocketList;

class LoadSocketIDMapper
{
public:
    static unsigned short getID(const DSocket *socket);
    static void setID(const DSocket *socket, unsigned short ID);
    static const DSocket * getSocket(unsigned short ID);
    static void remap();

private:
    static std::unordered_map<unsigned short, const DSocket*>loadIDMapper;
};

class CopySocketMapper
{
public:
    static void setSocketPair(DSocket *original, DSocket *copy);
    static DSocket * getCopy(const DSocket *original);
    static void remap();

private:
    static std::unordered_map<DSocket*, DSocket*> socketMap;
};


typedef DataType SocketType;

class DNode;
class DinSocket;
class DoutSocket;

IO::OutStream& operator<<(IO::OutStream &stream, const DinSocket &socket);
IO::OutStream& operator<<(IO::OutStream &stream, const DSocket &socket);
IO::InStream& operator>>(IO::InStream& stream, DinSocket &socket);
IO::InStream& operator>>(IO::InStream& stream, DSocket &socket);

namespace Signal {
    class LiveTimeTracker;
}

class DSocket : public PyExposable
{
public:
    enum SocketDir {
        IN = 0,
        OUT = 1
    };

	DSocket(std::string, SocketType, DNode *node);
    DSocket(const DSocket& socket, DNode *node=0);
    virtual ~DSocket();

    void listenToNameChange(DSocket *other);
    void listenToTypeChange(DSocket *other);
    void listenToChange(DSocket *other);
    void setTypePropagationFunction(std::function<SocketType(SocketType)> fn);

    virtual void listenToLinkedName() = 0;
    virtual void listenToLinkedType() = 0;
    virtual void listenToLinked() = 0;

    DinSocket* toIn();
    DoutSocket* toOut();
    const DinSocket* toIn()const;
    const DoutSocket* toOut()const;

	void  setNode(DNode*);
    bool operator==(DSocket &socket)const;
    bool operator!=(DSocket &socket)const;
    std::string getName() const;
	const SocketType& getType() const;
    SocketDir getDir() const;
	void setDir(SocketDir value);
	DNode* getNode() const;
	bool getVariable() const;
	void setVariable(bool value);
	unsigned short getID() const;

    void setIDName(std::string name);
    std::string getIDName();

    static unsigned short count;
    static void createLink(DSocket *socket1, DSocket *socket2);
    static void removeLink(DinSocket *in, DoutSocket *out);
    static bool isCompatible(DSocket *s1, DSocket *s2);
    static bool isCompatible(SocketType s1, SocketType s2);
    static DSocket* getSocket(unsigned short ID);

	void setType(SocketType value);
	void setName(std::string value);

protected:
    Signal::LiveTimeTracker* _signalLiveTime;

private:
    Signal::CallbackVector _callbacks;

    friend IO::InStream& operator>>(IO::InStream& stream, DSocket &socket);
    std::function<SocketType(SocketType)> _propagateType;

    std::string idName;
    std::string name;
    SocketType type;
    mutable std::mutex _typeLock;
    SocketDir dir;
    DNode *node;
    bool variable;
    unsigned short ID;

    static std::unordered_map<unsigned short, DSocket*>socketIDHash;
};

class DoutSocket;

class DinSocket : public DSocket
{
public:
    DinSocket(std::string, SocketType, DNode *node);
    DinSocket(const DinSocket& socket, DNode *node=0);
    virtual ~DinSocket();

    void setNode(DNode*);
    void addLink(DoutSocket*);

    void listenToLinkedName();
    void listenToLinkedType();
    void listenToLinked();

    static void createLink(DinSocket *in, DinSocket *out);
	const DoutSocket* getCntdSocketConst() const;
    DoutSocket* getCntdSocket() const;
    DoutSocket* getCntdFunctionalSocket() const;
    DoutSocket* getCntdWorkSocket() const;
	void setCntdSocket(DoutSocket *socket);
    void cntdSocketFromID();
    bool operator==(DinSocket &socket)const;
    bool operator!=(DinSocket &socket)const;
	void setTempCntdID(unsigned short value);
	unsigned short getTempCntdID() const;

    Property getProperty()const;
    void setProperty(Property property);

    void clearLink();

private:
    friend IO::InStream& operator>>(IO::InStream& stream, DinSocket &socket);

    Signal::CallbackHandler _linkedNameChangeCallback;
    Signal::CallbackHandler _linkedTypeChangeCallback;
    Signal::CallbackHandler _linkedChangeCallback;

	unsigned short tempCntdID;
    DoutSocket* cntdSocket;

    mutable Property prop;
    mutable std::mutex _propLock;
};


class DoutSocket: public DSocket
{
public:
	DoutSocket(std::string, SocketType, DNode *node);
    DoutSocket(const DoutSocket& socket, DNode *node=0);
    virtual ~DoutSocket();
    bool operator==(DoutSocket &socket)const;
    bool operator!=(DoutSocket &socket)const;
    std::vector<DinSocket*> getCntdSockets() const;

    void registerSocket(DinSocket *socket);
    void unregisterSocket(DinSocket *socket, bool decr=true);
    void pushSocket(DinSocket *socket);

    void listenToLinkedName();
    void listenToLinkedType();
    void listenToLinked();

private:
    std::vector<DinSocket*> cntdSockets;

    Signal::CallbackHandler _linkedNameChangeCallback;
    Signal::CallbackHandler _linkedTypeChangeCallback;
    Signal::CallbackHandler _linkedChangeCallback;
};

} /* MindTree */
#endif // DATA_NODE_SOCKET_H
