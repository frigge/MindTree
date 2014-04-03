#include "boost/python.hpp"
#include "../datatypes/Object/object.h"
#include "data/cache_main.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(utilities) {
    auto addPropertiesProc = [](DataCache *cache) {
        auto grp = cache->getData(0).getData<std::shared_ptr<Group>>();
        auto prop_name = cache->getData(1).getData<std::string>();
        auto prop_value = cache->getData(2);

        for (auto obj : grp->getMembers()) {
            obj->setProperty(prop_name, prop_value);
        }
    };

    DataCache::addProcessor("GROUPDATA", 
                            "ADDPROPERTIES", 
                            new CacheProcessor(addPropertiesProc));
}
