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

#include "viewport.h"

#include "QDockWidget"
#include "QGLWidget"
#include "QMenu"
#include "QGraphicsSceneContextMenuEvent"
#include "QMouseEvent"

#include "math.h"

#include "source/data/base/frg.h"
#include "source/data/base/frg_shader_author.h"

Viewport::Viewport(QWidget *parent)
    : QGLWidget(parent), camlookat(QVector3D(0, 0, 0)), rotate(false), zoom(false), pan(false)
{
    transform.translate(QVector3D(0, 50, -50));
}

void Viewport::initializeGL()    
{
    glClearColor(.2f, .2f, .2f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Viewport::paintGL()    
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    glLoadIdentity();
    QVector3D campos;
    campos = transform.map(campos);
    drawAxisGizmo();
    gluLookAt((GLfloat)campos.x(), (GLfloat)campos.y(), (GLfloat)campos.z(), (GLfloat)camlookat.x(), (GLfloat)camlookat.y(), (GLfloat)camlookat.z(), 0.0f, 1.0f, 0.0f);

    drawGrid();
}

void Viewport::drawAxisGizmo()    
{
    //x-Axis
    glBegin(GL_LINES);
    glColor3f(1.f, .0f, .0f);
    glVertex3f(2.f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glEnd();

    //y-Axis
    glBegin(GL_LINES);
    glColor3f(0.f, 1.0f, .0f);
    glVertex3f(1.0f, 2.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glEnd();

   //z-Axis
    glBegin(GL_LINES);
    glColor3f(0.f, .0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 2.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glEnd();
}

void Viewport::drawGrid()    
{
    GLfloat w, h;
    int xsegments = 10;
    int zsegments = 10;
    w = 10.0f;
    h = 10.0f;

    for(int x=0; x <= xsegments; x++){
        glBegin(GL_LINES);
        glColor3f(0.8f, 0.8f, 0.8f);
        glVertex3f(-10.0f + (20.0f * x/xsegments), 0.0f, -10.0f);
        glVertex3f(-10.0f + (20.0f * x/xsegments), 0.0f, 10.0f);        
        glEnd();
    }

    for(int z=0; z <= zsegments; z++){
        glBegin(GL_LINES);
        glColor3f(0.8f, 0.8f, 0.8f);
        glVertex3f(-10.0f, 0.0f, -10.0f + (20.0f * z/zsegments));
        glVertex3f(10.0f, 0.0f, -10.0f + (20.0f * z/zsegments));        
        glEnd();
    }
}

void Viewport::mousePressEvent(QMouseEvent *event)    
{
    lastpos = event->posF();
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
    lastpos = QPointF();
}

void Viewport::mouseMoveEvent(QMouseEvent *event)    
{
    qreal xdist = lastpos.x()  - event->posF().x();
    qreal ydist = event->posF().y() - lastpos.y();
    if(rotate)
        rotateView(xdist, ydist);
    else if(pan)
        panView(xdist, ydist);
    else if(zoom)
        zoomView(xdist, ydist);
    updateGL();
    lastpos = event->posF();
}

void Viewport::rotateView(qreal xdist, qreal ydist)
{
    QVector3D campos;
    QMatrix4x4 mat;
    campos = transform.map(QVector3D());
    campos = QVector3D(campos - camlookat);

    mat.rotate(ydist, QVector3D::crossProduct(QVector3D(0, 1, 0), QVector3D(camlookat - transform.map(QVector3D()))));
    mat.rotate(xdist, QVector3D(0, 1, 0));
    campos = mat.map(campos);
    transform = QMatrix4x4();
    transform.translate(campos + camlookat);
}

void Viewport::panView(qreal xdist, qreal ydist)    
{
    QVector3D campos, pxvec, pyvec, lookatvec;
    campos = transform.map(QVector3D());
    lookatvec = camlookat - campos;
    pxvec = QVector3D::crossProduct(QVector3D(0, 1, 0), lookatvec);
    pyvec = QVector3D::crossProduct(pxvec, lookatvec);
    pxvec.normalize();
    pyvec.normalize();
    transform.translate(pxvec * xdist * -1/10);
    transform.translate(pyvec * ydist * -1/10);
    camlookat += pxvec * xdist * -1 /10;
    camlookat += pyvec * ydist * -1 /10;
}

void Viewport::zoomView(qreal xdist, qreal ydist)    
{
    QVector3D campos, zvec;
    campos = transform.map(QVector3D());
    zvec = camlookat - campos;
    zvec.normalize();
    transform.translate(zvec * ydist);
}

void Viewport::resizeGL(int width, int height)    
{
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20);    
    gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

ViewportDock::ViewportDock(ViewportNode *node)
    : QDockWidget("viewport"), node(node)
{
    viewport = new Viewport(this);
    setWidget(viewport);
    FRG::Author->addDockWidget(Qt::RightDockWidgetArea, this);
}

Viewport *ViewportDock::getViewport()    
{
    return viewport; 
}

VViewportNode::VViewportNode(DNode *node)
    : VNode(node)
{
    contextMenu();
}

void VViewportNode::contextMenu()    
{
    cxtM = new QMenu();
    cxtM->addAction(data->getDerived<ViewportNode>()->getDock()->toggleViewAction());
}

void VViewportNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    cxtM->exec(event->screenPos());
}

ViewportNode::ViewportNode(bool raw)
    : DNode("viewport"), dock(new ViewportDock(this))
{
    if(!raw){
        new DinSocket("Data", OBJECT, this);
    }
}

ViewportDock* ViewportNode::getDock()    
{
    return dock;
}

VNode* ViewportNode::createNodeVis()    
{
    setNodeVis(new VViewportNode(this));
    return getNodeVis();
}
