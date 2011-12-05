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

#include "iostream"
#include "ctime"

#include "math.h"

#include "source/data/base/frg.h"
#include "source/data/base/frg_shader_author.h"
#include "source/data/scene/cache_data.h"

Viewport::Viewport(QWidget *parent, ViewportNode *node)
    : QGLWidget(parent), camlookat(QVector3D(0, 0, 0)), rotate(false), zoom(false), pan(false), cache(0), viewnode(node),
    zoomlvl(10)
{
    camData.edges = true;
    camData.points = true;
    camData.polys = true;
    camData.perspective = true;
    camData.fov = 45;
    camData.nearclip = .1;
    camData.farclip = 1000;
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

void Viewport::resizeGL(int width, int height)    
{
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLdouble aspect = (GLdouble)width/(GLdouble)height;
    gluPerspective(camData.fov, aspect, camData.nearclip, camData.farclip);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Viewport::paintGL()    
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    glEnable(GL_POINT_SMOOTH);
    glLoadIdentity();
    QVector3D campos;
    campos = transform.map(campos);
    drawAxisGizmo();
    gluLookAt((GLfloat)campos.x(), (GLfloat)campos.y(), (GLfloat)campos.z(), (GLfloat)camlookat.x(), (GLfloat)camlookat.y(), (GLfloat)camlookat.z(), 0.0f, 1.0f, 0.0f);

    drawGrid();

    drawScene();
}

void Viewport::drawScene()    
{
    if(!cache)
        return;

    QList<Object*> objects;
    objects = cache->getData();

    if(!objects.isEmpty())
        foreach(Object *obj, objects)
            drawObject(obj);
}

void Viewport::drawObject(Object* obj)    
{
    Vector* vertices = obj->getVertices();
    int vertcnt = obj->getVertCnt();
    int i = 0;
    int j=0, vi=0;
    //draw Vertices
    if (camData.points) {
        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        glPointSize(5.0);
        glBegin(GL_POINTS);
        for(i=0; i<vertcnt; i++)
            glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
        glEnd();
    }

    Polygon* polygons = obj->getPolygons();
    int polycnt = obj->getPolyCnt();
    Vector *vert=0;
    //draw Edges
    if (camData.edges) {
        glBegin(GL_LINES);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        for(i=0; i<polycnt; i++)
            for(j=0; j<polygons[i].vertexcount; j++){
                if(polygons[i].vertices) //prevent crash TODO: fix properly
                    vi = polygons[i].vertices[0];
                else
                    continue;
                vi = polygons[i].vertices[j];
                if(0 > vi || vi >= vertcnt) continue;
                vert = &vertices[vi];
                glVertex3f(vert->x, vert->y, vert->z);
                if(j > 0){
                    glVertex3f(vert->x, vert->y, vert->z);
                }
                if(j == polygons[i].vertexcount -1){
                    if(polygons[i].vertices) //prevent crash TODO: fix properly
                        vi = polygons[i].vertices[0];
                    else
                        continue;
                    if(0 > vi || vi >= vertcnt) continue;
                    vert = &vertices[vi];
                    glVertex3f(vert->x, vert->y, vert->z);
                }
            }
        glEnd();
    }

    //draw Polygons 
    if(camData.polys){
        for(i=0; i<polycnt; i++) {
            glBegin(GL_POLYGON);
            glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
            for(j=0; j<polygons[i].vertexcount; j++){
                if(polygons[i].vertices) //prevent crash TODO: fix properly
                    vi = polygons[i].vertices[0];
                else
                    continue;
                vi = polygons[i].vertices[j];
                if(0 > vi || vi >= vertcnt) continue;
                vert = &vertices[vi];
                glVertex3f(vert->x, vert->y, vert->z);
            }
            glEnd();
        }
    }
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
    zoomlvl +=ydist;
}

void Viewport::render()    
{
    if(cache) {
        delete cache;
        cache = 0;
    }
    viewnode->render(&cache, &camData, 0);
    //cache = viewnode->render();    
    resizeGL(width(), height());
    updateGL();
}

void Viewport::render(DNode *node)    
{
    if(cache) {
        delete cache;
        cache = 0;
    }
    viewnode->render(&cache, &camData, node);
    //cache = viewnode->render(node); 
    resizeGL(width(), height());
    updateGL();
}

void Viewport::clearCache()    
{
    cache->clear();
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
    setNodeType(VIEWPORTNODE);
    thread->setStart(this);
    if(!raw){
        DinSocket *fov, *nc, *fc, *pers, *poly, *vert, *edge;
        new DinSocket("Data", SCENEOBJECT, this);
        pers = new DinSocket("Perspective", CONDITION, this);
        ((BoolProperty*)pers->getProperty())->setValue(true);
        fov = new DinSocket("Field Of View", FLOAT, this);
        ((FloatProperty*)fov->getProperty())->setValue(45);
        nc = new DinSocket("Near Clipping", FLOAT, this);
        ((FloatProperty*)nc->getProperty())->setValue(0.1);
        fc = new DinSocket("Far Clipping", FLOAT, this);
        ((FloatProperty*)fc->getProperty())->setValue(1000);
        poly = new DinSocket("Show Polygons", CONDITION, this);
        ((BoolProperty*)poly->getProperty())->setValue(true);
        edge = new DinSocket("Show Edges", CONDITION, this);
        ((BoolProperty*)edge->getProperty())->setValue(true);
        vert = new DinSocket("Show Points", CONDITION, this);
        ((BoolProperty*)vert->getProperty())->setValue(true);
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

void ViewportNode::render(SceneCache **cache, CameraData *data, DNode *node)    
{
    //view->connect(thread, SIGNAL(finished()), view, SLOT(repaint()));
    thread->setData(cache, data, view, node);
    thread->start();
}

CacheThread::CacheThread()
{
}

void CacheThread::run()    
{
    if((node) 
        && (!view->getAllInNodes().contains(node)
        && node != view))
        return;

    AbstractDataCache *tmpc = 0;
    DinSocket *socket = 0;
    
    for(int i=0; i< view->getInSockets().size(); i++){
        socket = view->getInSockets().at(i);
        switch(i)
        {
            case 0:
                tmpc = new SceneCache(socket);
                *cache = (SceneCache*)tmpc;
                break;
            case 1:
                break;
            case 2:
                tmpc = new FloatCache(socket);
                camData->fov = ((FloatCache*)tmpc)->getData()[0];
                tmpc->setOwner(true);
                delete tmpc;
                break;
            case 3:
                tmpc = new FloatCache(socket);
                camData->nearclip = ((FloatCache*)tmpc)->getData()[0];
                tmpc->setOwner(true);
                delete tmpc;
                break;
            case 4:
                tmpc = new FloatCache(socket);
                camData->farclip = ((FloatCache*)tmpc)->getData()[0];
                tmpc->setOwner(true);
                delete tmpc;
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            default:
                break;
        }
    }
}

void CacheThread::setData(SceneCache **cache, CameraData *data, Viewport *view, DNode *node)    
{
    this->cache = cache;
    this->camData = data; 
    this->viewport = view;
    this->node = node;
}

void CacheThread::setStart(ViewportNode *start)    
{
    view = start;
}
