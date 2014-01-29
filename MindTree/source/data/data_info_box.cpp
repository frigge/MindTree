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

#include "data_info_box.h"

#include "source/lib/data/frg.h"
#include "source/lib/data/dnspace.h"

DInfoBox::DInfoBox()
    : width(100), height(100)
{
    //FRG::SpaceDataInFocus->addInfoBox(this);
}

DInfoBox::~DInfoBox()
{
    //FRG::SpaceDataInFocus->removeInfoBox(this);
}

int DInfoBox::getWidth()
{
    return width;
}

void DInfoBox::setWidth(int value)
{
    width = value;
}

int DInfoBox::getHeight()
{
    return height;
}

void DInfoBox::setHeight(int value)
{
    height = value;
}

QPointF DInfoBox::getPos()
{
    return pos;
}

void DInfoBox::setPos(QPointF value)
{
    pos = value;
}

QString DInfoBox::getText()
{
    return text;
}

void DInfoBox::setText(QString value)
{
    text = value;
}
