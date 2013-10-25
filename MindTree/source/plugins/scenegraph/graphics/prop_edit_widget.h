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

#ifndef PROP_EDIT_WIDGET

#define PROP_EDIT_WIDGET

#include "QMatrix4x4"

class Vec3d;
namespace MindTree{ class Property; }
class PropEditHandle
{
public:
    enum eDirection {X, Y, Z};
    PropEditHandle(eDirection dir);
    virtual ~PropEditHandle();
    Vec3d* getVertices();
    void setScale(float s);
    void draw(QMatrix4x4 mvMat, QMatrix4x4 pMat);

private:
    float scale;
    Vec3d *vlist;
    eDirection direction;
};

class TranslateHandle
{
public:
    TranslateHandle(MindTree::Property *prop);
    virtual ~TranslateHandle();
    void updateHandle(PropEditHandle::eDirection dir, float value);
    PropEditHandle* getZHandle();
    PropEditHandle* getYHandle();
    PropEditHandle* getXHandle();
    void setScale(float s);
    void draw(QMatrix4x4 mvMat, QMatrix4x4 pMat);

private:
    PropEditHandle *xHandle, *yHandle, *zHandle;
    MindTree::Property *prop;
};

#endif /* end of include guard: PROP_EDIT_WIDGET*/
