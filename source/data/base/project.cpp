#include "project.h"

#include "QFileDialog"
#include "QGLShader"

#include "source/data/base/frg.h"
#include "source/graphics/base/vnspace.h"

Project::Project(QString filename)
    : filename(filename), glShaderID(0)
{
    FRG::CurrentProject = this;
    DNSpace *space = 0;
    if(filename != "")
    {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QDataStream stream(&file);

        FRG_PROJECT_HEADER_CHECK
        {
            stream>>&space;
            file.close();
            LoadNodeIDMapper::clear();
            LoadSocketIDMapper::remap();
        }
        else
            space = new DNSpace;
    }
    else
    {
        space = new DNSpace;
    }
    setRootSpace(space);
    moveIntoRootSpace();
    root_scene->setName("Root");
}

Project::~Project()
{
    spaces.clear();
    delete root_scene;
    nodePositions.clear();
    FRG::SpaceDataInFocus = 0;
}

void Project::save()
{
    if(getFilename() == "")
    {
        setFilename(QFileDialog::getSaveFileName(0));
    }
    QFile file(getFilename());
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream<<FRG_PROJECT_HEADER;
    stream<<getRootSpace();
    file.close();
}

void Project::saveAs()
{
    filename = QFileDialog::getSaveFileName(0);
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream<<FRG_PROJECT_HEADER;
    stream<<getRootSpace();
    file.close();
}

void Project::registerSpace(DNSpace *space)    
{
    spaces.append(space);
}

void Project::unregisterSpace(DNSpace *space)    
{
    spaces.removeAll(space);
}

QString Project::getFilename()const
{
	return filename;
}

void Project::setFilename(QString value)
{
	filename = value;
}

DNSpace* Project::getRootSpace()
{
	return root_scene;
}

void Project::setRootSpace(DNSpace* value)
{
	root_scene = value;
}

void Project::moveIntoRootSpace()
{
    FRG::Space->moveIntoSpace(root_scene);
}

QList<DNodeLink*> Project::getOutLinks(NodeList nodes)
{
	QList<DNodeLink*> outs;
    foreach(DNode *node, nodes)
        foreach(DoutSocket* socket, node->getOutSockets())
            foreach(DNodeLink dnlink, socket->getLinks())
                if(!nodes.contains(dnlink.in->getNode()))
                    outs.append(new DNodeLink(dnlink));
    return outs;
}

QList<DNodeLink*> Project::getInLinks(DNode *node)    
{
    NodeList nodes;
    nodes.append(node);
    return getInLinks(nodes);
}

QList<DNodeLink *> Project::getInLinks(NodeList nodes)
{
    QList<DNodeLink *> ins;
    foreach(DNode *node, nodes)
        foreach(DinSocket *socket, node->getInSockets())
            if(socket->getCntdSocket()
                &&!nodes.contains(socket->getCntdSocket()->getNode()))
            ins.append(new DNodeLink(socket, socket->getCntdSocket()));
    return ins;
}

QList<DNodeLink*> Project::getOutLinks(DNode *node)    
{
    NodeList nodes;
    nodes.append(node);
    return getOutLinks(nodes);
}

QPointF Project::getNodePosition(const DNode *node)
{
    if(!nodePositions.contains(node)) return QPointF(0, 0);
	return nodePositions.value(node);
}

void Project::setNodePosition(const DNode *node, QPointF value)
{
    if(value == QPointF(0,0))
        nodePositions.remove(node);

	nodePositions[node] = value;
}

void Project::clearNodePosition(const DNode *node)    
{
    nodePositions.remove(node);
}

int Project::regGLSLShader(GLShaderCode *shader)    
{
    glslshaders.insert(++glShaderID, shader);
    return glShaderID;
}

void Project::remGLSLShader(int ID)    
{
    delete glslshaders.take(ID);
}

GLShaderCode* Project::getGLSLShader(int ID)    
{
    return glslshaders.value(ID);
}
