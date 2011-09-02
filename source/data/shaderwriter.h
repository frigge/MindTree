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
#include "QStringList"

class OutputNode;
class DoutSocket;
class DinSocket;

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

    inline bool isInputVar(const DoutSocket*);

    void incTabLevel();
    void decTabLevel();

    void evalSocketValue(const DoutSocket *socket);
    void initVar(const DoutSocket *socket);
    void outputVar(const DinSocket *socket);
    QString createCondition(const DoutSocket *socket);
    QString createMath(const DoutSocket *);

    void gotoNextNode(const DinSocket *socket);
    QString writeVarName(const DinSocket *socket);

    QString newline();

    void writeFunction(const DoutSocket *socket);
    void writeContainer(const DoutSocket *socket);
    QString writeMath(const DoutSocket *socket, QString mathOperator);
    void writeMathToVar(const DoutSocket *socket);
    QString writeCondition(const DoutSocket *socket, QString conditionOperator);
    QString writeNot(const DoutSocket *socket);
    void writeConditionContainer(const DoutSocket *socket);
    void writeForLoop(const DoutSocket *socket);
    void writeWhileLoop(const DoutSocket *socket);
    void writeRSLLoop(const DoutSocket *socket);
    QString writeString(const DoutSocket *socket);
    QString writeFloat(const DoutSocket *socket);
    QString writeVector(const DoutSocket *socket);
    QString writeColor(const DoutSocket *socket);

    const DinSocket *stepUp(const DoutSocket *socket);

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
    OutputNode *start;
};

#endif // SHADERWRITER_H
