#include "fstream"
#include "data/signal.h"
#include "data/io.h"
#include "project.h"

using namespace MindTree;

Project* Project::_project=nullptr;

Project::Project(std::string filename) noexcept
    : filename(filename)
{
    DNSpace *space = nullptr;
    if(filename != "")
    {
        space = fromFile(filename);
    }
    else
    {
        space = new DNSpace();
    }
    setRootSpace(space);
    root_scene->setName("Root");
    MT_CUSTOM_SIGNAL_EMITTER("newProject", this);
}

DNSpace* Project::fromFile(std::string filename)
{
    IO::InStream stream(filename);
    auto space = new DNSpace();
    stream >> *space;
    LoadSocketIDMapper::remap();
    return space;
}

Project::~Project()
{
    delete root_scene;
}

Project* Project::create()    
{
    if(_project) 
        delete _project;
    _project = new Project();
    return _project;
}

Project* Project::load(std::string filename)    
{
    if(_project) delete _project;
    _project = new Project(filename);
    return _project;
}

Project* Project::instance()    
{
    return _project;
}

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
    return "";
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
    IO::OutStream stream(filename);
    stream << *root_scene;
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
