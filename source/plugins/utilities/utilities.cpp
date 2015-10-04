#include "cmath"
#include "boost/python.hpp"
#include "../datatypes/Object/object.h"
#include "data/nodes/node_db.h"
#include "data/cache_main.h"

#include "utilities.h"

using namespace MindTree;

SwitchNode::SwitchNode(bool raw)
{
    setType("SWITCH");
    if(!raw) {
        auto sw = new DinSocket("switch", "INTEGER", this);
        auto out = new DoutSocket("output", "VARIABLE", this);

        setDynamicSocketsNode(DSocket::IN);
        getVarSocket()->listenToLinked();
        out->listenToChange(getVarSocket());
    }
}

void regSwitchNode()
{
    NodeDataBase::registerNodeType(std::make_unique<BuildInDecorator>("SWITCH", 
                                                        "General.Switch",
                                                        [] (bool raw) {
                                                            return std::make_shared<SwitchNode>(raw);
                                                        }));
    auto switchProc = [] (DataCache *cache) {
        const DNode *node = cache->getNode();

        int sw = cache->getData(0).getData<int>();

        sw = std::min(sw, (int)node->getInSockets().size() - 2);
        sw = std::max(0, sw);
        ++sw;

        cache->pushData(cache->getData(sw));
    };

    DataCache::addGenericProcessor("SWITCH", new CacheProcessor(switchProc));
}

void SwitchNode::incVarSocket()
{
    DNode::incVarSocket();
    auto *varsocket = getVarSocket();
    auto *out = getOutSockets().at(0);
    varsocket->toIn()->listenToLinked();
}

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

    regSwitchNode();
}
