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

#include <QMainWindow>
#include <QGraphicsScene>
#include <QApplication>
#include <QDialog>

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

namespace MindTree {
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static MainWindow* create();
    static MainWindow* window();
    void registerMenu(QMenu *menu);

    Q_SIGNAL void projectChanged();

    Q_SLOT void registerWindow(MindTree::WindowFactory *factory);
    Q_SLOT void openSettings();

    Q_SLOT //void createPreviewDock(DShaderPreview *prev);

    Q_SLOT static QWidget* addConsole();

    Q_SLOT void setupStyle();
#ifdef QT_DEBUG
    Q_SLOT void lookupStyle();
#endif

    Q_SLOT void showDock(QDockWidget* widget);

protected:
    void createMenus();
    void createDocks();
    void createSpaceDock();
    void initDefaultProperties();

private:
    static MainWindow *_window;

    qint64 style_age;
    QString stylePath;

    void change_window_title(QString);
    QHash<QString, QMenu*> menus;
};
} // namespace MindTree

#endif // FRG_SHADER_AUTHOR_H
