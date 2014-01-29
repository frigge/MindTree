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

#include "QtOpenGL"
#include "GL/glut.h"

#include "cmath"
#include "data/datatypes.h"
#include "data/properties.h"

#include "prop_edit_widget.h"

#define PI 3.141592654
using namespace MindTree;

PropEditHandle::PropEditHandle(PropEditHandle::eDirection dir)
    : vlist(0), direction(dir), scale(1)
{
}

PropEditHandle::~PropEditHandle()
{
    delete [] vlist;
}

void PropEditHandle::draw(QMatrix4x4 mvMat, QMatrix4x4 pMat)
{
    QGLShaderProgram prog;
    QString vertsrc("#version 330\n"
                    "in vec3 vertex;\n"
                    "uniform float window_width;\n"
                    "uniform float window_height;\n"
                    "uniform mat4 mvMat;\n"
                    "uniform mat4 pMat;\n"
                    "in vec4 color;\n"
                    "out vec4 vertColor;\n"

                    "void main(){\n"
                        "vec4 endMVvec = mvMat * vec4(1, 0, 0, 0);\n"
                        "vec4 projOrig = pMat *mvMat * vec4(0, 0, 0, 1);\n"
                        "vec4 projEndVec = pMat * mvMat * endMVvec;\n"
                        "vec3 nProjOrig = vec3(projOrig.x/window_width, projOrig.y/window_height, projOrig.z);\n"
                        "vec3 nProjEndVec = vec3(projEndVec.x/window_width, projEndVec.y/window_height, projEndVec.z);\n"
                        "vec4 pos = pMat * mvMat * vertex;\n"
                        "pos /= length(nProjEndVec - nProjOrig)*20;\n"
                        "gl_Position = pos;\n"
                        "vertColor = color;\n"
                    "}\n");
    
    QString fragsrc("#version 330\n"
                    "in vec4 vertColor;\n"
                    "out vec4 color;\n"
                    "void main(){\n"
                        "color = vertColor;\n"
                        "\n"
                    "\n");

    prog.addShaderFromSourceCode(QGLShader::Vertex, vertsrc);
    prog.addShaderFromSourceCode(QGLShader::Fragment, fragsrc);
    prog.bind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glLineWidth(2.5f);
    GLfloat verts[60];
    switch(direction)
    {
        case X:
    //        glBegin(GL_LINES);
    //        glVertex3f(0, 0, 0);
    //        glVertex3f(scale, 0, 0);
    //        glEnd();
    //        glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(scale, 0, 0);
            verts[0] = 0;
            verts[1] = 0;
            verts[2] = 0;
            verts[3] = 1;
            verts[4] = 0;
            verts[5] = 0;
            for(int i=6; i<33; i+=3){
                verts[i] = .75;
                verts[i+1] =  cos((i/4.0f)*PI);
                verts[i+2] = sin((i/4.0f)*PI);
            }
            for(int i=59; i>=33; i-=3){
                verts[i] = .75;
                verts[i+1] = cos((i/4.0f)*PI);
                verts[i+2] = sin((i/4.0f)*PI);
            }
    //        glVertex3f(scale - scale/4, sf, 0);
    //        glEnd();
    //        glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(scale - scale/4, 0, 0);
    //        glVertex3f(scale - scale/4, cos((8/4.0f)*PI)*sf, sin((8/4.0f)*PI)*sf);
    //        glEnd();
            break;
        case Y:
    //        glBegin(GL_LINES);
    //        glVertex3f(0, 0, 0);
    //        glVertex3f(0, scale, 0);
    //        glEnd();
    //        glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(0, scale, 0);
            verts[0] = 0;
            verts[1] = 0;
            verts[2] = 0;
            verts[3] = 1;
            verts[4] = 0;
            verts[5] = 0;
            for(int i=6; i<33; i+=3){
                verts[i] = sin((i/4.0f)*PI);
                verts[i+1] = .75;
                verts[i+2] =  cos((i/4.0f)*PI);
            }
            for(int i=59; i>=33; i-=3){
                verts[i] = sin((i/4.0f)*PI);
                verts[i+1] = .75;
                verts[i+2] = cos((i/4.0f)*PI);
            }
    //        for(int i=0; i<8; i++)
    //            glVertex3f(sin((i/4.0f)*PI)*sf, scale - scale/4, cos((i/4.0f)*PI)*sf);
    //        glVertex3f(0, scale - scale/4, sf);
    //        glEnd();
    //        glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(0, scale - scale/4, 0);
    //        for(int i=7; i>=0; i--)
    //            glVertex3f(sin((i/4.0f)*PI)*sf, scale - scale/4, cos((i/4.0f)*PI)*sf);
    //        glVertex3f(sin((8/4.0f)*PI)*sf, scale - scale/4, cos((8/4.0f)*PI)*sf);
    //        glEnd();
            break;
        case Z:
    //        glBegin(GL_LINES);
    //        glVertex3f(0, 0, 0);
    //        glVertex3f(0, 0, scale);
    //        glEnd();
    //        glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(0, 0, scale);
            verts[0] = 0;
            verts[1] = 0;
            verts[2] = 0;
            verts[3] = 1;
            verts[4] = 0;
            verts[5] = 0;
            for(int i=6; i<33; i+=3){
                verts[i] =  cos((i/4.0f)*PI);
                verts[i+1] = sin((i/4.0f)*PI);
                verts[i+2] = .75;
            }
            for(int i=59; i>=33; i-=3){
                verts[i] = cos((i/4.0f)*PI);
                verts[i+1] = sin((i/4.0f)*PI);
                verts[i+2] = .75;
            }
    //        for(int i=0; i<8; i++)
    //            glVertex3f(cos((i/4.0f)*PI)*sf, sin((i/4.0f)*PI)*sf, scale - scale/4);
    //        glVertex3f(cos((1/4.0f)*PI)*sf, sin((1/4.0f)*PI)*sf, scale - scale/4);
    //        glEnd();
    //        glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(0, 0, scale - scale/4);
    //        for(int i=7; i>=0; i--)
    //            glVertex3f(cos((i/4.0f)*PI)*sf, sin((i/4.0f)*PI)*sf, scale - scale/4);
    //        glVertex3f(cos((1/4.0f)*PI)*sf, sin((1/4.0f)*PI)*sf, scale - scale/4);
    //        glEnd();
            break;
        default:
            break;
    }
    prog.setAttributeArray("vertex", verts, 3);
    prog.enableAttributeArray("vertex");
    prog.setUniformValue("mvMat", mvMat);
    prog.setUniformValue("pMat", pMat);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_TRIANGLE_FAN, 2, 9);
    glDrawArrays(GL_TRIANGLE_FAN, 11, 9);
    prog.disableAttributeArray("vertex");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glLineWidth(1.0f);
}

void PropEditHandle::setScale(float s)    
{
    scale = s;
}

Vec3d* PropEditHandle::getVertices()    
{
    return vlist; 
}

TranslateHandle::TranslateHandle(Property *prop)
    : xHandle(new PropEditHandle(PropEditHandle::X)),
    yHandle(new PropEditHandle(PropEditHandle::Y)),
    zHandle(new PropEditHandle(PropEditHandle::Z)),
    prop(prop)
{
}

void TranslateHandle::draw(QMatrix4x4 mvMat, QMatrix4x4 pMat)
{
    //create a vector with length 1 perpendicular to the camera and then check its size in normalized image space
    //then use this size to scale the handle so  that it has constant size independend
    //of distance and FOV
    //double worig[3];

    //double origin[3] = {0, 0, 0};
    //QMatrix4x4 modelMat((qreal*)mvmat);
    //QVector3D endMVvec = modelMat.mapVector(QVector3D(1, 0, 0));

    //double wvec[3];
    //gluProject(origin[0], origin[1], origin[2], mvmat, pmat, view, &worig[0], &worig[1], &worig[2]);
    //gluProject(endMVvec.x(), endMVvec.y(), endMVvec.z(), mvmat, pmat, view, &wvec[0], &wvec[1], &wvec[2]);

    //QVector3D vecWOrig(worig[0]/view[2], worig[1]/view[3], worig[2]);
    //QVector3D endMVvecW(wvec[0]/view[2], wvec[1]/view[3], wvec[2]);

    //double transform_scale_factor = QVector3D(endMVvecW - vecWOrig).length();
    //transform_scale_factor *= 20;
    //glScalef(1.0f/transform_scale_factor, 1.0f/transform_scale_factor, 1.0f/transform_scale_factor);
    xHandle->draw(mvMat, pMat);
    yHandle->draw(mvMat, pMat);
    zHandle->draw(mvMat, pMat);
}

void TranslateHandle::setScale(float s)    
{
    xHandle->setScale(s);
    yHandle->setScale(s);
    zHandle->setScale(s);
}

void TranslateHandle::updateHandle(PropEditHandle::eDirection dir, float value)    
{
     Vec3d oldValue = prop->getData<Vec3d>();
     switch(dir)
     {
         case PropEditHandle::X:
            prop->setData(Vec3d( oldValue.x + value, oldValue.y, oldValue.z ));
            break;
         case PropEditHandle::Y:
            prop->setData(Vec3d( oldValue.x, oldValue.y + value, oldValue.z ));
            break;
         case PropEditHandle::Z:
            prop->setData(Vec3d( oldValue.x, oldValue.y, oldValue.z + value ));
            break;
     }
}

TranslateHandle::~TranslateHandle()
{
    delete xHandle; 
    delete yHandle; 
    delete zHandle; 
}

PropEditHandle* TranslateHandle::getXHandle()    
{
    return xHandle; 
}

PropEditHandle* TranslateHandle::getYHandle()    
{
    return yHandle; 
}

PropEditHandle* TranslateHandle::getZHandle()    
{
    return zHandle; 
}
