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

#ifndef OBJECT_NODE_VIS

#define OBJECT_NODE_VIS

#include "source/graphics/nodes/graphics_node.h"

class TranslateHandle;
class AbstractTransformableNode;
class ObjectNodeVis : public VContainerNode
{
public:
    ObjectNodeVis(AbstractTransformableNode *data);
    virtual ~ObjectNodeVis();
    TranslateHandle* getTranslateHandle();

private:
    TranslateHandle *translateHandle;
};

class VGroupNode : public VNode
{
public:
    VGroupNode(DNode *data);
    virtual ~VGroupNode();

    void updateNodeVis();
    QPainterPath shape() const;
    QRectF boundinRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
};

#endif /* end of include guard: OBJECT_NODE_VIS */
