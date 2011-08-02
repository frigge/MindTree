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


#include <QtGui/QApplication>
#include "QStyle"

#include "frg_shader_author.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QPalette p(QColor(40, 40, 40));
//    p.setColor(QPalette::Text, QColor(0, 0, 0));
//    p.setColor(QPalette::ButtonText, QColor(0, 0, 0));
//    p.setColor(QPalette::Button, QColor(50, 50, 50));
//    p.setColor(QPalette::WindowText, QColor(255, 255, 255, 180));
//    p.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
//    a.setPalette(p);
    frg_Shader_Author *w = new frg_Shader_Author;
    w->show();
    return a.exec();
}
