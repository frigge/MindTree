#ifndef MT_BASE_RELOADABLE
#define MT_BASE_RELOADABLE

#include <atomic>
#include <dlfcn.h>
#include <functional>
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
    Library(std::string path);
    Library(const Library &lib) = delete;
    Library(Library &&lib);

    ~Library();

    bool load();
    void unload();

    int64_t age() const;

    Library& operator=(const Library&) = delete;
    Library& operator=(Library&&);

    template<typename T>
    std::function<T> getFunction(std::string symbol)
    {
        if(!m_handle) return std::function<T>();
        CacheProcessor* (*fn)();
        fn = reinterpret_cast<decltype(fn)>(dlsym(m_handle, symbol.c_str()));
        return std::function<T>(fn);
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
    HotProcessor(std::string path);

    ~HotProcessor();
    CacheProcessor* getProcessor();
    int64_t age() const;

    std::string getLibPath() const;

private:
    std::unique_ptr<Library> m_lib;
    std::function<void()> m_unloadFn;
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
    static std::unordered_map<std::string, std::unique_ptr<HotProcessor>> m_processors;
    static std::atomic<bool> m_watching;
};

}

#endif
