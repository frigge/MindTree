#include "boost/python.hpp"
#include "../datatypes/Object/object.h"
#include "data/cache_main.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(utilities) {
    auto addPropertiesProc = [](DataCache *cache) {
        auto grp = cache->getData(0).getData<std::shared_ptr<Group>>();
        auto new_grp = std::make_shared<Group>(*grp);

        int prop_cnt = cache->getNode()->getInSockets().size();

        for(int i = 1; i <= prop_cnt; i += 2) {
            auto prop_name = cache->getData(i).getData<std::string>();
            auto prop_value = cache->getData(i + 1);


            new_grp->setProperty(prop_name, prop_value);
        }

        cache->pushData(new_grp);
    };

    DataCache::addProcessor("GROUPDATA", 
                            "ADDPROPERTIES", 
                            new CacheProcessor(addPropertiesProc));
}
