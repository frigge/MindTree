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

#ifndef FRGCALLBACKS
#define FRGCALLBACKS

class VNode;
class DSocket;

class Callback
{
public:
    virtual void exec()=0;
};

typedef struct CBstruct 
{
    Callback *cb;
    CBstruct *next;
} CBstruct;

class CallbackList
{
public:
    CallbackList();
    void add(Callback *cb);
    void remove(Callback *cb);
    void operator()();
    void clear();
    void setBlock(bool b);

private:
    bool block;
    CBstruct *cblist;
};

/*  Callbacks */

class VNodeUpdateCallback : public Callback
{
public:
    VNodeUpdateCallback(VNode*);
    virtual void exec();

private:
    VNode *nodeVis;
};

class ScpNameCB : public Callback
{
public:
    ScpNameCB(DSocket *src, DSocket *dst);
    virtual void exec();

private:
    DSocket *src, *dst;
};

class ScpTypeCB : public Callback
{
public:
    ScpTypeCB(DSocket *src, DSocket *dst);
    virtual void exec();

private:
    DSocket *src, *dst;
};

class SsetToVarCB : public Callback
{
public:
    SsetToVarCB(DSocket *socket);
    virtual void exec();

private:
    DSocket *socket;
};

#endif 
