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

    NodeDataBase::registerNodeType(new BuildInFactory("OBJIMPORTNODE", 
                                                      "Objects.Import",
                                                      importFn));

    auto proc = [] (MindTree::DataCache* cache)
    {
        const ObjImportNode *node = cache->getNode()
            ->getDerivedConst<ObjImportNode>();

        ObjImporter objio(node->getFilePath());
        cache->data = objio.getGroup();
    };

    DataCache::addProcessor(SocketType("GROUPDATA"), 
                            NodeType("OBJIMPORTNODE"), 
                            new CacheProcessor(proc));
}
