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

#include "QFile"
#include "QFileInfo"
#include "QString"
#include "glm/glm.hpp"

#include "obj.h"

using namespace MindTree;

ObjImporter::ObjImporter(std::string filepath)
{
    QFile file(filepath.c_str());
    file.open(QFile::ReadOnly);
    QFileInfo fi(file);
    if(!fi.exists()) {
        std::cout << filepath << " not found" << std::endl;
        return;
    }
    QTextStream stream(&file);

    readData(stream);
}

ObjImporter::~ObjImporter()
{
}

void ObjImporter::readData(QTextStream &stream)    
{
    std::shared_ptr<GeoObject> obj;

    std::cout <<  "importing object ...";
    while(!stream.atEnd()) {
        QString line = stream.readLine();
        if(line.startsWith("o ")) {
            obj = addObject(line);
        }
        else if(line.startsWith("v "))
                addVertex(line, obj);
        else if(line.startsWith("vn "))
                addNormal(line, obj);
        else if(line.startsWith("f "))
                addFace(line, obj);
        else if(line.startsWith("st "))
            addUV(line, obj);
    }

    std::cout <<  " done" << std::endl;
    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    if(!mesh->hasProperty("N")) {
        std::cout <<  "no normals found, computing normals ... ";
        mesh->computeVertexNormals();
        std::cout <<  "done" << std::endl;
    }
}

std::shared_ptr<GeoObject> ObjImporter::addObject(QString line)    
{
    auto obj = std::make_shared<GeoObject>();
    obj->setName("ObjImported");

    if(!grp) grp = std::make_shared<Group>();
    grp->addMember(obj);
    QStringList l = line.split(" ");
    l.takeFirst();
    obj->setName(l[0].toStdString());
    auto mesh = std::make_shared<MeshData>();
    mesh->setProperty("P", std::make_shared<VertexList>());
    mesh->setProperty("polygon", std::make_shared<PolygonList>());
    obj->setData(mesh);
    return obj;
}

void ObjImporter::addVertex(QString line, std::shared_ptr<GeoObject> obj)
{
    QStringList l = line.split(" ");
    double d[3];
    int i=0;
    l.takeFirst();
    for(QString vstr : l){
        d[i] = vstr.toDouble();
        ++i;
    }
    std::static_pointer_cast<MeshData>(obj->getData())
        ->getProperty("P")
        .getData<std::shared_ptr<VertexList>>()
        ->push_back(glm::vec3(d[0], d[1], d[2]));
}

void ObjImporter::addNormal(QString line, std::shared_ptr<GeoObject> obj)
{
    QStringList l = line.split(" ");
    double d[3];
    int i=0;
    l.takeFirst();
    for(QString vstr : l){
        d[i] = vstr.toDouble();
        ++i;
    }
    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    if(!mesh->hasProperty("N"))
        mesh->setProperty("N", std::make_shared<VertexList>());

    mesh->getProperty("N")
        .getData<std::shared_ptr<VertexList>>()
        ->push_back(glm::vec3(d[0], d[1], d[2]));
}

void ObjImporter::addFace(QString line, std::shared_ptr<GeoObject> obj)    
{
    QStringList l = line.split(" ");
    Polygon p;
    l.takeFirst();
    for(QString vstr : l){
        QStringList tmp = vstr.split("/");
        p.push_back(tmp.at(0).toInt() - 1);
    }
    std::static_pointer_cast<MeshData>(obj->getData())
        ->getProperty("polygon")
        .getData<std::shared_ptr<PolygonList>>()
        ->push_back(p);
}

void ObjImporter::addUV(QString line, std::shared_ptr<GeoObject> obj)    
{
}

std::shared_ptr<Group> ObjImporter::getGroup()
{
    return grp;
}

ObjImportNode::ObjImportNode(bool raw)
    : DNode("Obj")
{
    setType("OBJIMPORT");
    if(!raw){
        new DinSocket("Filename", std::string("DIRECTORY"), this);
        new DoutSocket("Group", "GROUPDATA", this);
    }
}

ObjImportNode::ObjImportNode(const ObjImportNode &node)
    : DNode(node)
{
}

std::string ObjImportNode::getFilePath() const
{
    return getInSockets()[0]->getProperty().getData<std::string>();
}

