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

#ifndef NEWNODEEDITOR_H
#define NEWNODEEDITOR_H

#include "QWidget"
#include "QTreeWidget"
#include "QComboBox"
#include "QLineEdit"
#include "QPushButton"
#include "QGridLayout"

class DNode;
class VNSpace;

class SlotTypeEditor : public QComboBox
{
    Q_OBJECT
public:
    SlotTypeEditor();
};

class RemoveButton : public QPushButton
{
    Q_OBJECT
public:
    RemoveButton(int);
    ~RemoveButton();
    void setIndex(int);

protected:
    int index;

public slots:
    void emitClicked();

signals:
    void clicked(int);
};

class SocketEditor : public QTreeWidget
{
    Q_OBJECT
public:
    SocketEditor();
    ~SocketEditor();

public slots:
    void addSocket();
    void removeSocket(int);

private:
    void refreshIndices();
};

class NewNodeEditor : public QWidget
{
    Q_OBJECT
public:
    NewNodeEditor();
    ~NewNodeEditor();
    void update();

signals:

public slots:
    void addtoLib();

private:
    void createLayout();
    SocketEditor *inputsockets, *outputsockets;
    QLineEdit *func_name;
    QLineEdit *node_name;
    QGridLayout *grid;

};

#endif // NEWNODEEDITOR_H
