#ifndef MT_BASE_RELOADABLE
#define MT_BASE_RELOADABLE

#include "string"
#include <dlfcn.h>

namespace MindTree {
class Library
{
public:
    Library(std::string path);
    Library(const Library &lib) = delete;
    Library(const Library &&lib);

    void load();
    void unload();

    Library& operator=(const Library&) = delete;
    Library& operator=(const Library&&);

    template<typename T>
    std::function<T> getFunction(std::string symbol)
    {
        if(!m_handle) return;
        return std::function<T>(dlsym(m_handle, symbol.c_str()));
    }

private:
    void getFunction();
    std::string m_path;
    void *m_handle{nullptr};
};

}

#endif
