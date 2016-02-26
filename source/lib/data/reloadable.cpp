#include <chrono>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include "reloadable_plugin.h"
#include "reloadable.h"

using namespace MindTree;

Library::Library() :
    m_handle(nullptr), m_age(0)
{
}

Library::Library(std::string path) :
    m_path(path), m_handle(nullptr), m_age(0)
{
    m_age = QFileInfo(path.c_str()).lastModified().toMSecsSinceEpoch();
    std::cout << "created lib with age: " << m_age << std::endl;
}

Library::Library(Library &&lib) :
    m_path(lib.m_path),
    m_handle(lib.m_handle),
    m_age(lib.m_age)
{
    lib.m_handle = nullptr;
    lib.m_path = "";
}

Library::~Library()
{
    if(m_handle) unload();
}

Library& Library::operator=(Library &&other)
{
    if(m_handle) unload();
    m_handle = other.m_handle;
    m_path = other.m_path;
    m_age = other.m_age;
    other.m_handle = nullptr;
    other.m_path = "";
    other.m_age = 0;
    return *this;
}

std::string Library::getPath() const
{
    return m_path;
}

bool Library::load()
{
    if(!m_path.empty()) {
        m_handle = dlopen(m_path.c_str(), RTLD_LAZY);
        std::cout << "loaded library: " << m_path << std::endl;
    }
    return m_handle;
}

void Library::unload()
{
    if(m_handle) dlclose(m_handle);
    m_handle = nullptr;
    std::cout << "unloaded library: " << m_path << std::endl;
}

int64_t Library::age() const
{
    return m_age;
}

HotProcessor::HotProcessor(std::string path) :
    m_lib(path), m_proc(nullptr)
{
    if(m_lib.load()) {
        auto loadFn = m_lib.getFunction<plugin::CacheProcessorInfo()>("load");
        m_unloadFn = m_lib.getFunction<void()>("unload");

        auto info = loadFn();
        m_proc = new CacheProcessor(info.socket_type, info.node_type, info.cache_proc);
        DataCache::addProcessor(m_proc);
    }
}

HotProcessor::HotProcessor(HotProcessor &&other) :
    m_lib(std::move(other.m_lib)),
    m_unloadFn(other.m_unloadFn),
    m_proc(other.m_proc)
{
    other.m_proc = nullptr;
    other.m_unloadFn = []{};
}

HotProcessor::~HotProcessor()
{
    m_unloadFn();
    if(m_proc)
        DataCache::removeProcessor(m_proc);
}

HotProcessor& HotProcessor::operator=(HotProcessor &&other)
{
    m_lib = std::move(other.m_lib);
    m_unloadFn = other.m_unloadFn;
    m_proc = other.m_proc;
    return *this;
}

CacheProcessor *HotProcessor::getProcessor()
{
    return m_proc;
}

std::string HotProcessor::getLibPath() const
{
    return m_lib.getPath();
}

int64_t HotProcessor::age() const
{
    return m_lib.age();
}

std::thread HotProcessorManager::m_watchThread;
std::unordered_map<std::string, HotProcessor> HotProcessorManager::m_processors;
std::atomic<bool> HotProcessorManager::m_watching{false};

void HotProcessorManager::watch()
{
    QDir libdir("../processors/");
    while(m_watching.load()) {
        libdir.refresh();
        auto infos = libdir.entryInfoList();
        for (auto info : infos) {
            if(!info.isFile()) continue;

            auto fp = info.filePath().toStdString();
            auto it = m_processors.find(fp);
            if (it == m_processors.end()) {
                std::cout << "new library" << std::endl;
                m_processors.emplace(std::make_pair(fp, HotProcessor(fp)));
            }
            else if (info.lastModified().toMSecsSinceEpoch() > it->second.age()) {
                std::cout << "library changed" << std::endl;
                std::cout << "old age: " << it->second.age() << "\n"
                          << "new age: " << info.lastModified().toMSecsSinceEpoch() << std::endl;
                m_processors.erase(fp);
                m_processors[fp] = HotProcessor(fp);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void HotProcessorManager::start()
{
    std::cout << "start watching libs" << std::endl;

    m_watching = true;
    m_watchThread = std::thread(watch);
}

void HotProcessorManager::stop()
{
    std::cout << "stop watching libs" << std::endl;
    m_watching = false;
}
