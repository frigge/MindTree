/*
    FRG Shader Editor, a Node-based Renderman Shading Language Editor
    Copyright (C) 2011  Sascha Fricke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define GLM_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GL/glew.h"

#include "QMenu"
#include "QGraphicsSceneContextMenuEvent"
#include "QMouseEvent"
#include "QGLShaderProgram"
#include "QGLFramebufferObject"
#include "GL/glut.h"

#include "iostream"
#include "ctime"

#include "math.h"

#include "source/plugins/datatypes/Object/object_cache.h"
#include "graphics/viewport_widget.h"
#include "graphics/prop_edit_widget.h"
#include "graphics/object_node_vis.h"
#include "source/data/base/raytracing/ray.h"

#include "viewport.h"

using namespace MindTree;

Viewport::Viewport()
    : QGLWidget(MindTree::GL::QtContext::getContext()), 
    rotate(false), 
    zoom(false), 
    pan(false), 
    defaultCamera(std::make_shared<Camera>()),
    selectionMode(false), 
    transformMode(0), 
    selectedNode(0)
{
    activeCamera = defaultCamera;
    DNode *snode = 0;
    setAutoBufferSwap(false);

    connect(&timer, SIGNAL(timeout()), this, SLOT(repaint()));
    //timer.start(1);

    rendermanager = std::unique_ptr<MindTree::GL::RenderManager>(new MindTree::GL::RenderManager());

    auto *pass = new MindTree::GL::RenderPass();

    rendermanager->addPass(pass);
}

Viewport::~Viewport()
{
}

void Viewport::setData(std::shared_ptr<Group> value)
{
    rendermanager->getPass(0)->setGeometry(value);
}

void Viewport::changeCamera(QString cam)    
{
    CameraNode* cnode = (CameraNode*)FRG::CurrentProject->getItem(cam);
    if(cnode) {
        activeCamera = std::static_pointer_cast<Camera>(cnode->getObject());
        if(selectedNode == cnode)
            selectedNode = 0;
    }
    updateGL();
}

void Viewport::repaint()
{
    updateGL();
}

void Viewport::initializeGL()    
{
    glewInit();
    glClearColor(0.2f, .2f, .2f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Viewport::resizeGL(int width, int height)    
{
    activeCamera->setProjection((GLdouble)width/(GLdouble)height);
    glViewport(0, 0, (GLint)width, (GLint)height);
}

void Viewport::paintGL()    
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    if(selectionMode) {
        glEnable(GL_DITHER);
        glClearColor(.0f, .0f, .0f, 1.0f); 
    }
    else {
        time(&start);
        glClearColor(.2f, .2f, .2f, 1.0f); 
        glEnable(GL_POINT_SMOOTH);
    }
    if(!selectionMode) {
        drawFps();
        drawGrid();
        drawScene();
        time(&end);
        glColor3f(1, 1, 1);
        swapBuffers();
    }
    else {
        drawScene();
        glDisable(GL_DITHER);
    }
}

void Viewport::drawFps()    
{
//    glPushMatrix();
//    QPainter paint(this);
//    paint.begin(this);
//    paint.drawText(10, 10, QString::number(1.0/difftime(end, start)) + " FPS");
//    paint.end();
//    glPopMatrix();
}

void Viewport::mouseToWorld()    
{
    //GLdouble *pmat= new GLdouble[16];
    //GLdouble *mvmat= new GLdouble[16];
    //GLint *view = new GLint[4];
    //glGetIntegerv(GL_VIEWPORT, view);
    //glGetDoublev(GL_PROJECTION_MATRIX, pmat);
    //glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
    //gluUnProject(lastpos.x(), height() - lastpos.y(), 0, mvmat, pmat, view, &mwNear[0], &mwNear[1], &mwNear[2]);
    //gluUnProject(lastpos.x(), height() - lastpos.y(), 1, mvmat, pmat, view, &mwFar[0], &mwFar[1], &mwFar[2]);
}

void Viewport::drawScene()    
{
    rendermanager->draw(activeCamera->getViewMatrix(), activeCamera->getProjection());
}

void Viewport::setShowPoints(bool b)    
{
    auto config = rendermanager->getConfig();
    config.setDrawPoints(b);
    rendermanager->setConfig(config);
    updateGL();
}

void Viewport::setShowEdges(bool b)    
{
    auto config = rendermanager->getConfig();
    config.setDrawEdges(b);
    rendermanager->setConfig(config);
    updateGL();
}

void Viewport::setShowPolygons(bool b)    
{
    auto config = rendermanager->getConfig();
    config.setDrawPolygons(b);
    rendermanager->setConfig(config);
    updateGL();
}

void Viewport::drawTransformable(std::shared_ptr<AbstractTransformable> transformable)
{
    DNode *node=0;
    drawOrigin();
    //NodeList selectedNodes = FRG::Space->selectedNodes();
    //TranslateHandle *handle=0;
    //if(!selectedNodes.isEmpty()){
    //    node = selectedNodes.first();
    //    if(node != transformable->getNode()) {
    //        glPopMatrix();
    //        return;
    //    }
    //    switch(node->getNodeType()) {
    //        case CAMERANODE:
    //        case LIGHTNODE:
    //        case OBJECTNODE:
    //            //handle = ((ObjectNodeVis*)node->getNodeVis())->getTranslateHandle();
    //            break;
    //        default:
    //            break;
    //    }
    //}
    //if(handle) {
    //    handle->draw(activeCamera->getViewMatrix(), activeCamera->getProjection());
    //}
    //if(selectionMode) checkTransform();
}

void Viewport::drawOrigin()    
{
    glDisable(GL_DEPTH_TEST);
    if(selectionMode) return;
    QGLShaderProgram prog;
    QString vertsrc("#version 330\n"
                    "in vec3 vertex;\n"
                    "uniform mat4 mvMat;\n"
                    "uniform mat4 pMat;\n"
                    
                    "void main(){\n"
                        "mat4 mvpMat = pMat * mvMat;\n"
                        "gl_Position = mvpMat * vec4(vertex, 1);\n"
                    "}\n"
                    );
    QString fragsrc("#version 330\n"
                    "out vec4 color;\n"
                    
                    "void main(){\n"
                        "color = vec4(1, 0, 0, 1);\n"
                    "}\n"
                    );
    prog.addShaderFromSourceCode(QGLShader::Vertex, vertsrc);
    prog.addShaderFromSourceCode(QGLShader::Fragment, fragsrc);
    prog.bind();
    GLfloat point[3];
    point[0] = 0;
    point[1] = 0;
    point[2] = 0;
    prog.setAttributeArray("vertex", point, 3);
    prog.enableAttributeArray("vertex");
    //prog.setUniformValue("mvMat", activeCamera->getViewMatrix());
    //prog.setUniformValue("pMat", activeCamera->getProjection());
    glPointSize(4.0f);
    glDrawArrays(GL_POINTS, 0, 1);
    prog.disableAttributeArray("vertex");
    prog.release();
    glEnable(GL_DEPTH_TEST);
}

void Viewport::drawAxisGizmo()    
{
    //x-Axis
    glBegin(GL_LINES);
    glColor3f(1.f, .0f, .0f);
    glVertex3f(2.f, -1.0f, -5.0f);
    glVertex3f(1.0f, -1.0f, -5.0f);
    glEnd();

    //y-Axis
    glBegin(GL_LINES);
    glColor3f(0.f, 1.0f, .0f);
    glVertex3f(1.0f, -2.0f, -5.0f);
    glVertex3f(1.0f, -1.0f, -5.0f);
    glEnd();

   //z-Axis
    glBegin(GL_LINES);
    glColor3f(0.f, .0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -6.0f);
    glVertex3f(1.0f, -1.0f, -5.0f);
    glEnd();
}

void Viewport::drawGrid()    
{
    int xsegments = 100;
    int zsegments = 100;

    float xsize = 100;
    float zsize = 100;
    MindTree::GL::ShaderProgram prog;
    std::string vert("#version 330\n"
        "in vec3 vertex;\n"
        "uniform mat4 mvMat;\n"
        "uniform mat4 pMat;\n"
        "out vec4 vertColor;\n"

        "void main(){\n"
            "mat4 mvpMat = pMat*mvMat;\n"
            "vertColor = vec4(1., 1., 1., .3);\n"
            "gl_Position = mvpMat * vec4(vertex, 1.0);\n"
        "}\n"
    ); 

    std::string frag("#version 330\n"
                 "out vec4 color;\n"
                 "in vec4 vertColor;\n"

                 "void main(){\n"
                    "color = vertColor;\n"
                 "}\n"
                 );
    auto verts = std::vector<glm::vec3>(xsegments*2 + zsegments*2 + 2);
    for(int i=0; i<=xsegments; i++){
        verts.push_back(glm::vec3((float(i)/xsegments * xsize) - xsize/2, 0, -zsize/2));
        verts.push_back(glm::vec3((float(i)/xsegments * xsize) - xsize/2, 0, zsize/2));
    }

    for(int i=0; i<=zsegments; i++){
        verts.push_back(glm::vec3(-xsize/2, 0, (float(i)/zsegments * zsize) - zsize/2));
        verts.push_back(glm::vec3(xsize/2, 0, (float(i)/zsegments * zsize) - zsize/2));
    }

    std::cout << "compiling grid shader" << std::endl;
    prog.addShaderFromSource(vert, GL_VERTEX_SHADER);
    prog.addShaderFromSource(frag, GL_FRAGMENT_SHADER);
    prog.link();
    prog.bind();
    prog.vertexAttribute("vertex", verts);
    prog.enableAttribute("vertex");
    prog.setUniform("mvMat", activeCamera->getViewMatrix());
    prog.setUniform("pMat", activeCamera->getProjection());

    glLineWidth(1);
    glDrawArrays(GL_LINES, 0, verts.size());
    prog.disableAttribute("vertex");
    prog.release();
}

void Viewport::checkTransform()    
{
    //AbstractTransformableNode *tn = (AbstractTransformableNode*)FRG::Space->selectedNodes().first();
    //GLubyte rgb[3];
    //glReadPixels(lastpos.x(), height() - lastpos.y(), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &rgb);
    //Vector mwn = Vector(mwNear[0], mwNear[1], mwNear[2]);
    //Vector mwf = Vector(mwFar[0], mwFar[1], mwFar[2]);
    //Vector dir = mwf - mwn;
    //Ray r(mwn, dir);
    //Vector hitpoint;
    //glm::vec3 lastObjPos = tn->getObject()->getTransformation().map(glm::vec3());
    //Vector lop(lastObjPos.x(), lastObjPos.y(), lastObjPos.z());
    //if(rgb[0] == 255) {
    //    transformMode = 1;
    //    if(!r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint))
    //        r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, 1, lop.z), 0, 0, 0, &hitpoint);
    //}
    //else if(rgb[1] == 255) {
    //    transformMode = 2;
    //    if(!r.intersectPlane(lop, Vector(lop.x, 1, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint))
    //        r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, 1, lop.z), 0, 0, 0, &hitpoint);
    //}
    //else if(rgb[2] == 255) {
    //    transformMode = 3;
    //    if(!r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint))
    //        r.intersectPlane(lop, Vector(lop.x, 1, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint);
    //}
    //else{
    //    return;
    //}
    //mouseDistToObj = glm::vec3(hitpoint.x, hitpoint.y, hitpoint.z) - lastObjPos;

    //selectionMode = false;
}

void Viewport::transform(QPointF mpos)
{
//    lastpos = mpos;
//    AbstractTransformableNode *tn = (AbstractTransformableNode*)FRG::Space->selectedNodes().first();
//    glm::vec3 campos = activeCamera->getPosition();
//    Vector mwn = Vector(mwNear[0], mwNear[1], mwNear[2]);
//    Vector mwf = Vector(mwFar[0], mwFar[1], mwFar[2]);
//    Vector dir = mwf - mwn;
//    Ray r(mwn, dir);
//    Vector hitpoint;
//    glm::vec3 lastObjPos = tn->getObject()->getPosition();
//    Vector lop(lastObjPos.x(), lastObjPos.y(), lastObjPos.z());
//    glm::vec3 tv;
//    glm::vec3 lastMousePos = lastObjPos + mouseDistToObj;
//    switch(transformMode) {
//        case 1:
//            if(!r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint))
//                r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, 1, lop.z), 0, 0, 0, &hitpoint);
//            tv = glm::vec3(hitpoint.x, hitpoint.y, hitpoint.z) - lastMousePos;
//            tn->getObject()->translate(tv.x(), 0, 0);
//            break;
//        case 2:
//            if(!r.intersectPlane(lop, Vector(lop.x, 1, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint))
//                r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, 1, lop.z), 0, 0, 0, &hitpoint);
//            tv = glm::vec3(hitpoint.x, hitpoint.y, hitpoint.z) - lastMousePos;
//            tn->getObject()->translate(0, tv.y(), 0);
//            break;
//        case 3:
//            if(!r.intersectPlane(lop, Vector(1, lop.y, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint))
//                r.intersectPlane(lop, Vector(lop.x, 1, lop.z), Vector(lop.x, lop.y, 1), 0, 0, 0, &hitpoint);
//            tv = glm::vec3(hitpoint.x, hitpoint.y, hitpoint.z) - lastMousePos;
//            tn->getObject()->translate(0, 0, tv.z());
//            break;
//        default:
//            break;
//    }
//    lastObjPos = tn->getObject()->getPosition();
//    mouseDistToObj = glm::vec3(hitpoint.x, hitpoint.y, hitpoint.z) - lastObjPos;
}

void Viewport::mousePressEvent(QMouseEvent *event)    
{
    lastpos = event->posF();
    //NodeList selectedNodes = FRG::Space->selectedNodes();
    if(event->modifiers() & Qt::ControlModifier)
        zoom = true;
    else if (event->modifiers() & Qt::ShiftModifier)
        pan = true;
    else
        rotate = true;
}

void Viewport::mouseReleaseEvent(QMouseEvent *event)    
{
    rotate = false;
    pan = false;
    zoom = false;
    selectionMode =false;
    transformMode = 0;
    lastpos = QPointF();
}

void Viewport::mouseMoveEvent(QMouseEvent *event)    
{
    float xdist = lastpos.x()  - event->posF().x();
    float ydist = event->posF().y() - lastpos.y();
    if(transformMode) {
        transform(event->pos());
        return;
    }
    if(rotate)
        rotateView(xdist, ydist);
    else if(pan)
        panView(xdist, ydist);
    else if(zoom)
        zoomView(xdist, ydist);
    updateGL();
    lastpos = event->posF();
}

void Viewport::wheelEvent(QWheelEvent *event)    
{
    zoomView(0, event->delta());
    updateGL();
}

AbstractTransformableNode* Viewport::getSelectedNode()    
{
    if(selectedNode && selectedNode->getObject() == activeCamera)
        return 0;
    else return selectedNode;
}

glm::vec3 Viewport::getCamPivot()    
{
    if(getSelectedNode())
        return glm::vec4(getSelectedNode()->getObject()->getTransformation() * glm::vec4()).xyz();
    else
        return glm::vec3(0, 0, 0);
}

void Viewport::rotateView(float xdist, float ydist)
{
    if(!activeCamera)
        return;
    glm::mat4 mat;
    glm::vec3 campos = activeCamera->getPosition();
    glm::vec3 center = activeCamera->getCenter();
    glm::vec3 lookat = campos - center;

    mat = glm::rotate(mat, (float)ydist, glm::cross(glm::vec3(0, 1, 0), center - campos));
    mat = glm::rotate(mat, (float)xdist, glm::vec3(0, 1, 0));
    lookat = (mat * glm::vec4(lookat, 1)).xyz();
    activeCamera->posAroundCenter(lookat + center);
}

void Viewport::panView(float xdist, float ydist)    
{
    if(!activeCamera)
        return;
    float lalen = glm::length(activeCamera->getCenter() - activeCamera->getPosition());
    float xtrans = xdist * lalen * 0.005;
    float ytrans = ydist * lalen * 0.005;
    activeCamera->translate(glm::vec3(1, 0, 0) * xtrans);
    activeCamera->translate(glm::vec3(0, 1, 0) * ytrans);
}

void Viewport::zoomView(float xdist, float ydist)    
{
    if(!activeCamera)
        return;
    //glm::vec3 zvec = activeCamera->getCenter() - activeCamera->getPosition();
    activeCamera->moveToCenter(ydist/height());
}

void Viewport::render()    
{
    updateGL();
}

void Viewport::render(DNode *node)    
{
    updateGL();
}
