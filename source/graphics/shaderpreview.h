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

#ifndef FRGShaderPreview
#define FRGShaderPreview
#include "QGraphicsPixmapItem"
#include "QTimer"
#include "QProcess"
#include "QWidget"
#include "QListWidget"
#include "QLineEdit"
#include "QHBoxLayout"
#include "QGridLayout"
#include "QPushButton"
#include "QLabel"

#include "source/data/nodes/data_node.h"
#include "source/data/code_generator/outputs.h"
#include "source/graphics/nodes/graphics_node.h"

typedef struct _PreviewScene 
{
    QString material;
    QString image;
    QString scene;
    QString dir;
} PreviewScene;

class PreviewSceneEditor : public QWidget
{
    Q_OBJECT
public:
    PreviewSceneEditor();
    void show();

public slots:
    void brImg();
    void brScene();
    void brMat();
    void updateEditFields(QListWidgetItem *current, QListWidgetItem *previous);
    void updateFile(QString newval);
    QHash<QString, PreviewScene> getPreviews();

private:
    QListWidget *previewScenes;
    QLineEdit *image, *scene, *material;
    QPushButton *browseImage, *browseScene, *browseMaterial;
    QHBoxLayout *hlay;
    QGridLayout *vlay;
    QLabel *img, *sce, *mat;
    QHash<QListWidgetItem*, PreviewScene> previews;
};

class PreviewSceneCache
{
public:
    PreviewSceneCache();
    PreviewScene getScene(QString dirname);
    QList<QString> getPreviews();
    void updateCache();

private:
    QHash<QString, PreviewScene> previews;
};

class DShaderPreview;

class VShaderPreview : public VNode
{
    Q_OBJECT
public:
    VShaderPreview(DShaderPreview *data);
    virtual ~VShaderPreview();
    void updateNodeVis();

public slots:
    void updatePreview();
    void render();
    void render(DNode* node);
    void changePreview();
    void externalPreview();
    void updateScene();
    void detachP();
    void viewCode();

protected:
    void NodeWidth();
    void NodeHeight(int numSockets);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void contextMenu();

private:
    QMenu *cxtM;
    int resx, resy;
    QGraphicsPixmapItem *preview;
    QImage *img;
};

class PreviewDock;

class DShaderPreview : public RSLOutputNode
{
public:
    DShaderPreview(bool raw=false);
    DShaderPreview(const DShaderPreview *preview);
    ~DShaderPreview();

    void render(DNode *node=0);
    QString getImageFile();
    void writeMaterial();
    void setPrevScene(PreviewScene scene);
    PreviewScene getPrevScene() const;
    void createTmpPrevDir();
    void createTmpExtPrevDir();
    void updateScene();
    void setFileName(QString name);

    static unsigned short count;
    bool isExtScene() const;
    void setExtScene(bool ext);
    void attach();
    void detach();
    bool isDetached();
    QProcess* getRenderProcess();
    QTimer* getTimer();
    void showDock();
    void changeName(QString name);

    virtual void setSpace(DNSpace* value);

protected:
    void setSockets();
    const VShaderPreview* getPreviewVis();
    virtual VNode* createNodeVis();
    void deleteNodeVis();

private:
    unsigned short prevID;
    PreviewScene prevScene;
    PreviewScene tmpScene;
    QTimer timer;
    QProcess renderprocess;
    QString shadername;
    bool ext_scene;
    bool detached;
    PreviewDock *dock;
};

#endif /* FRGShaderPreview */
