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


#ifndef SHADERWRITER_H
#define SHADERWRITER_H

#include "QString"
#include "QHash"
#include "QStringList"
#include "vector"

#include "source/data/nodes/data_node_socket.h"

class OutputNode;
class DNode;
class ContainerNode;

struct Parameter {
    Parameter(QString name, socket_type t) : name(name), type(t) {}
    socket_type type;
    QString name;
    bool array;
};

struct SubCache
{
    SubCache *next;
    const ContainerNode *node;
    QString code;
};

class CodeCache
{
public:
    CodeCache();
    QString get();
    void add(QString s, const ContainerNode *n);
    bool isCached(const ContainerNode *n);

private:
    SubCache *cache;
};

class ShaderCodeGenerator
{
public:
    ShaderCodeGenerator(const DNode *start);
    virtual QString getCode();
    std::vector<Parameter> getParameter();

protected:
    virtual void init();
    virtual void createHeader()=0;

    virtual void addToCode(QString);
    virtual void addToVarDeclares(QString);
    virtual QString createVarDeclares();
    virtual void addToShaderHeader(QString);
    virtual void addToShaderParameter(QString);
    virtual QString createShaderParameterCode();

    virtual inline bool isInputVar(const DoutSocket*)=0;

    virtual void incTabLevel();
    virtual void decTabLevel();

    virtual void evalSocketValue(const DoutSocket *socket);
    virtual void initVar(const DSocket *socket)=0;
    virtual void outputVar(const DinSocket *socket)=0;
    virtual QString createCondition(const DoutSocket *socket);
    virtual QString createMath(const DoutSocket *);

    virtual void gotoNextNode(const DinSocket *socket);
    virtual QString writeVarName(const DinSocket *socket);

    virtual QString newline();

    virtual void writeFunction(const DoutSocket *socket);
    virtual void writeContainer(const DoutSocket *socket);
    virtual QString writeMath(const DoutSocket *socket, QString mathOperator);
    virtual void writeMathToVar(const DoutSocket *socket);
    virtual QString writeCondition(const DoutSocket *socket, QString conditionOperator);
    virtual QString writeNot(const DoutSocket *socket);
    virtual void writeConditionContainer(const DoutSocket *socket);
    virtual void writeForLoop(const DoutSocket *socket);
    virtual void writeCustom(const DoutSocket *socket);
    virtual void writeWhileLoop(const DoutSocket *socket);
    virtual QString writeString(const DoutSocket *socket);
    virtual QString writeFloat(const DoutSocket *socket);
    virtual QString writeVector(const DoutSocket *socket)=0;
    virtual QString writeColor(const DoutSocket *socket)=0;
    virtual void writeGetArray(const DoutSocket *socket);
    virtual void writeSetArray(const DoutSocket *socket);
    virtual void writeVariable(const DoutSocket *socket);

    virtual const DinSocket *stepUp(const DoutSocket *socket);

    virtual void setVariables(const DNode *node=0);
    virtual QString getVariable(const DSocket* socket)const;
    virtual void insertVariable(const DSocket *socket, QString variable);
    void insertLoopVar(const DNode *n, const DSocket *socket);
    virtual DoutSocket* getSimilar(DoutSocket *socket);
    virtual QString createOutputVars();
    virtual void addToOutputVars(QString);

    const DNode* getStart();
    QList<QString> getWrittenSockets();
    QString getVar();
    QList<QString> getSocketNames();
    QString getCodeCache();
    QStringList getVarDeclares();
    QString getShaderHeader();
    QStringList getShaderParameter();
    QStringList getOutputVars();
    CodeCache* getCache();
    void addParameter(QString name, socket_type t);
    
private:
    const DNode *start;
    int tabLevel;
    const ContainerNode *focus;
    QList<QString>written_sockets;
    QString var;
    QList<QString> socketnames;
    CodeCache cache;
    QString code;
    QStringList VarDeclares;
    QString ShaderHeader;
    QStringList ShaderParameter;
    std::vector<Parameter> parameterList;
    QStringList OutputVars;
    QHash<const DSocket*, QString> variables;
    QHash<QString, unsigned short>variableCnt;
};

#endif // SHADERWRITER_H
