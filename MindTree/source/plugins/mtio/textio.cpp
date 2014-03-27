#include "boost/python.hpp"
#include "data/cache_main.h"
#include "iostream"
#include "fstream"
#include "textio.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(textio) {
    auto readText = [] (DataCache *cache) {
        std::string filename = cache->getData(0).getData<std::string>();
        std::ifstream stream(filename); 

        std::string content;
        std::string line;
        while (stream) {
            std::getline(stream, line);
            content += line;
            content += "\n";
        }
        
        cache->pushData(content);
    };

    DataCache::addProcessor("STRING", "TEXTREAD", new CacheProcessor(readText));
}
