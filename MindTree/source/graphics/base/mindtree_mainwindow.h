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
#include "QApplication"
#include "QDialog"

#include "data/project.h"
#include "data/windowfactory.h"
/*Forward declarations*/

class SettingsDialog : public QDialog
{
public:
    SettingsDialog(QWidget *parent);
    virtual ~SettingsDialog();

private:
};

class frg_Shader_Author : public QMainWindow
{
    Q_OBJECT

public:
    frg_Shader_Author(QWidget *parent = 0);
    ~frg_Shader_Author();

    static void start_app();
    bool isMouseNodeGraphPos();
    static void togglePickWidget();
    static void toggleMouseNodeGraphPos();

    void registerMenu(QMenu *menu);

signals:
    void projectChanged();

public slots:
    void registerWindow(MindTree::WindowFactory *factory);
    void openSettings();

    //void createPreviewDock(DShaderPreview *prev);

    static QWidget* addConsole();

    void setupStyle();
    void lookupStyle();

    void showDock(QDockWidget* widget);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void createMenus();
    void createDocks();
    void createSpaceDock();
    void initDefaultProperties();
    bool eventFilter(QObject *obj, QEvent *event);

private:
    qint64 style_age;
    QString stylePath;

    static bool pickWidget, mouseNodeGraphPos;
    void change_window_title(QString);
    QDockWidget *nodelib, *nodeeditor, *propeditor, *detailsview, *spaceDock;
    QHash<QString, QMenu*> menus;
};

#endif // FRG_SHADER_AUTHOR_H
