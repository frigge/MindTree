#include "reloadable.h"

using namespace MindTree;

Library::Library(std::string path) :
    m_path(path), m_handle(nullptr)
{
}

Library::Library(const Library &&lib) :
    m_path(lib.m_path), m_handle(lib.m_handle)
{
    lib.m_handle = nullptr;
    lib.m_path = "";
}

Library::~Library()
{
    unload();
}

Library& Library::operator=(const Library &&other)
{
    m_handle = other.m_handle;
    m_path = other.m_path;
    other.m_handle = nullptr;
    other.m_path = "";
}

void Library::load()
{
    if(m_path)
        m_handle = dlopen(m_path);
}

void Library::unload()
{
    if(m_handle) dlclose(m_handle);
    m_handle = nullptr;
}
