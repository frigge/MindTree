#ifndef MT_BASE_RELOADABLE
#define MT_BASE_RELOADABLE

#include <string>
#include <functional>
#include <dlfcn.h>

#include "cache_main.h"

namespace MindTree {
class Library
{
public:
    Library(std::string path);
    Library(const Library &lib) = delete;
    Library(Library &&lib);

    ~Library();

    void load();
    void unload();

    Library& operator=(const Library&) = delete;
    Library& operator=(Library&&);

    template<typename T>
    std::function<T> getFunction(std::string symbol)
    {
        if(!m_handle) return;
        return std::function<T>(dlsym(m_handle, symbol.c_str()));
    }

private:
    std::string m_path;
    void *m_handle{nullptr};
};

class HotProcessor
{
public:
    HotProcessor(std::string path);

private:
    Library m_lib;
    CacheProcessor *m_proc;
};

}

#endif
