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
#include "QMutexLocker"

#include "math.h"

#include "source/data/base/frg.h"
#include "source/data/base/frg_shader_author.h"
#include "source/data/scene/cache_data.h"

Viewport::Viewport(QWidget *parent, ViewportNode *node)
    : QGLWidget(parent), camlookat(QVector3D(0, 0, 0)), rotate(false), zoom(false), pan(false), cache(0), viewnode(node)
{
    transform.translate(QVector3D(0, 50, -50));

    connect(&timer, SIGNAL(timeout()), this, SLOT(repaint()));
    timer.start(1);
}

void Viewport::repaint()
{
    updateGL();
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

    drawScene();
    glFlush();
}

void Viewport::drawScene()    
{
    QMutex mutex;
    if(!cache)
        return;

    QList<Object*> objects;
    if(mutex.tryLock())
        objects = cache->getData();

    if(!objects.isEmpty())
        foreach(Object *obj, objects)
            drawObject(obj);
}

void Viewport::drawObject(Object* obj)    
{
    //draw Vertices
    Vector* vertices = obj->getVertices();
    int vertcnt = obj->getVertCnt();
    
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    int i = 0;
    for(i=0; i<vertcnt; i++)
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    glEnd();

    //draw Edges
    Polygon* polygons = obj->getPolygons();
    int polycnt = obj->getPolyCnt();
    glBegin(GL_LINES);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    int j=0, vi=0;
    Vector *vert=0;
    for(i=0; i<polycnt; i++)
        for(j=0; j<polygons[i].vertexcount; j++){
            vi = polygons[i].vertices[j];
            if(vi >= vertcnt) continue;
            vert = &vertices[vi];
            glVertex3f(vert->x, vert->y, vert->z);
            if(j > 0){
                glVertex3f(vert->x, vert->y, vert->z);
            }
            if(j == polygons[i].vertexcount -1){
                vi = polygons[i].vertices[0];
                if(vi >= vertcnt) continue;
                vert = &vertices[vi];
                glVertex3f(vert->x, vert->y, vert->z);
            }
        }
    glEnd();

    //draw Polygons 
    for(i=0; i<polycnt; i++) {
        glBegin(GL_POLYGON);
        glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
        for(j=0; j<polygons[i].vertexcount; j++){
            vi = polygons[i].vertices[j];
            if(vi >= vertcnt) continue;
            vert = &vertices[vi];
            glVertex3f(vert->x, vert->y, vert->z);
        }
        glEnd();
    }
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

void Viewport::render()    
{
    viewnode->render(&cache);    
    //cache = viewnode->render();    
    updateGL();
}

void Viewport::render(DNode *node)    
{
    viewnode->render(&cache, node); 
    //cache = viewnode->render(node); 
    updateGL();
}

ViewportDock::ViewportDock(ViewportNode *node)
    : QDockWidget("viewport"), node(node)
{
    viewport = new Viewport(this, node);
    setWidget(viewport);
    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(adjust(bool)));
    FRG::Author->addDockWidget(Qt::RightDockWidgetArea, this);
}

void ViewportDock::adjust(bool vis)    
{
    if(vis)
    {
        connect((QObject*)FRG::Space, SIGNAL(linkChanged()), (QObject*)viewport, SLOT(render()));
        connect((QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)), (QObject*)viewport, SLOT(render(DNode*)));
    }
    else
    {
        disconnect((QObject*)FRG::Space, SIGNAL(linkChanged()), (QObject*)viewport, SLOT(render()));
        disconnect((QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)), (QObject*)viewport, SLOT(render(DNode*)));
    }
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
    : DNode("viewport"), dock(new ViewportDock(this)), thread(new CacheThread())
{
    thread->setStart(this);
    if(!raw){
        new DinSocket("Data", SCENEOBJECT, this);
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

void ViewportNode::render(SceneCache **cache, DNode *node)    
{
    thread->setNode(node);
    thread->setCache(cache);
    thread->setViewport(view);
    thread->start();
}

CacheThread::CacheThread()
{
}

void CacheThread::run()    
{
    if(node && !view->getAllInNodes().contains(node))
        return;

    DoutSocket *cntdSocket = view->getInSockets().first()->getCntdWorkSocket();
    if(cntdSocket)
    {
        QMutex mutex;
        SceneCache *sc = new SceneCache(cntdSocket);
        QMutexLocker lock(&mutex);
        *cache = sc;
    }
    else
        return;
}

void CacheThread::setNode(DNode *node)    
{
    this->node = node;
}

void CacheThread::setCache(SceneCache **cache)    
{
    this->cache = cache; 
}

void CacheThread::setStart(ViewportNode *viewnode)    
{
    view = viewnode;
}

void CacheThread::setViewport(Viewport *view)    
{
   viewport = view; 
}
