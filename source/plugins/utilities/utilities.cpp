#include "boost/python.hpp"
#include "../datatypes/Object/object.h"
#include "data/cache_main.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(utilities) {
    auto addPropertiesProc = [](DataCache *cache) {
        auto grp = cache->getData(0).getData<std::shared_ptr<Group>>();
        auto new_grp = std::make_shared<Group>(*grp);

        auto sockets = cache->getNode()->getInSockets();
        int prop_cnt = sockets.size();

        for(int i = 0; i < prop_cnt; ++i) {
            auto prop_name = sockets[i]->getName();
            auto prop_value = cache->getData(i);

            new_grp->setProperty(prop_name, prop_value);
        }

        cache->pushData(new_grp);
    };

    DataCache::addProcessor("GROUPDATA", 
                            "ADDPROPERTIES", 
                            new CacheProcessor(addPropertiesProc));
}
