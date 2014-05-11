#include "obj.h"
#include "boost/python.hpp"
#include "data/cache_main.h"
#include "data/nodes/node_db.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(objio)
{
    auto importFn = []
    {
        return new ObjImportNode();
    };

    NodeDataBase::registerNodeType(new BuildInDecorator("OBJIMPORT", 
                                                      "Objects.Import",
                                                      importFn));

    auto proc = [] (MindTree::DataCache* cache)
    {
        const ObjImportNode *node = cache->getNode()
            ->getDerivedConst<ObjImportNode>();

        ObjImporter objio(node->getFilePath());
        cache->pushData(objio.getGroup());
    };

    DataCache::addProcessor(SocketType("GROUPDATA"), 
                            NodeType("OBJIMPORT"), 
                            new CacheProcessor(proc));
}
