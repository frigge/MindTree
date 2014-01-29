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

#include "QTextEdit"
#include "graphics/viewer_dock_base.h"

namespace MindTree
{
class DNode;
} /* MindTree */
    
class TextViewerNode;
class SourceEdit : public QTextEdit
{
    Q_OBJECT
public:
    SourceEdit(QWidget *parent);

public slots:
    void load(MindTree::DNode *node);
    void load();

private:
    TextViewerNode *outnode;
};

class SourceDock : public MindTree::ViewerDockBase
{
public:
    SourceDock();
    virtual ~SourceDock();
    void exec(MindTree::DNode* node);
    void exec();
    //void setFocusNode(ViewerNodeBase *node);
    //void changeViewerNode(ViewerNodeBase* node);

private:
    SourceEdit *edit;
};
