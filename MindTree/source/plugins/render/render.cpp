#define GLM_SWIZZLE

#include "GL/glew.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"

#include "rendermanager.h"
#include "render.h"

using namespace MindTree::GL;

Render::Render(std::shared_ptr<GeoObject> o)
    : obj(o), initialized(false)
{
}

Render::~Render()
{
    for(auto *indices : polyindices)
        delete [] indices;

    auto manager = QtContext::getSharedContext()->getManager();

    if(pointProgram) manager->scheduleCleanUp(std::move(pointProgram));
    if(edgeProgram) manager->scheduleCleanUp(std::move(edgeProgram));
    if(polyProgram) manager->scheduleCleanUp(std::move(polyProgram));

    if(vao) manager->scheduleCleanUp(std::move(vao));
}

void Render::init()    
{
    initialized = true;
}

void Render::draw(const CameraPtr camera, const RenderConfig &config)
{
    if(!initialized)
        init();

    auto props = obj->getProperties();
    vao->bind();
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POLYGON_OFFSET_POINT);
    if(config.drawPoints()) drawPoints(camera);
    if(config.drawEdges()) drawEdges(camera);
    if(config.drawPolygons()) drawPolygons(camera, config);

    if(props.find("display.drawVertexNormal") != props.end())
        if(props["display.drawVertexNormal"].getData<bool>())
            drawVertexNormals(camera);

    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POLYGON_OFFSET_POINT);
    glLineWidth(1);
    vao->release();
}

void Render::setUniforms(ShaderProgram *prog)
{
    auto propmap = obj->getProperties();
    for(const auto &p : propmap) {
        std::string str = p.first;
        std::string sub = str.substr(0, str.find("."));
        if(sub == "display") {
            if(p.second.getType() == "FLOAT")
                prog->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 (float)p.second.getData<double>());

            else if(p.second.getType() == "INTEGER")
                prog->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 p.second.getData<int>());

            else if(p.second.getType() == "BOOLEAN")
                prog->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 p.second.getData<bool>());

            else if(p.second.getType() == "COLOR") {
                std::string temp = str.substr(str.find(".")+1, str.length());
                prog->setUniform(temp, 
                                 p.second.getData<glm::vec4>());
            }

            else if(p.second.getType() == "VECTOR3D")
                prog->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 p.second.getData<glm::vec3>());
        }
    }
}

void Render::drawPoints(const CameraPtr camera)
{
    if(!pointProgram) return;
    glm::mat4 model = obj->getWorldTransformation();
    glm::mat4 modelView = camera->getViewMatrix() * model;
    glm::mat4 projection = camera->getProjection();
    pointProgram->bind();
    pointProgram->setUniform("modelView", modelView);
    pointProgram->setUniform("projection", projection);
    setUniforms(pointProgram.get());

    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto verts = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
    glDrawArrays(GL_POINTS, 0, verts->size());
    pointProgram->release();
}

void Render::drawEdges(const CameraPtr camera)
{
    if(!edgeProgram) return;
    glm::mat4 model = obj->getWorldTransformation();
    glm::mat4 modelView = camera->getViewMatrix() * model;
    glm::mat4 projection = camera->getProjection();
    edgeProgram->bind();
    edgeProgram->setUniform("modelView", modelView);
    edgeProgram->setUniform("projection", projection);
    setUniforms(edgeProgram.get());


    float lineWidth = 1.5;

    if (obj->hasProperty("display.lineWidth"))
        lineWidth =  obj->getProperty("display.lineWidth").getData<double>();

    glLineWidth(lineWidth);
    glMultiDrawElements(GL_LINE_LOOP, //Primitive type
                        (const GLsizei*)&polysizes[0], //polygon sizes
                        GL_UNSIGNED_INT, //index datatype
                        (const GLvoid**)&polyindices[0],
                        polysizes.size()); //primitive count


    edgeProgram->release();
}

void Render::drawVertexNormals(const CameraPtr camera)
{
    glm::mat4 model = obj->getWorldTransformation();
    glm::mat4 modelView = camera->getViewMatrix() * model;
    glm::mat4 projection = camera->getProjection();
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
    auto normals = mesh->getProperty("N").getData<std::shared_ptr<VertexList>>();
    auto verts = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
    std::vector<glm::vec3> lines;

    for(size_t i=0; i<verts->size(); i++){
        lines.push_back(verts->at(i));
        lines.push_back(verts->at(i) + normals->at(i));
    }

    glm::vec4 color(1, 1, 0, 1);
    int linewidth = 1;

    auto props = obj->getProperties();
    if(props.find("display.vertexNormalWidth") != props.end())
        linewidth = props["display.vertexNormalWidth"].getData<int>();
    if(props.find("display.vertexNormalColor") != props.end())
        color = props["display.vertexNormalColor"].getData<glm::vec4>();
    
    prog.addShaderFromSource(vertexShader, GL_VERTEX_SHADER);
    prog.link();
    prog.bind();
    prog.vertexAttribute("vertex", lines);

    prog.enableAttribute("vertex");
    prog.setUniform("mvMat", modelView);
    prog.setUniform("projection", projection);
    prog.setUniform("color", color);

    glLineWidth(linewidth);
    glDrawArrays(GL_LINES, 0, lines.size());
    prog.disableAttribute("vertex");
    prog.release();
    glLineWidth(1);
}

void Render::drawFaceNormals(const CameraPtr camera)
{

}

void Render::drawPolygons(const CameraPtr camera, const RenderConfig &config)    
{
    if(!polyProgram) return;
    glm::mat4 model = obj->getWorldTransformation();
    glm::mat4 modelView = camera->getViewMatrix() * model;
    glm::mat4 projection = camera->getProjection();
    polyProgram->bind();
    polyProgram->setUniform("modelView", modelView);
    polyProgram->setUniform("projection", projection);
    setUniforms(polyProgram.get());

    polyProgram->setUniform("flatShading", (int)config.flatShading());

    glPolygonOffset(1.0, 1.0);

    glMultiDrawElements(GL_TRIANGLE_FAN, //Primitive type
                        (const GLsizei*)&polysizes[0], //polygon sizes
                        GL_UNSIGNED_INT, //index datatype
                        (const GLvoid**)&polyindices[0],
                        polysizes.size()); //primitive count

    polyProgram->release();
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

MeshRender::MeshRender(std::shared_ptr<GeoObject> o)
    : Render(o)
{
}

MeshRender::~MeshRender()
{
}

void MeshRender::generateIndices()    
{
    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto plist = mesh->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
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
    auto &plist = *mesh->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
    auto &verts = *mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
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

void MeshRender::initPointProgram()
{
    std::string vertPropID = "display.GLSL.pointVertexShader";
    std::string fragPropID = "display.GLSL.pointFragmentShader";

    ShaderProgram *prog = new ShaderProgram;
    prog->bind();

    if(obj->hasProperty(vertPropID)) {
        std::string vertSrc = obj->getProperty(vertPropID)
            .getData<std::string>(); 
        prog->addShaderFromSource(vertSrc, GL_VERTEX_SHADER);
    }
    else {
        prog->addShaderFromFile("../plugins/render/defaultShaders/points.vert", 
                                GL_VERTEX_SHADER);
    }

    if(obj->hasProperty(fragPropID)) {

        std::string fragSrc = obj->getProperty(fragPropID)
            .getData<std::string>(); 

        prog->addShaderFromSource(fragSrc, GL_FRAGMENT_SHADER);
    }
    else {
        prog->addShaderFromFile("../plugins/render/defaultShaders/points.frag", 
                                GL_FRAGMENT_SHADER);

    }

    prog->link();
    prog->release();
    setPointProgram(prog);
}

void MeshRender::initEdgeProgram()
{
    std::string vertPropID = "display.GLSL.edgeVertexShader";
    std::string fragPropID = "display.GLSL.edgeFragmentShader";

    ShaderProgram *prog = new ShaderProgram();
    prog->bind();

    if(obj->hasProperty(vertPropID)) {
        std::string vertSrc = obj->getProperty(vertPropID)
            .getData<std::string>(); 
        prog->addShaderFromSource(vertSrc, GL_VERTEX_SHADER);
    }
    else {
        prog->addShaderFromFile("../plugins/render/defaultShaders/edges.vert", 
                                GL_VERTEX_SHADER);
    }

    if(obj->hasProperty(fragPropID)) {
        std::string fragSrc = obj->getProperty(fragPropID)
            .getData<std::string>(); 

        prog->addShaderFromSource(fragSrc, GL_FRAGMENT_SHADER);
    }
    else {
        prog->addShaderFromFile("../plugins/render/defaultShaders/edges.frag", 
                                GL_FRAGMENT_SHADER);
    }

    prog->link();
    prog->release();
    setEdgeProgram(prog);
}

void MeshRender::initPolyProgram()
{
    std::string vertPropID = "display.GLSL.polyVertexShader";
    std::string fragPropID = "display.GLSL.polyFragmentShader";

    ShaderProgram *prog = new ShaderProgram();
    prog->bind();

    if(obj->hasProperty(vertPropID)) {
        std::string vertSrc = obj->getProperty(vertPropID)
            .getData<std::string>(); 
        prog->addShaderFromSource(vertSrc, GL_VERTEX_SHADER);
    }
    else {
        prog->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert", 
                                GL_VERTEX_SHADER);
    }
    
    if(obj->hasProperty(fragPropID)) {

        std::string fragSrc = obj->getProperty(fragPropID)
            .getData<std::string>(); 
        prog->addShaderFromSource(fragSrc, GL_FRAGMENT_SHADER);
    }
    else {
        prog->addShaderFromFile("../plugins/render/defaultShaders/polygons.frag", 
                                GL_FRAGMENT_SHADER);
    }

    prog->link();
    prog->release();
    setPolyProgram(prog);
}

void MeshRender::init()    
{
    Render::init();
    vao = std::unique_ptr<VAO>(new VAO());
    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());

    initPointProgram();
    initEdgeProgram();
    initPolyProgram();

    generateIndices();
    //tesselate();

    vao->bind();

    auto propmap = mesh->getProperties();
    for(auto propPair : propmap){
        bool pointprog_has = pointProgram->hasAttribute(propPair.first);
        bool edgeprog_has = edgeProgram->hasAttribute(propPair.first);
        bool polyprog_has = polyProgram->hasAttribute(propPair.first);
        if (pointprog_has || edgeprog_has || polyprog_has) {
            //vao->addData(propPair.second.getData<std::shared_ptr<VertexList>>());
            auto vbo = Context::getSharedContext()->getManager()->getVBO(mesh, propPair.first);
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

void RenderGroup::addObject(std::shared_ptr<GeoObject> obj)    
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            auto render = new MeshRender(obj);
            for(auto child : obj->getChildren()){
                addObject(std::static_pointer_cast<GeoObject>(child));
            }
            renders.push_back(std::unique_ptr<Render>(render));
            break;
    }
}

void RenderGroup::draw(const CameraPtr camera, const RenderConfig &config)    
{
    for(auto &render : renders)
        render->draw(camera, config);
}

RenderPass::RenderPass()
    : _viewportChanged(true)
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::setSize(int width, int height)
{
    std::lock_guard<std::mutex> lock(_sizeLock);
    if(_width != width || _height != height)
        _viewportChanged = true;

    _width = width;
    _height = height;
}

void RenderPass::setCamera(CameraPtr camera)
{
    _camera = camera;
}

CameraPtr RenderPass::getCamera()
{
    return _camera;
}

void RenderPass::setGeometry(std::shared_ptr<Group> g)    
{
    std::lock_guard<std::mutex> lock(_geometryLock);
    group = std::make_shared<RenderGroup>(g);
}

void RenderPass::render(const RenderConfig &config)
{
    GLObjectBinder<std::shared_ptr<FBO>> fbobinder(target);

    if(!group) return;

    {
        std::lock_guard<std::mutex> lock(_sizeLock);
        if(!_width || !_height) {
            std::cout << "No viewport geometry" << std::endl;
            return;
        }
    }

    if(_viewportChanged) {
        glViewport(0, 0, (GLint)_width, (GLint)_height);
        _viewportChanged = false;
    }

    {
        std::lock_guard<std::mutex> lock(_geometryLock);
        group->draw(_camera, config);
    }
}

