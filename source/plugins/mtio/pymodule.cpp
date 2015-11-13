#include "obj.h"
#include "boost/python.hpp"
#include "data/cache_main.h"
#include "data/nodes/node_db.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(objio)
{
    auto importFn = [] (bool raw)
    {
        return std::make_shared<ObjImportNode>(raw);
    };

    NodeDataBase::registerNodeType(std::make_unique<BuildInDecorator>("OBJIMPORT", 
                                                      "Objects.Import",
                                                      importFn));

    auto proc = [] (MindTree::DataCache* cache)
    {
        const ObjImportNode *node = cache->getNode()
            ->getDerivedConst<ObjImportNode>();

        ObjImporter objio(node->getFilePath());
        cache->pushData(objio.getGroup());
    };

    DataCache::addProcessor(new CacheProcessor(SocketType("GROUPDATA"),
                                               NodeType("OBJIMPORT"),
                                               proc));
}
