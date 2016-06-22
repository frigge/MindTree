#ifndef MT_BASE_RELOADABLE
#define MT_BASE_RELOADABLE

#include <atomic>
#include <dlfcn.h>
#include <memory>
#include <string>
#include <thread>
#include <cstdint>
#include <unordered_map>

#include "cache_main.h"

namespace MindTree {
class Library
{
public:
    Library();
    Library(const std::string &path, int64_t age=0);
    Library(const Library &lib) = delete;
    Library(Library &&lib);

    ~Library();

    bool load();
    void unload();

    int64_t age() const;

    Library& operator=(const Library&) = delete;
    Library& operator=(Library&&);

    template<typename T>
    T* getFunction(const std::string &symbol)
    {
        if(!m_handle) return nullptr;
        return reinterpret_cast<T*>(dlsym(m_handle, symbol.c_str()));
    }

    std::string getPath() const;

private:
    std::string m_path;
    void *m_handle{nullptr};
    int64_t m_age;
};

class HotProcessor
{
public:
    HotProcessor(const std::string &path="", int64_t age=0);
    HotProcessor(HotProcessor &&other);

    ~HotProcessor();

    HotProcessor& operator=(HotProcessor &&other);
    CacheProcessor* getProcessor();
    int64_t age() const;

    std::string getLibPath() const;

private:
    Library m_lib;
    void (*m_unloadFn)();
    CacheProcessor *m_proc;
};

class HotProcessorManager
{
public:
    static void start();
    static void stop();
    static void watch();

private:
    static std::thread m_watchThread;
    static std::unordered_map<std::string, HotProcessor> m_processors;
    static std::atomic<bool> m_watching;
    static std::condition_variable m_initCondition;
    static std::mutex m_initMutex;
    static std::atomic<bool> m_init;
};

}

#endif
