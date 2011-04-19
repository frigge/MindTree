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


#ifndef FRG_SHADER_AUTHOR_H
#define FRG_SHADER_AUTHOR_H

#include <QtGui/QMainWindow>
#include "QGraphicsScene"
#include "shader_view.h"
#include "QAction"
#include "QApplication"

class ChangeSpaceAction : public QAction
{
    Q_OBJECT

public:
    ChangeSpaceAction(QGraphicsScene *space, QObject *parent);

signals:
    void triggered(QGraphicsScene *space, bool checked = false);

private slots:
    void emitTriggered();

private:
    QGraphicsScene *new_space;

};

class frg_Shader_Author : public QMainWindow
{
    Q_OBJECT

public:
    frg_Shader_Author(QWidget *parent = 0);
    ~frg_Shader_Author();
    void updateToolBar();
    void moveIntoSpace(QGraphicsScene *space);

    static QApplication *app;
    static void start_app();

public slots:
    void setShader_Space(QGraphicsScene *space);
    void setRoot_Space();

    void newfile();
    void save();
    void saveAs();
    void open();

protected:
    void createMenus();
    QList<QGraphicsScene*>spaces;
private:
    QToolBar *toolbar;
    Shader_View *view;
    QGraphicsScene *root_scene;
    QString filename;
    void change_window_title(QString);
};

#endif // FRG_SHADER_AUTHOR_H
