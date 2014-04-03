#include "data/properties.h"
#include "object.h"
#include "python.h"

ObjectDataPyWrapper::ObjectDataPyWrapper(ObjectData *data)
    : PyWrapper(data)
{
}

ObjectDataPyWrapper::~ObjectDataPyWrapper()
{
}

void ObjectDataPyWrapper::wrap()    
{
    BPy::class_<ObjectDataPyWrapper>("ObjectData", BPy::no_init);
}

ObjectPyWrapper::ObjectPyWrapper(GeoObject *obj)
    : PyWrapper(obj)
{
}

ObjectPyWrapper::~ObjectPyWrapper()
{
}

void ObjectPyWrapper::wrap()    
{
    MindTree::PropertyData<VertexList>::registerType("VertexList");
    BPy::class_<ObjectPyWrapper>("Object", BPy::no_init);
}

GroupPyWrapper::GroupPyWrapper(Group *grp)
    : PyWrapper(grp)
{
}

GroupPyWrapper::~GroupPyWrapper()
{
}

void GroupPyWrapper::wrap()    
{
    BPy::class_<GroupPyWrapper>("Group", BPy::no_init);
}

BOOST_PYTHON_MODULE(object){
    MindTree::PropertyData<Group>::registerType("GROUPDATA");
    MindTree::PropertyData<GeoObject>::registerType("SCENEOBJECT");
    ObjectDataPyWrapper::wrap();
    ObjectPyWrapper::wrap();
    GroupPyWrapper::wrap();

    //register Nodes
    MindTree::NodeDataBase::registerNodeType(new MindTree::BuildInFactory("OBJECT", "Objects.Object",
                                    []{
                                        return new ObjectNode();
                                    }));

    MindTree::NodeDataBase::registerNodeType(new MindTree::BuildInFactory("SCENEGROUP", "Objects.Group",
                                    []{
                                        return new CreateGroupNode();
                                    }));

    MindTree::NodeDataBase::registerNodeType(new MindTree::BuildInFactory("CAMERANODE", "Objects.Camera",
                                    []{
                                        return new CreateGroupNode();
                                    }));

    MindTree::NodeDataBase::registerNodeType(new MindTree::BuildInFactory("LIGHTNODE", "Objects.Light",
                                    []{
                                        return new CreateGroupNode();
                                    }));
}
