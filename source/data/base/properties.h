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

#ifndef PROPERTIES_K7LMQN2D

#define PROPERTIES_K7LMQN2D

#include "QColor"

typedef struct Vector
{
    Vector() : x(0), y(0), z(0) {}
    Vector(double x, double y, double z) : x(x), y(y), z(z) {}
    Vector(const Vector &vec) : x(vec.x), y(vec.y), z(vec.z) {}
    double x;
    double y;
    double z;
}   Vector;

class PropWidget;
class PropertiesEditor;
class DinSocket;
class Property
{
public:
    Property(DinSocket *socket);
    virtual ~Property();
    virtual QWidget *getEditor();
    void setEditor(QWidget* edit);
    virtual Property* derived();
    void getValue()const;
    void installTreeItem(PropertiesEditor *edit);
    DinSocket* getSocket();

private:
    QWidget *prop_editor;
    DinSocket *socket;
};

class IntProperty : public Property
{
public:
    IntProperty(DinSocket *socket);
    void setValue(int value);
    int getValue()const;
    QWidget* getEditor();
    virtual IntProperty* derived();

private:
    int prop;
};

class FloatProperty : public Property
{
public:
    FloatProperty(DinSocket *socket);
    void setValue(double value);
    double getValue()const;
    virtual FloatProperty* derived();
    QWidget* getEditor();

private:
    double prop;
};

class VectorProperty : public Property
{
public:
    VectorProperty(DinSocket *socket);
    void setValue(Vector value);
    Vector getValue()const;
    virtual VectorProperty* derived();
    QWidget* getEditor();

private:
    Vector prop;
};

class BoolProperty : public Property
{
public:
    BoolProperty(DinSocket *socket);
    void setValue(bool value);
    bool getValue()const;
    virtual BoolProperty* derived();
    QWidget* getEditor();

private:
    bool prop;
};

class ColorProperty : public Property
{
public:
    ColorProperty(DinSocket *socket);
    void setValue(QColor value);
    QColor getValue()const;
    virtual ColorProperty* derived();
    QWidget* getEditor();

private:
    QColor prop;
};

class StringProperty : public Property
{
public:
    enum pathType {
        NOPATH,
        FILEPATH,
        DIRPATH
    };

    StringProperty(DinSocket *socket);
    void setValue(QString value);
    QString getValue()const;
    virtual StringProperty* derived();
    QWidget* getEditor();
    void setPath(pathType p);
    int getPath();

private:
    pathType path;
    QString prop;
};
#endif /* end of include guard: PROPERTIES_K7LMQN2D */
