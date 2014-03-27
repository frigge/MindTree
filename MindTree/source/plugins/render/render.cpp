#define GLM_SWIZZLE

#include "GL/glew.h"
#include "memory"
#include "../datatypes/Object/object.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"

#include "render.h"

using namespace MindTree::GL;

Render::Render(std::shared_ptr<Object> o)
    : obj(o), initialized(false)
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

void Render::draw(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config)    
{
    if(!initialized)
        init();

    auto props = obj->getPropertyMap();
    glm::mat4 model = obj->getWorldTransformation();
    glm::mat4 modelView = view * model;
    vao->bind();
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glEnable(GL_POLYGON_OFFSET_POINT);
    glLineWidth(1.1);
    glPolygonOffset(2, 2.);
    if(config.drawPoints()) drawPoints(modelView, projection);
    if(config.drawEdges()) drawEdges(modelView, projection);
    if(config.drawPolygons()) drawPolygons(modelView, projection, config);

    if(props.find("display.drawVertexNormal") != props.end())
        if(props["display.drawVertexNormal"].getData<bool>())
            drawVertexNormals(modelView, projection);

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
    
    glm::vec4 color = glm::vec4(1, 0, 0, 1);
    auto props = obj->getPropertyMap();
    if(props.find("display.pointColor") != props.end())
        color = props["display.pointColor"].getData<glm::vec4>();

    pointProgram->setUniform("color", color);

    int pointSize = 3;
    if(props.find("display.pointSize") != props.end())
        pointSize = props["display.pointSize"].getData<int>();

    pointProgram->setUniform("size", pointSize);

    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto verts = mesh->getProperty<std::shared_ptr<VertexList>>("P");
    glDrawArrays(GL_POINTS, 0, verts->size());
    pointProgram->release();
}

void Render::drawEdges(const glm::mat4 &view, const glm::mat4 &projection)    
{
    if(!edgeProgram) return;
    edgeProgram->bind();
    edgeProgram->setUniform("modelView", view);
    edgeProgram->setUniform("projection", projection);

    glm::vec4 color = glm::vec4(1, 1, 1, 1);
    auto props = obj->getPropertyMap();
    if(props.find("display.edgeColor") != props.end())
        color = props["display.edgeColor"].getData<glm::vec4>();

    edgeProgram->setUniform("color", color);
    
    glMultiDrawElements(GL_LINE_LOOP, //Primitive type
                        (const GLsizei*)&polysizes[0], //polygon sizes
                        GL_UNSIGNED_INT, //index datatype
                        (const GLvoid**)&polyindices[0],
                        polysizes.size()); //primitive count

    edgeProgram->release();
}

void Render::drawVertexNormals(const glm::mat4 &view, const glm::mat4 &projection)
{
    MindTree::GL::ShaderProgram prog;
    std::string vertexShader(
                             "in vec3 vertex;\
                             uniform mat4 mvMat;\
                             uniform mat4 projection;\
                            void main(){\
                                gl_Position = projection * mvMat * vec4(vertex, 1);\
                                };\
                            ");

    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto normals = mesh->getProperty<std::shared_ptr<VertexList>>("N");
    auto verts = mesh->getProperty<std::shared_ptr<VertexList>>("P");
    std::vector<glm::vec3> lines;

    for(size_t i=0; i<verts->size(); i++){
        lines.push_back(verts->at(i));
        lines.push_back(verts->at(i) + normals->at(i));
    }

    glm::vec4 color(1, 1, 0, 1);
    int linewidth = 1;

    auto props = obj->getPropertyMap();
    if(props.find("display.vertexNormalWidth") != props.end())
        linewidth = props["display.vertexNormalWidth"].getData<int>();
    if(props.find("display.vertexNormalColor") != props.end())
        color = props["display.vertexNormalColor"].getData<glm::vec4>();
    
    prog.addShaderFromSource(vertexShader, GL_VERTEX_SHADER);
    prog.link();
    prog.bind();
    prog.vertexAttribute("vertex", lines);

    prog.enableAttribute("vertex");
    prog.setUniform("mvMat", view);
    prog.setUniform("projection", projection);
    prog.setUniform("color", color);

    glLineWidth(linewidth);
    glDrawArrays(GL_LINES, 0, lines.size());
    prog.disableAttribute("vertex");
    prog.release();
    glLineWidth(1);
}

void Render::drawFaceNormals(const glm::mat4 &view, const glm::mat4 &projection)
{

}

void Render::drawPolygons(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config)    
{
    if(!polyProgram) return;
    polyProgram->bind();
    polyProgram->setUniform("modelView", view * obj->getWorldTransformation());
    polyProgram->setUniform("projection", projection);

    glm::vec4 color = glm::vec4(1, 1, 1, 1);
    auto props = obj->getPropertyMap();
    if(props.find("display.polyColor") != props.end())
        color = props["display.polyColor"].getData<glm::vec4>();

    polyProgram->setUniform("color", color);
    polyProgram->setUniform("flatShading", (int)config.flatShading());

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

    prog->addShaderFromFile("../plugins/render/defaultShaders/points.vert", 
                            GL_VERTEX_SHADER);

    prog->addShaderFromFile("../plugins/render/defaultShaders/points.frag", 
                            GL_FRAGMENT_SHADER);

    prog->link();
    prog->release();
    return prog;
}

ShaderProgram* Render::defaultEdgeProgram()    
{
    auto *prog = new ShaderProgram;
    prog->bind();

    prog->addShaderFromFile("../plugins/render/defaultShaders/edges.vert", 
                            GL_VERTEX_SHADER);

    prog->addShaderFromFile("../plugins/render/defaultShaders/edges.frag", 
                            GL_FRAGMENT_SHADER);

    prog->link();
    prog->release();
    return prog;
}

ShaderProgram* Render::defaultPolyProgram()    
{
    auto *prog = new ShaderProgram;
    prog->bind();

    prog->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert", 
                            GL_VERTEX_SHADER);

    prog->addShaderFromFile("../plugins/render/defaultShaders/polygons.frag", 
                            GL_FRAGMENT_SHADER);

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

void RenderConfig::setShowFlatShaded(bool b)
{
    _flatShading = b;
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

bool RenderConfig::flatShading() const
{
    return _flatShading;
}

RenderManager::RenderManager()
    : backgroundColor(glm::vec4(.2, .2, .2, 1.))
{
}

RenderManager::~RenderManager()
{
}

void RenderManager::init()
{
    glewInit();
    glClearColor(backgroundColor.r, 
                 backgroundColor.g, 
                 backgroundColor.b, 
                 backgroundColor.a);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderManager::setConfig(RenderConfig cfg)    
{
    config = cfg;
}

RenderConfig RenderManager::getConfig()    
{
    return config;
}

RenderPass* RenderManager::addPass()
{
    auto pass = new RenderPass();
    passes.push_back(std::unique_ptr<RenderPass>(pass));
    return pass;
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
