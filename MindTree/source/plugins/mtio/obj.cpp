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

#include "obj.h"
#include "glm/glm.hpp"
#include "QFile"
#include "QFileInfo"

using namespace MindTree;

ObjImporter::ObjImporter(std::string filepath)
{
    QFile file(filepath.c_str());
    file.open(QFile::ReadOnly);
    QFileInfo fi(file);
    if(!fi.exists()) return;
    QTextStream stream(&file);

    readData(stream);
}

ObjImporter::~ObjImporter()
{
}

void ObjImporter::readData(QTextStream &stream)    
{
    std::shared_ptr<Object> obj;

    while(!stream.atEnd()) {
        QString line = stream.readLine();
        if(line.startsWith("o ")) {
            if(obj) std::static_pointer_cast<MeshData>(obj->getData())->computeVertexNormals();
            obj = addObject(line);
        }
        else if(line.startsWith("v "))
                addVertex(line, obj);
        else if(line.startsWith("f "))
                addFace(line, obj);
        else if(line.startsWith("st "))
            addUV(line, obj);
    }
    std::static_pointer_cast<MeshData>(obj->getData())->computeVertexNormals();
}

std::shared_ptr<Object> ObjImporter::addObject(QString line)    
{
    auto obj = std::make_shared<Object>();
    if(!grp) grp = std::make_shared<Group>();
    grp->addMember(obj);
    QStringList l = line.split(" ");
    l.takeFirst();
    obj->setName(l[0].toStdString());
    auto mesh = std::make_shared<MeshData>();
    mesh->addProperty("P", std::make_shared<VertexList>());
    mesh->addProperty("polygon", std::make_shared<PolygonList>());
    obj->setData(mesh);
    return obj;
}

void ObjImporter::addVertex(QString line, std::shared_ptr<Object> obj)
{
    QStringList l = line.split(" ");
    double d[3];
    int i=0;
    l.takeFirst();
    foreach(QString vstr, l){
        d[i] = vstr.toDouble();
        ++i;
    }
    std::static_pointer_cast<MeshData>(obj->getData())
        ->getProperty<std::shared_ptr<VertexList>>("P")->push_back(glm::vec3(d[0], d[1], d[2]));
}

void ObjImporter::addFace(QString line, std::shared_ptr<Object> obj)    
{
    QStringList l = line.split(" ");
    std::vector<uint> p;
    l.takeFirst();
    foreach(QString vstr, l){
        QStringList tmp = vstr.split("/");
        p.push_back(tmp.at(0).toInt() - 1);
    }
    Polygon poly;
    poly.set(p);
    std::static_pointer_cast<MeshData>(obj->getData())
        ->getProperty<std::shared_ptr<PolygonList>>("polygon")->push_back(poly);
}

void ObjImporter::addUV(QString line, std::shared_ptr<Object> obj)    
{
}

std::shared_ptr<Group> ObjImporter::getGroup()
{
    return grp;
}

ObjImportNode::ObjImportNode(bool raw)
    : DNode("Obj")
{
    setNodeType("OBJIMPORT");
    if(!raw){
        filesocket = new DinSocket("Filename", std::string("DIRECTORY"), this);
        new DoutSocket("Group", "GROUPDATA", this);
    }
}

ObjImportNode::ObjImportNode(const ObjImportNode &node)
    : DNode(node)
{
}

DinSocket* ObjImportNode::getFileSocket()    
{
    return filesocket; 
}

std::string ObjImportNode::getFilePath() const
{
    return filesocket->getProperty().getData<std::string>();
}

