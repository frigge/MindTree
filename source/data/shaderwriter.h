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

#include "source/data/nodes/data_node.h"

#include "QString"
#include "QStringList"

class ShaderWriter
{
public:
    ShaderWriter(OutputNode *start);
    QString getCode();

private:
    void addToCode(QString);
    void addToVarDeclares(QString);
    QString createVarDeclares();
    void addToShaderHeader(QString);
    void addToShaderParameter(QString);
    QString createShaderParameterCode();
    void addToOutputVars(QString);
    QString createOutputVars();

    inline bool isInputVar(DoutSocket*);

    void incTabLevel();
    void decTabLevel();

    void evalSocketValue(DoutSocket *socket);
    void initVar(DoutSocket *socket);
    void outputVar(DinSocket *socket);
    QString createCondition(DoutSocket *socket);
    QString createMath(DoutSocket *);

    void gotoNextNode(DinSocket *socket);
    QString writeVarName(DinSocket *socket);

    QString newline();

    void writeFunction(DoutSocket *socket);
    void writeContainer(DoutSocket *socket);
    QString writeMath(DoutSocket *socket, QString mathOperator);
    void writeMathToVar(DoutSocket *socket);
    QString writeCondition(DoutSocket *socket, QString conditionOperator);
    QString writeNot(DoutSocket *socket);
    void writeConditionContainer(DoutSocket *socket);
    void writeForLoop(DoutSocket *socket);
    void writeWhileLoop(DoutSocket *socket);
    void writeRSLLoop(DoutSocket *socket);
    QString writeString(DoutSocket *socket);
    QString writeFloat(DoutSocket *socket);
    QString writeVector(DoutSocket *socket);
    QString writeColor(DoutSocket *socket);

    DinSocket *stepUp(DoutSocket *socket);

private:
    QList<QString>written_sockets;
    QString var;
    QList<QString> socketnames;
    QString code;
    QStringList VarDeclares;
    QString ShaderHeader;
    QStringList ShaderParameter;
    QStringList OutputVars;
    int tabLevel;
};

#endif // SHADERWRITER_H
