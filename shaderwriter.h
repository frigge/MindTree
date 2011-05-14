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

#include "node.h"

class ShaderWriter
{
public:
    ShaderWriter(OutputNode *start);
    QString getCode();

private:
    QString code;
    QStringList VarDeclares;
    QString ShaderHeader;
    QStringList ShaderParameter;
    QStringList OutputVars;

    void addToCode(QString);
    void addToVarDeclares(QString);
    QString createVarDeclares();
    void addToShaderHeader(QString);
    void addToShaderParameter(QString);
    QString createShaderParameterCode();
    void addToOutputVars(QString);
    QString createOutputVars();

    inline bool isInputVar(NSocket*);

    int tabLevel;
    void incTabLevel();
    void decTabLevel();

    void evalSocketValue(NSocket *socket);
    void initVar(NSocket *socket);
    void outputVar(NSocket *socket);
    QString createCondition(NSocket *socket);
    NSocket *getPreviousSocket(NSocket *socket);

    void gotoNextNode(NSocket *socket);
    QString writeVarName(NSocket *socket);
    int cnode_depth_cnt;

    QString newline();

    void writeFunction(NSocket *socket);
    void writeContainer(NSocket *socket);
    void writeMath(NSocket *socket, QString mathOperator);
    QString writeCondition(NSocket *socket, QString conditionOperator);
    QString writeNot(NSocket *socket);
    void writeConditionContainer(NSocket *socket);
    void writeForLoop(NSocket *socket);
    void writeWhileLoop(NSocket *socket);
    void writeRSLLoop(NSocket *socket);
    QString writeString(NSocket *socket);
    QString writeFloat(NSocket *socket);
    QString writeVector(NSocket *socket);
    QString writeColor(NSocket *socket);

    QString var;
    QList<QString> socketnames;

    NSocket *stepUp(NSocket *socket);
    void incCNodeDepth(ContainerNode *cnode);
    ContainerNode *takeCNodeDepth();
    ContainerNode *getCNodeDepthbyCnt();
    QList<ContainerNode*> cnode_depth;
    void setCNodeDepthCnt();
    QList<QString>written_sockets;
};

#endif // SHADERWRITER_H
