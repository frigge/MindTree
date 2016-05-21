#include "data/properties.h"
#include "object.h"
#include "lights.h"
#include "data/cache_main.h"
#include "glm/gtc/matrix_transform.hpp"
#include "data/debuglog.h"

#include "data/nodes/node_db.h"

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
    glm::vec3 translate = cache->getData(0).getData<glm::vec3>();
    glm::vec3 rotation = cache->getData(1).getData<glm::vec3>();
    glm::vec3 scale = cache->getData(2).getData<glm::vec3>();

    glm::mat4 rotx = glm::rotate(glm::mat4(), glm::radians(rotation.x), glm::vec3(1, 0, 0));
    glm::mat4 roty = glm::rotate(glm::mat4(), glm::radians(rotation.y), glm::vec3(0, 1, 0));
    glm::mat4 rotz = glm::rotate(glm::mat4(), glm::radians(rotation.z), glm::vec3(0, 0, 1));

    glm::mat4 rot = rotz * roty * rotx;

    glm::mat4 scalemat = glm::scale(glm::mat4(), scale);
    glm::mat4 trans = glm::translate(glm::mat4(), translate);
    glm::mat4 newtrans;
    newtrans = trans * scalemat * rot;

    cache->pushData(newtrans);
}

void transformObjProc(MindTree::DataCache *cache)
{
    auto transformable = cache->getData(0).getData<AbstractTransformablePtr>();
    auto trans = cache->getData(1).getData<glm::mat4>();
    if(!transformable)
        return;

    auto newtransformable = transformable->clone();
    newtransformable->applyTransform(trans);

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

void emptyProc(MindTree::DataCache *cache)
{
    auto empty = std::make_shared<Empty>();
    auto trans = cache->getData(0).getData<glm::mat4>();
    auto children = cache->getData(1).getData<GroupPtr>();
    empty->setTransformation(trans);
    if(children)
        for (const auto &child : children->getMembers())
            empty->addChild(child);
    cache->pushData(empty);
}

void cameraProc(MindTree::DataCache *cache)
{
    auto trans = cache->getData(0).getData<glm::mat4>();
    auto children = cache->getData(1).getData<GroupPtr>();

    auto fov = cache->getData(2).getData<double>();
    auto res = cache->getData(3).getData<glm::ivec2>();
    auto near = cache->getData(4).getData<double>();
    auto far = cache->getData(5).getData<double>();

    auto cam = std::make_shared<Camera>();
    cam->setTransformation(trans);
    if(children)
        for (const auto &child : children->getMembers())
            cam->addChild(child);

    cam->setFov(fov);
    cam->setNear(near);
    cam->setFar(far);

    cache->pushData(cam);
}

void createTranslateWidget()
{
    auto translater1 = [] () { return std::make_unique<TranslateXWidget>(); };
    auto translater2 = [] () { return std::make_unique<TranslateYWidget>(); };
    auto translater3 = [] () { return std::make_unique<TranslateZWidget>(); };
    auto translater4 = [] () { return std::make_unique<TranslateXYPlaneWidget>(); };
    auto translater5 = [] () { return std::make_unique<TranslateXZPlaneWidget>(); };
    auto translater6 = [] () { return std::make_unique<TranslateYZPlaneWidget>(); };
    auto screenPlaneTranslater = [] () { return std::make_unique<TranslateScreenPlaneWidget>(); };

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
        auto trans = cache->getData(0).getData<glm::mat4>();
        auto children = cache->getData(1).getData<GroupPtr>();
        float intensity = cache->getData(2).getData<double>();
        glm::vec4 color = cache->getData(3).getData<glm::vec4>();
        auto light = std::make_shared<PointLight>(intensity, color);
        light->setTransformation(trans);
        if(children)
            for (const auto &child : children->getMembers())
                light->addChild(child);
        cache->pushData(light);
    };

    auto spotLightProc = [](DataCache *cache) {
        auto trans = cache->getData(0).getData<glm::mat4>();
        auto children = cache->getData(1).getData<GroupPtr>();
        auto intensity = cache->getData(2).getData<double>();
        auto color = cache->getData(3).getData<glm::vec4>();
        auto coneangle = cache->getData(4).getData<double>();
        auto attenuation = cache->getData(5).getData<bool>();
        auto shadowMapping = cache->getData(6).getData<bool>();
        auto shadowSize = cache->getData(7).getData<int>();
        auto shadowBias = cache->getData(8).getData<double>();
        auto near = cache->getData(9).getData<double>();
        auto far = cache->getData(10).getData<double>();

        auto light = std::make_shared<SpotLight>(intensity, color, coneangle);

        light->setTransformation(trans);
        if(children)
            for (const auto &child : children->getMembers())
                light->addChild(child);

        Light::ShadowInfo info{shadowMapping, glm::ivec2(shadowSize)};
        info._near = near;
        info._far = far;
        info._bias = shadowBias;
        light->setShadowInfo(info);
        cache->pushData(light);
    };

    auto distantLightProc = [](DataCache *cache) {
        auto trans = cache->getData(0).getData<glm::mat4>();
        auto children = cache->getData(1).getData<GroupPtr>();
        float intensity = cache->getData(2).getData<double>();
        glm::vec4 color = cache->getData(3).getData<glm::vec4>();
        auto light = std::make_shared<DistantLight>(intensity, color);

        light->setTransformation(trans);
        if(children)
            for (const auto &child : children->getMembers())
                light->addChild(child);

        cache->pushData(light);
    };

    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "POINTLIGHT", pointLightProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "SPOTLIGHT", spotLightProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "DISTANTLIGHT", distantLightProc));
}

void materialProcs()
{
    auto materialInstanceProc = [] (DataCache *cache) {
        auto diff_color = cache->getData(0).getData<glm::vec4>();
        auto diff_int = cache->getData(1).getData<double>();
        auto spec_int = cache->getData(2).getData<double>();

        auto material = std::make_shared<DefaultMaterial>();
        material->setProperty("diffuse_color", diff_color);
        material->setProperty("specular_intensity", spec_int);
        material->setProperty("diffuse_intensity", diff_int);

        cache->pushData(material);
    };

    auto setMaterialProc = [] (DataCache *cache) {
        auto obj = cache->getData(0).getData<GeoObjectPtr>();
        if(obj->getType() != AbstractTransformable::GEO)
            return;

        auto mat = cache->getData(1).getData<MaterialPtr>();
        auto instance = std::make_shared<MaterialInstance>(mat);

        auto newObj = obj->clone();
        std::dynamic_pointer_cast<GeoObject>(newObj)->setMaterial(instance);

        cache->pushData(newObj);
    };

    DataCache::addProcessor(new CacheProcessor("MATERIAL", "DEFAULTMATERIAL", materialInstanceProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "SETMATERIALINSTANCE", setMaterialProc));
}

void objectProc(DataCache *cache)
{
    auto obj = std::make_shared<GeoObject>();

    auto trans = cache->getData(0).getData<glm::mat4>();
    auto children = cache->getData(1).getData<GroupPtr>();
    obj->setTransformation(trans);
    if(children)
        for (const auto &child : children->getMembers())
            obj->addChild(child);

    auto data = cache->getData(2).getData<ObjectDataPtr>();
    auto mat = cache->getData(3).getData<MaterialPtr>();

    if(data) obj->setData(data);
    if (mat) {
        auto instance = std::make_shared<MaterialInstance>(mat);
        obj->setMaterial(instance);
    }

    cache->pushData(obj);
}

void registerSkeleton()
{
    auto jointProc = [](DataCache *cache) {
        auto trans = cache->getData(0).getData<glm::mat4>();
        auto children = cache->getData(1).getData<std::vector<std::unique_ptr<Joint>>>();
    };

    auto skeletonProc = [] (DataCache *cache) {
        auto skel = std::make_shared<Skeleton>();

        auto trans = cache->getData(0).getData<glm::mat4>();
        auto children = cache->getData(1).getData<GroupPtr>();
        empty->setTransformation(trans);
        if(children)
            for (const auto &child : children->getMembers())
                empty->addChild(child);

        auto joints = cache->getData(2).getData<std::vector<std::unique_ptr<Joints>>>();
    };
}

BOOST_PYTHON_MODULE(object){
    DataCache::addProcessor(new CacheProcessor("GROUPDATA", "GROUP", groupProc));
    DataCache::addProcessor(new CacheProcessor("MAT4", "TRANSFORM", transformProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFOMRABLE", "TRANSFORMOBJECT", transformObjProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "PARENTOBJECT", parentProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "EMPTY",  emptyProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "CAMERA", cameraProc));
    DataCache::addProcessor(new CacheProcessor("TRANSFORMABLE", "OBJECTNODE", objectProc));

    NodeDataBase::setNotConvertible("TRANSFORMABLE");

    ObjectDataPyWrapper::wrap();
    ObjectPyWrapper::wrap();
    GroupPyWrapper::wrap();

    createTranslateWidget();

    regLightProcs();
    materialProcs();
}
