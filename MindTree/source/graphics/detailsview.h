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

#ifndef DETAILSDOCK_9CL1HBMB

#define DETAILSDOCK_9CL1HBMB

#include "QTreeWidget"

class DetailsView : public QTreeWidget
{
    Q_OBJECT
public:
    DetailsView(QWidget *parent);
    virtual ~DetailsView();

public slots:
    void updateFocus();

protected:
    void setFocusNode(DNode* node);

private:
    DNode *node;
};

#endif /* end of include guard: DETAILSDOCK_9CL1HBMB */
