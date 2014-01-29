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

#ifndef SOCKET_PROPERTIES_ZV9JM40T

#define SOCKET_PROPERTIES_ZV9JM40T

#include "QColor"
#include "data/datatypes.h"

class PropWidget;
class PropertiesEditor;
namespace MindTree {
class DinSocket;
}
class SocketProperty
{
public:
    SocketProperty(MindTree::DinSocket *socket);
    virtual ~SocketProperty();
    virtual QWidget *getEditor();
    void setEditor(QWidget* edit);
    virtual SocketProperty* derived();
    void getValue()const;
    void installTreeItem(PropertiesEditor *edit);
    MindTree::DinSocket* getSocket();

private:
    QWidget *prop_editor;
    MindTree::DinSocket *socket;
};

class IntSocketProperty : public SocketProperty
{
public:
    IntSocketProperty(MindTree::DinSocket *socket);
    void setValue(int value);
    int getValue()const;
    QWidget* getEditor();
    virtual IntSocketProperty* derived();

private:
    int prop;
};

class FloatSocketProperty : public SocketProperty
{
public:
    FloatSocketProperty(MindTree::DinSocket *socket);
    void setValue(double value);
    double getValue()const;
    virtual FloatSocketProperty* derived();
    QWidget* getEditor();

private:
    double prop;
};

class VectorSocketProperty : public SocketProperty
{
public:
    VectorSocketProperty(MindTree::DinSocket *socket);
    void setValue(Vec3d value);
    Vector getValue()const;
    virtual VectorSocketProperty* derived();
    QWidget* getEditor();

private:
    Vector prop;
};

class BoolSocketProperty : public SocketProperty
{
public:
    BoolSocketProperty(MindTree::DinSocket *socket);
    void setValue(bool value);
    bool getValue()const;
    virtual BoolSocketProperty* derived();
    QWidget* getEditor();

private:
    bool prop;
};

class ColorSocketProperty : public SocketProperty
{
public:
    ColorSocketProperty(MindTree::DinSocket *socket);
    void setValue(QColor value);
    QColor getValue()const;
    virtual ColorSocketProperty* derived();
    QWidget* getEditor();

private:
    QColor prop;
};

class StringSocketProperty : public SocketProperty
{
public:
    enum pathType {
        NOPATH,
        FILEPATH,
        DIRPATH
    };

    StringSocketProperty(MindTree::DinSocket *socket);
    void setValue(QString value);
    QString getValue()const;
    virtual StringSocketProperty* derived();
    QWidget* getEditor();
    void setPath(pathType p);
    int getPath();

private:
    pathType path;
    QString prop;
};

#endif /* end of include guard: SOCKET_PROPERTIES_ZV9JM40T */
