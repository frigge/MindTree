#include "data/properties.h"
#include "object.h"
#include "lights.h"
#include "data/cache_main.h"
#include "glm/gtc/matrix_transform.hpp"
#include "data/debuglog.h"

#include "../../3dwidgets/translate_widgets.h"
#include "python.h"

using namespace MindTree;

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

void groupProc(MindTree::DataCache *cache)
{
    auto *node = cache->getNode();

    auto grp = std::make_shared<Group>();
    for (size_t i = 0; i < node->getInSockets().size(); ++i) {
        auto *socket = node->getInSockets()[i];
        if (socket->getCntdSocket()) {
            auto data = cache->getData(i);
            if (data.getType() == "TRANSFORMABLE")
                grp->addMember(data.getData<GeoObjectPtr>());
            else if (data.getType() == "GROUPDATA")
                grp->addMembers(data.getData<GroupPtr>()->getMembers());
        }
            
    }

    cache->pushData(grp);
}

void transformProc(MindTree::DataCache *cache)
{
    auto transformable = cache->getData(0).getData<AbstractTransformablePtr>();
    if(!transformable)
        return;

    glm::vec3 translate = cache->getData(1).getData<glm::vec3>();
    glm::vec3 rotation = cache->getData(2).getData<glm::vec3>();
    glm::vec3 scale = cache->getData(3).getData<glm::vec3>();

    auto newtransformable = transformable->clone();

    glm::mat4 rotx = glm::rotate(glm::mat4(), rotation.x, glm::vec3(1, 0, 0));
    glm::mat4 roty = glm::rotate(glm::mat4(), rotation.y, glm::vec3(0, 1, 0));
    glm::mat4 rotz = glm::rotate(glm::mat4(), rotation.z, glm::vec3(0, 0, 1));

    glm::mat4 rot = rotz * roty * rotx;
    
    glm::mat4 scalemat = glm::scale(glm::mat4(), scale);
    glm::mat4 trans = glm::translate(glm::mat4(), translate);
    glm::mat4 newtrans;
    newtrans = trans * scalemat * rot;

    newtransformable->applyTransform(newtrans);
    
    cache->pushData(newtransformable);
}

void parentProc(MindTree::DataCache *cache)
{
    auto oldparent = cache->getData(0).getData<AbstractTransformablePtr>();
    if(!oldparent)
        return;

    auto parent = oldparent->clone();
    Property childOrChildren = cache->getData(1);
    
    if(childOrChildren.getType() == "GROUPDATA") {
        auto grp = childOrChildren.getData<GroupPtr>();
        for (auto &oldchild : grp->getMembers()) {
            parent->addChild(oldchild->clone());
        }
    }
    cache->pushData(parent);
}

void createTranslateWidget()
{
    auto translater1 = [] () { return std::make_shared<TranslateXWidget>(); };
    auto translater2 = [] () { return std::make_shared<TranslateYWidget>(); };
    auto translater3 = [] () { return std::make_shared<TranslateZWidget>(); };
    auto translater4 = [] () { return std::make_shared<TranslateXYPlaneWidget>(); };
    auto translater5 = [] () { return std::make_shared<TranslateXZPlaneWidget>(); };
    auto translater6 = [] () { return std::make_shared<TranslateYZPlaneWidget>(); };
    auto screenPlaneTranslater = [] () { return std::make_shared<TranslateScreenPlaneWidget>(); };

    Widget3D::registerWidget(translater1);
    Widget3D::registerWidget(translater2);
    Widget3D::registerWidget(translater3);
    Widget3D::registerWidget(translater4);
    Widget3D::registerWidget(translater5);
    Widget3D::registerWidget(translater6);
    Widget3D::registerWidget(screenPlaneTranslater);
}

void regLightProcs()
{
    auto pointLightProc = [](DataCache *cache) {
        float intensity = cache->getData(0).getData<double>();
        glm::vec4 color = cache->getData(1).getData<glm::vec4>();
        auto light = std::make_shared<PointLight>(intensity, color);
        cache->pushData(light);
    };

    auto spotLightProc = [](DataCache *cache) {
        auto intensity = cache->getData(0).getData<double>();
        auto color = cache->getData(1).getData<glm::vec4>();
        auto coneangle = cache->getData(2).getData<double>();
        auto attenuation = cache->getData(3).getData<bool>();
        auto shadowMapping = cache->getData(4).getData<bool>();
        auto shadowSize = cache->getData(5).getData<glm::ivec2>();
        auto near = cache->getData(6).getData<double>();
        auto far = cache->getData(7).getData<double>();
        auto light = std::make_shared<SpotLight>(intensity, color, coneangle);
        Light::ShadowInfo info{shadowMapping, shadowSize};
        info._near = near;
        info._far = far;
        light->setShadowInfo(info);
        cache->pushData(light);
    };

    auto distantLightProc = [](DataCache *cache) {
        float intensity = cache->getData(0).getData<double>();
        glm::vec4 color = cache->getData(1).getData<glm::vec4>();
        auto light = std::make_shared<DistantLight>(intensity, color);
        cache->pushData(light);
    };

    DataCache::addProcessor("TRANSFORMABLE", "POINTLIGHT", new CacheProcessor(pointLightProc));
    DataCache::addProcessor("TRANSFORMABLE", "SPOTLIGHT", new CacheProcessor(spotLightProc));
    DataCache::addProcessor("TRANSFORMABLE", "DISTANTLIGHT", new CacheProcessor(distantLightProc));
}

BOOST_PYTHON_MODULE(object){
    auto proc = new CacheProcessor(groupProc);
    DataCache::addProcessor("GROUPDATA", "GROUP", proc);

    proc = new CacheProcessor(transformProc);
    DataCache::addProcessor("TRANSFORMABLE", "TRANSFORM", proc);

    proc = new CacheProcessor(parentProc);
    DataCache::addProcessor("TRANSFORMABLE", "PARENTOBJECT", proc);

    ObjectDataPyWrapper::wrap();
    ObjectPyWrapper::wrap();
    GroupPyWrapper::wrap();

    createTranslateWidget();

    regLightProcs();
}
