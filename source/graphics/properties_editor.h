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

#ifndef PROPERTIES_EDITOR_HTC8OG2V

#define PROPERTIES_EDITOR_HTC8OG2V

#include "QTreeWidget"
#include "QGridLayout"

class DNode;

class PropertiesEditor : public QTreeWidget
{
    Q_OBJECT
public:
    PropertiesEditor (QWidget *parent);
    DNode* getFocusNode();
    void setFocusNode(DNode* value);

public slots:
    void updateFocus();

private:
    DNode *focus_node;
    QList<QHBoxLayout*> layouts;
};

#endif /* end of include guard: PROPERTIES_EDITOR_HTC8OG2V */

