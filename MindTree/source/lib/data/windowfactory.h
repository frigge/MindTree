/*
    frg_Shader_Author Shader Editor, a Node-based Renderman Shading Language Editor
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

#ifndef WINDOWFACTORY_6K40MCK4

#define WINDOWFACTORY_6K40MCK4

#include "QObject"
#include "QAction"
#include "functional"

namespace MindTree
{
    
class Viewer;
class DoutSocket;
class ViewerDockBase;
class WindowFactory : public QObject
{
Q_OBJECT
public:
    WindowFactory(QString name, std::function<QWidget*()> func);
    WindowFactory(QString name);
    virtual ~WindowFactory();
    QString getName();
    QAction* action();
    void setWindowFunc(std::function<QWidget*()> fn);
    virtual MindTree::ViewerDockBase* createWindow();

public slots:
    virtual QString showWindow();

private:
    QString name;
    QAction *m_action;
    std::function<QWidget*()> windowFunc;
};

class DoutSocket;
class ViewerFactory
{
public:
    ViewerFactory(QString name, QString type, std::function<Viewer*(DoutSocket*)> func);
    ViewerFactory(QString name, QString type);
    virtual ~ViewerFactory();
    QString getName();
    QString getType();
    QAction* action();
    void setWindowFunc(std::function<Viewer*(DoutSocket*)> fn);
    virtual ViewerDockBase* createViewer(DoutSocket *socket);

private:
    QString name, type;
    std::function<Viewer*(DoutSocket*)> windowFunc;
};

} /* MindTree */

#endif /* end of include guard: WINDOWFACTORY_6K40MCK4 */
