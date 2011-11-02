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


#ifndef NODELIB_H
#define NODELIB_H

#include <QTreeWidget>
#include "QGraphicsScene"
#include "QWidget"
#include "QLineEdit"
#include "QVBoxLayout"
#include "QDir"
#include "QMenu"

class NodeLib;

class NodeLibWidget : public QWidget
{
    Q_OBJECT
public:
    NodeLibWidget(QWidget *parent);

public slots:
    void filter();

private:
    QVBoxLayout *lay;
    QLineEdit *filteredit;
};

class NodeLib : public QTreeWidget
{
    Q_OBJECT
public:
    NodeLib(QWidget *parent = 0);
    ~NodeLib();
    void filter(QString txt);

signals:

public slots:
    void remove();
    void addFolder();
    void renamed(QTreeWidgetItem *item);
    void update();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QList<QTreeWidgetItem *> addNodeItems(QDir dir);
    void createContextMenu();

    void addBuildInNodes();
    QPoint dragstartpos;
};

#endif // NODELIB_H
