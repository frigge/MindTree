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

    void evalSocketValue(NSocket *socket);
    void initVar(NSocket *socket);
    void outputVar(NSocket *socket);

    void gotoNextNode(NSocket *socket);
    QString writeVarName(NSocket *socket);

    QString var;
    QList<QString> socketnames;

    QList<ContainerNode*> cnode_depth;
    QList<QString>written_sockets;
};

#endif // SHADERWRITER_H
