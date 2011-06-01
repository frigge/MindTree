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

#ifndef DNSPACE_H
#define DNSPACE_H

#include "QList"
#include "QString"

class VNSpace;
class DNode;

class DNSpace
{
public:
    DNSpace();
    void addNode(DNode *node);
    void removeNode(DNode *node);
    QString getName();
	void setName(QString value);
    bool operator==(DNSpace &space);
    bool operator!=(DNSpace &space);

    VNSpace *getSpaceVis();
    void setSpaceVis(VNSpace*);

    QList<DNode*> selectedItems();

    qint16 getNodeCnt();
    QList<DNode*>getNodes();

private:
    VNSpace *spaceVis;
    QString name;
    QList<DNode*> nodes;
};

QDataStream & operator<<(QDataStream &stream, DNSpace *space);
QDataStream & operator>>(QDataStream &stream, DNSpace **space);

#endif // DNSPACE_H
