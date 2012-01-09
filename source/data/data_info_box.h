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

#ifndef INFO_BOX_O9AGS0XD

#define INFO_BOX_O9AGS0XD

#include "QString"
#include "QPointF"

class VInfoBox;

class DInfoBox
{
public:
    DInfoBox();
    virtual ~DInfoBox();

    VInfoBox* getVis();
    void setVis(VInfoBox *value);

    int getWidth();
    void setWidth(int value);
    int getHeight();
    void setHeight(int value);
    QPointF getPos();
    void setPos(QPointF value);
    QString getName();
    void setName(QString value);
    QString getText();
    void setText(QString value);

protected:
    VInfoBox* createVis();

private:
    VInfoBox *vis;
    int width, height;
    QPointF pos;
    QString name, text;
};

#endif /* end of include guard: INFO_BOX_O9AGS0XD */
