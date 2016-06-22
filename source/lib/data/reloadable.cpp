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

Library::Library(const std::string &path, int64_t age) :
    m_path(path),
    m_handle(nullptr),
    m_age(age ? age : QFileInfo(path.c_str()).lastModified().toMSecsSinceEpoch())
{
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
        if(!m_handle)
            std::cout << "library loading failed: " << dlerror() << std::endl;
        else
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

HotProcessor::HotProcessor(const std::string &path, int64_t age) :
    m_lib(path, age), m_proc(nullptr)
{
    if(m_lib.load()) {
        auto loadFn = m_lib.getFunction<CacheProcessorInfo()>("load");
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
    other.m_unloadFn = nullptr;
}

HotProcessor::~HotProcessor()
{
    if(m_proc)
        DataCache::removeProcessor(m_proc);
    if(m_unloadFn) m_unloadFn();
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
std::atomic<bool> HotProcessorManager::m_init{false};
std::mutex HotProcessorManager::m_initMutex;
std::condition_variable HotProcessorManager::m_initCondition;

void HotProcessorManager::watch()
{
    while(m_watching.load()) {
        QDir libdir("../processors/");
        auto infos = libdir.entryInfoList();
        for (auto info : infos) {
           if(!info.isFile()) continue;

            auto fp = info.filePath().toStdString();
            auto it = m_processors.find(fp);
            int64_t age = info.lastModified().toMSecsSinceEpoch();
            if (it == m_processors.end()) {
                std::cout << "new library" << std::endl;
                m_processors.emplace(std::make_pair(fp, HotProcessor(fp)));
            }
            else if (age > it->second.age()) {
                std::cout << "library changed" << std::endl;
                std::cout << "old age: " << it->second.age() << "\n"
                          << "new age: " << age << std::endl;
                m_processors.erase(fp);
                m_processors.emplace(std::make_pair(fp, HotProcessor(fp, age)));
            }
        }
        if(!m_init) {
            m_init = true;
            m_initCondition.notify_all();
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void HotProcessorManager::start()
{
    std::cout << "start watching libs" << std::endl;

    m_watching = true;
    m_watchThread = std::thread(watch);

    if(!m_init) {
        std::unique_lock<std::mutex> lock(m_initMutex);
        m_initCondition.wait(lock);
    }
}

void HotProcessorManager::stop()
{
    std::cout << "stop watching libs" << std::endl;
    m_watching = false;
}
