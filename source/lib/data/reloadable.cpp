#include "cache_main.h"
#include <chrono>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include "reloadable.h"

using namespace MindTree;

Library::Library(std::string path) :
    m_path(path), m_handle(nullptr), m_age(0)
{
    m_age = QFileInfo(path.c_str()).lastModified().toMSecsSinceEpoch();
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
    return *this;
}

std::string Library::getPath() const
{
    return m_path;
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

int Library::age() const
{
    return m_age;
}

HotProcessor::HotProcessor(std::string path) :
    m_lib(std::make_unique<Library>(path)), m_proc(nullptr)
{
    if(m_lib->load()) {
        auto loadFn = m_lib->getFunction<CacheProcessor*()>("load");
        m_unloadFn = m_lib->getFunction<void()>("unload");
        m_proc = loadFn();
        DataCache::addProcessor(m_proc);

        std::cout << "added new processor from: " << path << std::endl;
    }
}

HotProcessor::~HotProcessor()
{
    std::cout << "removed processor from: " << getLibPath() << std::endl;
    m_unloadFn();
    DataCache::removeProcessor(m_proc);
}

CacheProcessor *HotProcessor::getProcessor()
{
    return m_proc;
}

std::string HotProcessor::getLibPath() const
{
    return m_lib->getPath();
}

int HotProcessor::age() const
{
    return m_lib->age();
}

std::thread HotProcessorManager::m_watchThread;
std::unordered_map<std::string, std::unique_ptr<HotProcessor>> HotProcessorManager::m_processors;
std::atomic<bool> HotProcessorManager::m_watching{false};

void HotProcessorManager::watch()
{
    QDir libdir("../processors/");
    while(m_watching.load()) {
        auto infos = libdir.entryInfoList();
        for (auto info : infos) {
            auto fp = info.filePath().toStdString();
            auto it = m_processors.find(fp);
            if (it == m_processors.end()) {
                auto prc = std::make_unique<HotProcessor>(fp);
                m_processors.insert(std::make_pair(fp, std::move(prc)));
            }
            else if (info.lastModified().toMSecsSinceEpoch() > it->second->age()) {
                m_processors.erase(fp);
                auto prc = std::make_unique<HotProcessor>(fp);
                m_processors.insert(std::make_pair(fp, std::move(prc)));
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
