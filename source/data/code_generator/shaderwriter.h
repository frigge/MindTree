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

#include "source/lib/mindtree_core.h"

class OutputNode;
namespace MindTree
{
class DNode;
class ContainerNode;
} /* Mind */

struct Parameter {
    Parameter(QString name, MindTree::socket_type t) : name(name), type(t) {}
    MindTree::socket_type type;
    QString name;
    bool array;
};

struct SubCache
{
    SubCache *next;
    const MindTree::ContainerNode *node;
    QString code;
};

class CodeCache
{
public:
    CodeCache();
    QString get();
    void add(QString s, const MindTree::ContainerNode *n);
    bool isCached(const MindTree::ContainerNode *n);

private:
    SubCache *cache;
};

class ShaderCodeGenerator
{
public:
    ShaderCodeGenerator(const MindTree::DNode *start);
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

    virtual inline bool isInputVar(const MindTree::DoutSocket*)=0;

    virtual void incTabLevel();
    virtual void decTabLevel();

    virtual void evalSocketValue(const MindTree::DoutSocket *socket);
    virtual void initVar(const MindTree::DSocket *socket)=0;
    virtual void outputVar(const MindTree::DinSocket *socket)=0;
    virtual QString createCondition(const MindTree::DoutSocket *socket);
    virtual QString createMath(const MindTree::DoutSocket *);

    virtual void gotoNextNode(const MindTree::DinSocket *socket);
    virtual QString writeVarName(const MindTree::DinSocket *socket);

    virtual QString newline();

    virtual void writeFunction(const MindTree::DoutSocket *socket);
    virtual void writeContainer(const MindTree::DoutSocket *socket);
    virtual QString writeMath(const MindTree::DoutSocket *socket, QString mathOperator);
    virtual void writeMathToVar(const MindTree::DoutSocket *socket);
    virtual QString writeCondition(const MindTree::DoutSocket *socket, QString conditionOperator);
    virtual QString writeNot(const MindTree::DoutSocket *socket);
    virtual void writeConditionContainer(const MindTree::DoutSocket *socket);
    virtual void writeForLoop(const MindTree::DoutSocket *socket);
    virtual void writeCustom(const MindTree::DoutSocket *socket);
    virtual void writeWhileLoop(const MindTree::DoutSocket *socket);
    virtual QString writeString(const MindTree::DoutSocket *socket);
    virtual QString writeFloat(const MindTree::DoutSocket *socket);
    virtual QString writeVector(const MindTree::DoutSocket *socket)=0;
    virtual QString writeColor(const MindTree::DoutSocket *socket)=0;
    virtual void writeGetArray(const MindTree::DoutSocket *socket);
    virtual void writeSetArray(const MindTree::DoutSocket *socket);
    virtual void writeVariable(const MindTree::DoutSocket *socket);

    virtual const MindTree::DinSocket *stepUp(const MindTree::DoutSocket *socket);

    virtual void setVariables(const MindTree::DNode *node=0);
    virtual QString getVariable(const MindTree::DSocket* socket)const;
    virtual void insertVariable(const MindTree::DSocket *socket, QString variable);
    void insertLoopVar(const MindTree::DNode *n, const MindTree::DSocket *socket);
    virtual MindTree::DoutSocket* getSimilar(MindTree::DoutSocket *socket);
    virtual QString createOutputVars();
    virtual void addToOutputVars(QString);

    const MindTree::DNode* getStart();
    QList<QString> getWrittenSockets();
    QString getVar();
    QList<QString> getSocketNames();
    QString getCodeCache();
    QStringList getVarDeclares();
    QString getShaderHeader();
    QStringList getShaderParameter();
    QStringList getOutputVars();
    CodeCache* getCache();
    void addParameter(QString name, MindTree::socket_type t);
    
private:
    const MindTree::DNode *start;
    int tabLevel;
    const MindTree::ContainerNode *focus;
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
    QHash<const MindTree::DSocket*, QString> variables;
    QHash<QString, unsigned short>variableCnt;
};

#endif // SHADERWRITER_H
