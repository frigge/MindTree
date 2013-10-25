#include "obj.h"
#include "boost/python.hpp"
#include "data/cache_main.h"
#include "data/nodes/node_db.h"

BOOST_PYTHON_MODULE(objio){
    MindTree::NodeDataBase::registerNodeType(new MindTree::BuildInFactory("OBJIMPORTNODE", "Objects.Import",
                []{
                    return new ObjImportNode();
                }));

    MindTree::DataCache::addProcessor(MindTree::SocketType("GROUPDATA"), 
                                    MindTree::NodeType("OBJIMPORTNODE"), 
                                    new MindTree::CacheProcessor(
                [](MindTree::DataCache* cache){
                    ObjImportNode *node = cache->getNode()->getDerived<ObjImportNode>();
                    ObjImporter objio(node);
                    auto grp = std::make_shared<Group>();
                    grp->addMembers(node->getGroup()->getMembers());
                    cache->data = grp;
                }));
}
