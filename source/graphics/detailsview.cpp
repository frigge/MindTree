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

#include "detailsview.h"
#include "source/data/base/frg.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/scene/cache_data.h"
#include "source/data/base/project.h"

DetailsView::DetailsView(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(5);
    connect((QObject*)FRG::Space, SIGNAL(selectionChanged()), this, SLOT(updateFocus()));
    connect((QObject*)FRG::Space, SIGNAL(linkChanged()), this, SLOT(updateFocus()));
}

DetailsView::~DetailsView()
{
}

void DetailsView::updateFocus()    
{
    if(FRG::Space->selectedNodes().isEmpty())
        clear();
    else
        setFocusNode(FRG::Space->selectedNodes().first()); 
}

void DetailsView::setFocusNode(DNode* node)
{
    clear();
    foreach(Object *obj, FRG::CurrentProject->getObjects())
        if(!obj->getParent())
            addTopLevelItem(showObjectData(obj));
}

QTreeWidgetItem* DetailsView::showObjectData(Object *obj)
{
    QTreeWidgetItem *topItem = new QTreeWidgetItem(QStringList() << obj->getName());

    //vertex indices
    double *verts = obj->getVertices();
    if(verts){
        QTreeWidgetItem *vertsItem = new QTreeWidgetItem(QStringList() << "Vertices");
        topItem->addChild(vertsItem);
        for(int i=0; i<obj->getVertCnt(); i++) {
            vertsItem->addChild(new QTreeWidgetItem(QStringList() 
                << QString::number(i)
                << QString::number(verts[i*3+0], 'f', 5)
                << QString::number(verts[i*3+1], 'f', 5)
                << QString::number(verts[i*3+2], 'f', 5)));
        }
        delete [] verts;
    }
    
    //polygon indices
    Polygon *polys = obj->getPolygons();
    if(polys){
        QTreeWidgetItem *polyItem = new QTreeWidgetItem(QStringList() << "Polygons");
        topItem->addChild(polyItem);
        for(int j=0; j<obj->getPolyCnt(); j++) {
            QStringList faceList;
            faceList << QString::number(j);
            QString fi;
            int *polyvertindex = polys[j].vertices;
            for(int k=0; k<polys[j].vertexcount; k++)
                if(k>0)
                    fi.append("," + QString::number(polys[j].vertices[k]));
                else
                    fi.append(QString::number(polys[j].vertices[k]));
            faceList << fi;
            polyItem->addChild(new QTreeWidgetItem(faceList));
        }
        delete [] polys;
    }
    if(obj->getOwner()) topItem->addChild(showObjectData(obj->getOwner()));
    foreach(Object *ob, obj->getChildren())
        topItem->addChild(showObjectData(ob));

    return topItem;
}
