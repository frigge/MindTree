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
    QString VarDeclares;
    QString ShaderHeader;
    QString OutputVars;

    void addToCode(QString);
    void addToVarDeclares(QString);
    void addToShaderHeader(QString);
    void addToOutputVars(QString);

    int tabLevel;

    void evalSocketValue(NSocket *socket);
    void initVar(NSocket *socket);
    void outputVar(NSocket *socket);

    void gotoNextNode(NSocket *socket);
    QString writeVarName(NSocket *socket);

    void writeFunction(NSocket *socket);
    void writeContainer(NSocket *socket);
    void writeMath(NSocket *socket, QString mathOperator);
    void writeCondition(NSocket *socket, QString conditionOperator);
    void writeNot(NSocket *socket);
    void writeConditionContainer(NSocket *socket);
    void writeForLoop(NSocket *socket);
    void writeWhileLoop(NSocket *socket);
    void writeRSLLoop(NSocket *socket);
    void writeString(NSocket *socket);
    void writeFloat(NSocket *socket);
    void writeVector(NSocket *socket);
    void writeColor(NSocket *socket);

    QString var;
    QList<QString> socketnames;

    QList<ContainerNode*> cnode_depth;
    QList<QString>written_sockets;
};

#endif // SHADERWRITER_H
