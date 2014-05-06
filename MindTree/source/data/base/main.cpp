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

#include "source/graphics/base/mindtree_mainwindow.h"
#include "source/data/base/init.h"

int main(int argc, char *argv[])
{
    MindTree::initApp(argc, argv);

    if(MindTree::noGui()) {
        MindTree::finalizeApp();
        return 0;
    }

    QApplication a(argc, argv);

    MindTree::MainWindow *w = new MindTree::MainWindow();

    w->show();
    MindTree::initGui();
    return a.exec();
}
