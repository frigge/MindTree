#define GLM_SWIZZLE

#include "GL/glew.h"
#include "memory"
#include "../datatypes/Object/object.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"

#include "render.h"

using namespace MindTree::GL;

Render::Render(std::shared_ptr<Object> o)
    : obj(o), initialized(false), points(true),
        edges(true),
        polygons(true)
{
}

Render::~Render()
{
    for(auto *indices : polyindices)
        delete [] indices;
}

void Render::init()    
{
    initialized = true;
}

void Render::setDrawPoints(bool b)    
{
    points = b;
}

void Render::setDrawEdges(bool b)    
{
    edges = b;
}

void Render::setDrawPolygons(bool b)    
{
    polygons = b;
}

void Render::draw(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config)    
{
    if(!initialized)
        init();

    vao->bind();
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glEnable(GL_POLYGON_OFFSET_POINT);
    glLineWidth(1.1);
    glPolygonOffset(2, 2.);
    if(config.drawPoints()) drawPoints(view, projection);
    if(config.drawEdges()) drawEdges(view, projection);
    if(config.drawPolygons()) drawPolygons(view, projection);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POLYGON_OFFSET_POINT);
    glLineWidth(1);
    vao->release();
}

void Render::drawPoints(const glm::mat4 &view, const glm::mat4 &projection)    
{
    if(!pointProgram) return;
    pointProgram->bind();
    pointProgram->setUniform("modelView", view);
    pointProgram->setUniform("projection", projection);

    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto verts = mesh->getProperty<std::shared_ptr<VertexList>>("P");
    glDrawArrays(GL_POINTS, 0, verts->getSize());
    pointProgram->release();
}

void Render::drawEdges(const glm::mat4 &view, const glm::mat4 &projection)    
{
    if(!edgeProgram) return;
    edgeProgram->bind();
    edgeProgram->setUniform("modelView", view);
    edgeProgram->setUniform("projection", projection);

    
    glMultiDrawElements(GL_LINE_LOOP, //Primitive type
                        (const GLsizei*)&polysizes[0], //polygon sizes
                        GL_UNSIGNED_INT, //index datatype
                        (const GLvoid**)&polyindices[0],
                        polysizes.size()); //primitive count

    edgeProgram->release();
}

void Render::drawPolygons(const glm::mat4 &view, const glm::mat4 &projection)    
{
    if(!polyProgram) return;
    polyProgram->bind();
    polyProgram->setUniform("modelView", view * obj->getWorldTransformation());
    polyProgram->setUniform("projection", projection);

    glMultiDrawElements(GL_TRIANGLE_FAN, //Primitive type
                        (const GLsizei*)&polysizes[0], //polygon sizes
                        GL_UNSIGNED_INT, //index datatype
                        (const GLvoid**)&polyindices[0],
                        polysizes.size()); //primitive count

    polyProgram->release();
}

ShaderProgram* Render::defaultPointProgram()    
{
    auto *prog = new ShaderProgram;
    prog->bind();
    std::string src = 
        "#version 330\n"
        "in vec3 P;\n"
        "uniform mat4 modelView;\n"
        "uniform mat4 projection;\n"
        "void main(){\n"
        "   gl_Position = projection * modelView * vec4(P, 1.);\n"
        "   gl_PointSize = 2;\n"
        "}\n";

    prog->addShaderFromSource(src, GL_VERTEX_SHADER);

    src = 
        "#version 330\n"
        "out vec4 outcolor;\n"
        "void main(){\n"
        "   outcolor = vec4(1,0,0, 1.);\n"
        "}\n";

    prog->addShaderFromSource(src, GL_FRAGMENT_SHADER);
    prog->link();
    prog->release();
    return prog;
}

ShaderProgram* Render::defaultEdgeProgram()    
{
    auto *prog = new ShaderProgram;
    prog->bind();
    auto src = 
        "#version 330\n"
        "uniform mat4 modelView;\n"
        "uniform mat4 projection;\n"
        "in vec3 P;\n"
        "void main(){\n"
        "   gl_Position = projection * modelView * vec4(P, 1);\n"
        "}\n";
    prog->addShaderFromSource(src, GL_VERTEX_SHADER);

    src = 
        "#version 330\n"
        "out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(1.);\n"
        "}\n";

    prog->addShaderFromSource(src, GL_FRAGMENT_SHADER);
    prog->link();
    prog->release();
    return prog;
}

ShaderProgram* Render::defaultPolyProgram()    
{
    auto *prog = new ShaderProgram;
    prog->bind();

    auto src = 
        "#version 330\n"
        "uniform mat4 modelView;\n"
        "uniform mat4 projection;\n"
        "in vec3 P;\n"
        "in vec3 N;\n"
        "out vec3 pos;\n"
        "out vec3 sn;\n"
        "void main(){\n"
        "   gl_Position = projection * modelView * vec4(P, 1);\n"
        "   pos = (modelView * vec4(P, 1)).xyz;\n"
        "   sn = (modelView * vec4(N, 0)).xyz;\n"
        "}\n";
    prog->addShaderFromSource(src, GL_VERTEX_SHADER);

    src = 
        "#version 330\n"
        "in vec3 pos;\n"
        "in vec3 sn;\n"
        "in vec3 eye;\n"
        "out vec4 color;\n"

        "struct Light {"
            "vec3 pos;\n"
            "vec3 color;\n"
            "float intensity;\n"
        "};\n"
        "Light lights[3];\n"
        "vec3 lambert(){\n"
            "vec3 outcol = vec3(0.0);\n"
            "for(int i=0; i<3; i++){\n"
                "Light l = lights[i];\n"
                "vec3 lvec = l.pos - pos ;\n"
                "float cosine = dot(normalize(sn), normalize(lvec));\n"
                "cosine = clamp(cosine, 0.0, 1.0);\n"
                "vec3 col = l.color * l.intensity * cosine;\n"
                "outcol += col * 1./dot(lvec, lvec);\n"
            "}\n"
            "return outcol;\n"
        "}\n"

        "vec3 phong(){\n"
            "vec3 outcol = vec3(0.);\n"
            "for(int i=0; i<3; i++){\n"
                "Light l = lights[i];\n"
                "vec3 lvec = l.pos-pos;\n"
                "vec3 ln = normalize(lvec);\n"
                "vec3 Half = normalize(eye + ln);\n"
                "vec3 nn = normalize(sn);\n"
                "float cosine = clamp(dot(nn, Half), 0., 1.);\n"
                "float roughness =  .01;\n"
                "cosine = pow(cosine, 1./roughness);\n"
                "vec3 col = vec3(1.0) * l.intensity * (cosine);\n"
                "outcol += col/dot(lvec, lvec);\n"
            "}\n"
            "return outcol;\n"
        "}\n"

        "void main(){"
            "lights[0] = Light(vec3(50, 50, -20), vec3(1, .8, .7), 5000.);\n"
            "lights[1] = Light(vec3(-50, 0, 0), vec3(.6, .6, 1), 700.);\n"
            "lights[2] = Light(vec3(0, 0, -50), vec3(.6, .6, 1), 100.);\n"
            "vec3 ambient = vec3(.8, .8, 1.0);\n"
            "float ambientIntensity = .15;\n"
            "float diffint = .8;\n"
            "float specint = .7;\n"
            "color = vec4(lambert()*diffint + ambient*ambientIntensity + phong()*specint, 1);\n"
        "}";

    prog->addShaderFromSource(src, GL_FRAGMENT_SHADER);
    prog->link();
    prog->release();
    return prog;
}

void Render::setPointProgram(ShaderProgram *prog)    
{
    pointProgram = std::unique_ptr<ShaderProgram>(prog);
}

void Render::setEdgeProgram(ShaderProgram *prog)    
{
    edgeProgram = std::unique_ptr<ShaderProgram>(prog);
}

void Render::setPolyProgram(ShaderProgram *prog)    
{
    polyProgram = std::unique_ptr<ShaderProgram>(prog);
}

MeshRender::MeshRender(std::shared_ptr<Object> o)
    : Render(o)
{
}

MeshRender::~MeshRender()
{
}

void MeshRender::generateIndices()    
{
    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto plist = mesh->getProperty<std::shared_ptr<PolygonList>>("polygon");
    for (auto const &p : *plist) {
        //get the size of each polygon
        polysizes.push_back(p.size());
        //get the polygon indices per polygon
        uint* data = new uint[p.size()];
        uint i = 0;
        for (auto index : p.verts()){
            data[i] = index;
            i++;
        }
        polyindices.push_back(data);
    }
}

void MeshRender::tesselate()    
{
    //setting up polygon indices
    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto &plist = *mesh->getProperty<std::shared_ptr<PolygonList>>("polygon");
    auto &verts = *mesh->getProperty<std::shared_ptr<VertexList>>("P");
    int offset = 0;
    for (auto const &p : plist) {
        auto vertsi(p.verts()); //copy indices
        int i=0;
        while(vertsi.size() > 3) {
            // get vectors
            glm::vec3 u = verts[vertsi[i+1]] - verts[vertsi[i]];
            glm::vec3 v = verts[vertsi[i+2]] - verts[vertsi[i+1]];
            glm::vec3 ru = (glm::rotate(glm::mat4(1.f), 90.f, glm::cross(u, v)) * glm::vec4(u, 0.f)).xyz();

            // determine direction
            float dot = glm::dot(glm::normalize(ru), glm::normalize(v));
            if(dot > 1) { // turns left
                triangles.insert(begin(triangles) + offset, {vertsi[i], vertsi[vertsi[i+1]], vertsi[i+2]});
                }
            // if direction is left, triangulate 
            i++; 
        }
    }
}

void MeshRender::init()    
{
    Render::init();
    vao = std::unique_ptr<VAO>(new VAO());
    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());

    generateIndices();
    //tesselate();

    vao->bind();

    setPointProgram(defaultPointProgram());
    setEdgeProgram(defaultEdgeProgram());
    setPolyProgram(defaultPolyProgram());

    auto propmap = mesh->getPropertyMap();
    for(auto propPair : propmap){
        bool pointprog_has = pointProgram->hasAttribute(propPair.first);
        bool edgeprog_has = edgeProgram->hasAttribute(propPair.first);
        bool polyprog_has = polyProgram->hasAttribute(propPair.first);
        if (pointprog_has || edgeprog_has || polyprog_has) {
            //vao->addData(propPair.second.getData<std::shared_ptr<VertexList>>());
            auto vbo = QtContext::getCurrent()->getManager()->getVBO(mesh, propPair.first);
            vbo->bind();
            vbo->data(propPair.second.getData<std::shared_ptr<VertexList>>());
            if(pointprog_has) pointProgram->bindAttributeLocation(vbo->getIndex(), propPair.first);
            if(edgeprog_has) edgeProgram->bindAttributeLocation(vbo->getIndex(), propPair.first);
            if(polyprog_has) polyProgram->bindAttributeLocation(vbo->getIndex(), propPair.first);
            vbo->release();
        }
    }


    //vao->addData(mesh->getProperty<std::shared_ptr<VertexList>>("P"));
    //vao->addData(mesh->getProperty<std::shared_ptr<VertexList>>("N"));
    //vao->setPolygons(plist);
    vao->release();
}

RenderGroup::RenderGroup(std::shared_ptr<Group> g)
    : group(g)
{
    for(auto obj : group->getGeometry()){
        addObject(obj);
    }
}

RenderGroup::~RenderGroup()
{
}

void RenderGroup::addObject(std::shared_ptr<Object> obj)    
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            auto render = new MeshRender(obj);
            for(auto child : obj->getChildren()){
                addObject(std::static_pointer_cast<Object>(child));
            }
            renders.push_back(std::unique_ptr<Render>(render));
            break;
    }
}

void RenderGroup::draw(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config)    
{
    for(auto &render : renders)
        render->draw(view, projection, config);
}

RenderPass::RenderPass()
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::setGeometry(std::shared_ptr<Group> g)    
{
    group = std::make_shared<RenderGroup>(g);
}

void RenderPass::render(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config)
{
    if(!group) return;
    if(target) target->bind();
    group->draw(view, projection, config);
    if(target) target->release();
}

void RenderConfig::setDrawPoints(bool draw)    
{
    _drawPoints = draw;
}

void RenderConfig::setDrawEdges(bool draw)    
{
    _drawEdges = draw;
}

void RenderConfig::setDrawPolygons(bool draw)    
{
    _drawPolygons = draw;
}

bool RenderConfig::drawPoints()    const 
{
    return _drawPoints;
}

bool RenderConfig::drawEdges()    const 
{
    return _drawEdges;
}

bool RenderConfig::drawPolygons()    const 
{
    return _drawPolygons;
}

RenderManager::RenderManager()
{
}

RenderManager::~RenderManager()
{
}

void RenderManager::setConfig(RenderConfig cfg)    
{
    config = cfg;
}

RenderConfig RenderManager::getConfig()    
{
    return config;
}

void RenderManager::addPass(RenderPass *pass)    
{
    passes.push_back(std::unique_ptr<RenderPass>(pass));
}

void RenderManager::removePass(uint index)    
{
    passes.remove(*std::next(passes.begin(), index));
}

RenderPass* RenderManager::getPass(uint index)
{
    return std::next(begin(passes), index)->get();
}

void RenderManager::draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    for(auto &pass : passes){
        pass->render(view, projection, config);
    }
}
