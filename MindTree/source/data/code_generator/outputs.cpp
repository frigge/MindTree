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

#include "outputs.h"

#include "QFile"
#include "QTextStream"
#include "QString"
#include "QProcess"
#include "QFileInfo"
#include "QGLShader"

//#include "source/graphics/sourcedock.h"
#include "source/graphics/nodes/graphics_node.h"
#include "data/properties.h"
#include "source/data/code_generator/rslwriter.h"
#include "source/data/code_generator/glslwriter.h"
#include "data/frg.h"
#include "data/project.h"

AbstractOutputNode::AbstractOutputNode(QString name, bool raw)
    : DNode(name)
{
    //sedit = new SourceDock(this);
    if(!raw){
        new DoutSocket("Code", STRING, this);
    }
}

AbstractOutputNode::~AbstractOutputNode()
{
    //delete sedit;
}

AbstractOutputNode::AbstractOutputNode(const AbstractOutputNode* node)
    : DNode(node)
{
    //sedit = new SourceDock(this);
}

//SourceDock* AbstractOutputNode::getSourceEdit()    const
//{
//    return sedit; 
//}

QString AbstractOutputNode::getShaderName() const
{
    QString name = getInSockets().first()->getProperty().getData<QString>();
    return name;
}

QString AbstractOutputNode::getFileName()   const 
{
    QString path = getInSockets().at(1)->getProperty().getData<QString>();
    return path + getShaderName() + ".sl" ;
}

RSLOutputNode::RSLOutputNode(QString name, bool raw)
    : AbstractOutputNode(name, raw)
{
}

RSLOutputNode::RSLOutputNode(const RSLOutputNode* node)
    : AbstractOutputNode(node)
{
}

QString RSLOutputNode::writeCode()
{
    QFile file(getFileName());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    ShaderWriter sw(this);
    QString code = sw.getCode();
    stream<<code;
    file.close();
    return sw.getCode();
}

int RSLOutputNode::compile()    const
{
    QProcess *process = new QProcess();
    QStringList arguments;
    arguments << getFileName();
    process->setStandardErrorFile("compiler.log");
    process->setWorkingDirectory(QFileInfo(getFileName()).canonicalPath());
    process->start("shaderdl", arguments);
    process->waitForFinished(10000);
    return 1;
}

SurfaceOutputNode::SurfaceOutputNode(bool raw)
    : RSLOutputNode("Surface Output", raw)
{
    setNodeType(SURFACEOUTPUT);
    if(!raw){
        //new DinSocket("Name", STRING, this);
        //DinSocket *dirsocket = new DinSocket("Directory", STRING, this);
        //((StringSocketProperty*)dirsocket->getSocketProperty())->setPath(StringSocketProperty::DIRPATH);
        new DinSocket("Ci", COLOR, this);
        new DinSocket("Oi", COLOR, this);
        setDynamicSocketsNode(IN);
    }
}

SurfaceOutputNode::SurfaceOutputNode(const SurfaceOutputNode *node)
    : RSLOutputNode(node)
{
}

DisplacementOutputNode::DisplacementOutputNode(bool raw)
    : RSLOutputNode("Displacement Output", raw)
{
    setNodeType(DISPLACEMENTOUTPUT);
    if(!raw){
        new DinSocket("P", POINT, this);
        new DinSocket("N", NORMAL, this);
        setDynamicSocketsNode(IN);
    }
}

DisplacementOutputNode::DisplacementOutputNode(const DisplacementOutputNode *node)
    : RSLOutputNode(node)
{
}

VolumeOutputNode::VolumeOutputNode(bool raw)
    : RSLOutputNode("Volume Output", raw)
{
    setNodeType(VOLUMEOUTPUT);
    if(!raw){
        new DinSocket("Ci", COLOR, this);
        new DinSocket("Oi", COLOR, this);
        setDynamicSocketsNode(IN);
    }
}

VolumeOutputNode::VolumeOutputNode(const VolumeOutputNode *node)
    : RSLOutputNode(node)
{
}

LightOutputNode::LightOutputNode(bool raw)
    : RSLOutputNode("Light Output", raw)
{
    setNodeType(LIGHTOUTPUT);
    if(!raw){
        new DinSocket("Cl", COLOR, this);
        setDynamicSocketsNode(IN);
    }
}

LightOutputNode::LightOutputNode(const LightOutputNode *node)
    : RSLOutputNode(node)
{
}

GLSLOutputNode::GLSLOutputNode(QString name, bool raw)
    : AbstractOutputNode(name, raw)
{
}

GLSLOutputNode::GLSLOutputNode(const GLSLOutputNode* node)
    : AbstractOutputNode(node)
{
}

QString GLSLOutputNode::getCode()const
{
    return code;
}

void GLSLOutputNode::setCode(QString value)
{
    code = value;
}

QString GLSLOutputNode::writeCode()    
{
    GLSLWriter *w = new GLSLWriter(this);
    code = w->getCode();
    delete w;
    return code;
}

FragmentOutputNode::FragmentOutputNode(bool raw)
    : GLSLOutputNode("fragment output", raw)
{
    setNodeType(FRAGMENTOUTPUT);
    if(!raw){
        //new DinSocket("Name", STRING, this);
        //DinSocket *dirsocket = new DinSocket("Directory", STRING, this);
        //((StringSocketProperty*)dirsocket->getSocketProperty())->setPath(StringSocketProperty::DIRPATH);
        new DinSocket("gl_FragColor", COLOR, this);
        new DinSocket("gl_FragData", POINT, this);
        new DinSocket("gl_FragDepth", FLOAT, this);
        new DoutSocket("ID", INTEGER, this);
    }
}

FragmentOutputNode::FragmentOutputNode(const FragmentOutputNode* node)
    : GLSLOutputNode(node)
{
}

int FragmentOutputNode::compile()    const
{
    GLSLWriter *w = new GLSLWriter(this);
    delete w;
    return 0;
}

VertexOutputNode::VertexOutputNode(bool raw)
    : GLSLOutputNode("vertex output", raw)
{
    setNodeType(VERTEXOUTPUT);
    if(!raw){
        //new DinSocket("Name", STRING, this);
        //DinSocket *dirsocket = new DinSocket("Directory", STRING, this);
        //((StringSocketProperty*)dirsocket->getSocketProperty())->setPath(StringSocketProperty::DIRPATH);
        new DoutSocket("ID", INTEGER, this);
    }
}

VertexOutputNode::VertexOutputNode(const VertexOutputNode* node)
    : GLSLOutputNode(node)
{
}

int VertexOutputNode::compile()    const
{
    GLSLWriter *w = new GLSLWriter(this);
    delete w;
    return 0;
}

GeometryOutputNode::GeometryOutputNode(bool raw)
    : GLSLOutputNode("geometry output", raw)
{
    setNodeType(GEOMETRYOUTPUT);
    if(!raw){
        //new DinSocket("Name", STRING, this);
        //DinSocket *dirsocket = new DinSocket("Directory", STRING, this);
        //((StringSocketProperty*)dirsocket->getSocketProperty())->setPath(StringSocketProperty::DIRPATH);
        new DoutSocket("ID", INTEGER, this);
    }
}

GeometryOutputNode::GeometryOutputNode(const GeometryOutputNode* node)
    : GLSLOutputNode(node)
{
}

int GeometryOutputNode::compile()    const
{
    GLSLWriter *w = new GLSLWriter(this);
    delete w;
    return 0;
}

