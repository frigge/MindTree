#include "cache_main.h"
#include "reloadable.h"

using namespace MindTree;

Library::Library(std::string path) :
    m_path(path), m_handle(nullptr)
{
}

Library::Library(Library &&lib) :
    m_path(lib.m_path), m_handle(lib.m_handle)
{
    lib.m_handle = nullptr;
    lib.m_path = "";
}

Library::~Library()
{
    unload();
}

Library& Library::operator=(Library &&other)
{
    if(m_handle) unload();
    m_handle = other.m_handle;
    m_path = other.m_path;
    other.m_handle = nullptr;
    other.m_path = "";
}

bool Library::load()
{
    if(!m_path.empty())
        m_handle = dlopen(m_path.c_str(), RTLD_LAZY);
    return m_handle;
}

void Library::unload()
{
    if(m_handle) dlclose(m_handle);
    m_handle = nullptr;
}

HotProcessor::HotProcessor(std::string path) :
    m_lib(path), m_proc(nullptr)
{
    if(m_lib.load()) {
        auto loadFn = m_lib.getFunction<void(DataCache*)>("load");
        m_proc = loadFn();
        DataCache::addProcessor(m_proc);
    }
}

