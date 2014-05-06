#include "fstream"
#include "data/signal.h"
#include "project.h"

using namespace MindTree;

Project* Project::project=0;

Project::Project(std::string filename)
    : filename(filename)
{
    FRG::CurrentProject = this;
    DNSpace *space = 0;
    if(filename != "")
    {
        fromFile(filename);
    }
    else
    {
        space = new DNSpace();
    }
    setRootSpace(space);
    root_scene->setName("Root");
}

void Project::fromFile(std::string filename)
{
    //QFile file(filename);
    //file.open(QIODevice::ReadOnly);
    //QDataStream stream(&file);

    //FRG_PROJECT_HEADER_CHECK
    //{
    //    stream>>&space;
    //    file.close();
    //    LoadNodeIDMapper::clear();
    //    LoadSocketIDMapper::remap();
    //}
    //else
    //    space = new DNSpace;
}

Project::~Project()
{
    spaces.clear();
    delete root_scene;
    //FRG::SpaceDataInFocus = 0;
}

Project* Project::create()    
{
    if(project) delete project;
    project = new Project();
    return project;
}

Project* Project::load(std::string filename)    
{
    if(project) delete project;
    project = new Project(filename.c_str());
    return project;
}

Project* Project::instance()    
{
    return project;
}

//QString Project::registerViewer(ViewerBase *viewer)    
//{
//    return QString(); 
//}

std::string Project::registerNode(DNode *node)    
{
    return "";
}

std::string Project::registerSocket(DSocket *socket)    
{
    return "";
}

std::string Project::registerSocketType(SocketType t)    
{
    return "";
}

//QString Project::registerNodeType(NodeType t)    
//{
//    return QString(); 
//}

std::string Project::registerItem(void* ptr, std::string name)    
{
    int count=0;
    std::string idname = name;
    while(idNames.find(idname) != idNames.end())
    {
       count++;
       idname = name + std::to_string(count);
    }
    idNames.insert({idname, ptr});
    return idname;
}

std::string Project::getIDName(void *ptr)    
{
    for (auto p : idNames)
        if (ptr == p.second) return p.first;
}

void* Project::getItem(std::string idname)    
{
    if(idNames.find(idname) != idNames.end())
        return 0;
    return idNames[idname];
}

void Project::unregisterItem(void *ptr)    
{
    idNames.erase(getIDName(ptr));
}

void Project::unregisterItem(std::string idname)    
{
    idNames.erase(idname);
}

void Project::save()
{
    //if(getFilename() == "")
    //{
    //    setFilename(QFileDialog::getSaveFileName(0));
    //}
    //QFile file(getFilename());
    //file.open(QIODevice::WriteOnly);
    //QDataStream stream(&file);
    //stream<<FRG_PROJECT_HEADER;
    //stream<<getRootSpace();
    //file.close();
}

void Project::saveAs()
{
    //filename = QFileDialog::getSaveFileName(0);
    //QFile file(filename);
    //file.open(QIODevice::WriteOnly);
    //QDataStream stream(&file);
    //stream<<FRG_PROJECT_HEADER;
    //stream<<getRootSpace();
    //file.close();
}

void Project::registerSpace(DNSpace *space)    
{
    spaces.push_back(space);
}

void Project::unregisterSpace(DNSpace *space)    
{
    spaces.erase(std::find(spaces.begin(), spaces.end(), space));
}

std::string Project::getFilename()const
{
	return filename;
}

void Project::setFilename(std::string value)
{
	filename = value;
    MT_CUSTOM_SIGNAL_EMITTER("filename_changed", value);
}

DNSpace* Project::getRootSpace()
{
	return root_scene;
}

void Project::setRootSpace(DNSpace* value)
{
	root_scene = value;
}

//void Project::moveIntoRootSpace()
//{
//    FRG::Space->moveIntoSpace(root_scene);
//}

std::vector<DNodeLink*> Project::getOutLinks(NodeList nodes)
{
    std::vector<DNodeLink*> outs;
    for(DNode *node : nodes)
        for(DoutSocket* socket : node->getOutSockets())
            for(DNodeLink dnlink : socket->getLinks())
                if(std::find(nodes.begin(), 
                             nodes.end(), 
                             dnlink.in->getNode()) == nodes.end())
                    outs.push_back(new DNodeLink(dnlink));
    return outs;
}

std::vector<DNodeLink*> Project::getInLinks(DNode *node)    
{
    NodeList nodes;
    nodes.push_back(node);
    return getInLinks(nodes);
}

std::vector<DNodeLink *> Project::getInLinks(NodeList nodes)
{
    std::vector<DNodeLink *> ins;
    for(DNode *node : nodes)
        for(DinSocket *socket : node->getInSockets())
            if(socket->getCntdSocket()
                &&std::find(nodes.begin(), 
                            nodes.end(), 
                            socket->getCntdSocket()->getNode()) == nodes.end())

                ins.push_back(new DNodeLink(socket, socket->getCntdSocket()));
    return ins;
}

std::vector<DNodeLink*> Project::getOutLinks(DNode *node)    
{
    NodeList nodes;
    nodes.push_back(node);
    return getOutLinks(nodes);
}
