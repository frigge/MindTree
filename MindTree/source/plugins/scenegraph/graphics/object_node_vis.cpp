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

#include "object_node_vis.h"

#include "cmath"
#include "QSvgRenderer"
#include "data/object.h"
#include "prop_edit_widget.h"

ObjectNodeVis::ObjectNodeVis(AbstractTransformableNode *data)
    : VContainerNode(data), translateHandle(new TranslateHandle((VectorSocketProperty*)data->getTransformSocket()->getSocketProperty()))
{
}

ObjectNodeVis::~ObjectNodeVis()
{
    delete translateHandle; 
}

TranslateHandle* ObjectNodeVis::getTranslateHandle()    
{
   return translateHandle;
}

VGroupNode::VGroupNode(DNode *data)
    : VNode(data)
{
    setNodeColor(QColor( 125, 150, 184 ));
    setNodeWidth(35);
    setNodeHeight(35);
}

VGroupNode::~VGroupNode()
{
}

void VGroupNode::updateNodeVis()    
{
    setNodeWidth(50);
    setNodeHeight(50);
    drawName();
    DAInSocket *s = (DAInSocket*)data->getInSockets().first();
    s = (DAInSocket*)data->getInSockets().at(1);
    DoutSocket *os = data->getOutSockets().first();
}

QRectF VGroupNode::boundinRect()    const
{
    return QRectF(-((getNodeWidth()+20)/2), -((getNodeHeight()+20)/2), getNodeWidth() + 20, getNodeHeight() + 20);
}

QPainterPath VGroupNode::shape()    const
{
    QPainterPath path;
    path.addEllipse(QRectF(-(getNodeWidth()/2), -(getNodeHeight()/2), getNodeWidth(), getNodeHeight()));
    return path;
}

void VGroupNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    updateNodeVis();
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    //painter->setClipRect(option->exposedRect);
    QColor textColor;
    QPen node_outline;
    int nodePen = 1;
    QPointF diagVec = QPointF(-getNodeWidth()/2, getNodeHeight()) - QPointF(getNodeWidth(), -getNodeHeight()/2);
    float length = sqrt(pow(diagVec.x(), 2) + pow(diagVec.y(), 2));
    QPointF ndiagVec(-diagVec.y() / length, diagVec.x() / length);

    QLinearGradient outlineCol(ndiagVec , -1 * ndiagVec);
    outlineCol.setColorAt(0, QColor(150, 150, 150));
    outlineCol.setColorAt(1, QColor(0, 0, 0));

    if (isSelected())
    {
        //textColor = QColor(255, 255, 255);
        //node_outline.setColor(QColor(255, 145, 0, 200));
        //node_outline.setWidth(nodePen);
        //painter->drawPixmap(-35,-35, QPixmap(":./groupnode_selected.svg"));
        QSvgRenderer svg(QString(":./groupnode_selected.svg"));
        svg.render(painter, QRectF(-35, -35, 70, 70));
    }
    else
    {
        //textColor = QColor(255, 255, 255, 255);
        //node_outline.setColor(QColor(30, 30, 30, 255));
        //node_outline = QPen(QBrush(outlineCol), 0.5);
        //node_outline.setWidth(nodePen);
        QSvgRenderer svg(QString(":./groupnode_unselected.svg"));
        svg.render(painter, shape().boundingRect());
        //painter->drawPixmap(-25,-25, QPixmap(":./groupnode_unselected.svg"));
    };
   // painter->setPen(node_outline);
   // painter->setBrush(QBrush(getNodeColor(), Qt::SolidPattern));
//    painter->drawRect(-node_width/2, -node_height/2, node_width, node_height);
    //painter->drawRoundedRect(-getNodeWidth()/2, -getNodeHeight()/2, getNodeWidth(), getNodeHeight(), 5, 5);
    drawName();
};

