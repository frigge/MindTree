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

#include "list"

class VNode;
class DSocket;
class Callback
{
public:
    virtual void exec()=0;
};

class CallbackList
{
public:
    void add(Callback *cb);
    void remove(Callback *cb);
    void operator()();

private:
    std::list<Callback*> list;
};

class VNodeUpdateCallback : public Callback
{
public:
    VNodeUpdateCallback(VNode*);
    virtual void exec();

private:
    VNode *nodeVis;
};

class SNchangeNameCB : public Callback
{
public:
    SNchangeNameCB(DSocket *socket);
    virtual void exec();

private:
    DSocket *socket;
};

class SNchangeTypeCB : public Callback
{
public:
    SNchangeTypeCB(DSocket *socket);
    virtual void exec();

private:
    DSocket *socket;
};

#endif 
