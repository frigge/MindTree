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

#include "shaderpreview.h"

#include "QCoreApplication"
#include "QDir"
#include "QObject"
#include "QTextStream"
#include "QFileDialog"
#include "QMenu"
#include "QInputDialog"
#include "QGraphicsSceneContextMenuEvent"
#include "iostream"

#include "source/data/base/frg.h"

unsigned short DShaderPreview::count = 0;

PreviewSceneEditor::PreviewSceneEditor()
{
    //init
    hlay = new QHBoxLayout();
    vlay = new QGridLayout();
    previewScenes = new QListWidget();
    browseScene = new QPushButton("...");
    browseMaterial= new QPushButton("...");
    browseImage= new QPushButton("...");
    image = new QLineEdit();
    material = new QLineEdit();
    scene = new QLineEdit();

    setLayout(hlay);
    hlay->addWidget(previewScenes);
    hlay->addLayout(vlay);

    sce = new QLabel("Scene");
    vlay->addWidget(sce, 0, 0);
    vlay->addWidget(scene, 0, 1);
    vlay->addWidget(browseScene, 0, 2);
    connect(browseScene, SIGNAL(clicked()), this, SLOT(brScene()));
    connect(scene, SIGNAL(textChanged(QString)), this, SLOT(updateFile(QString)));

    mat = new QLabel("Material");
    vlay->addWidget(mat, 1, 0);
    vlay->addWidget(material, 1, 1);
    vlay->addWidget(browseMaterial, 1, 2);
    connect(browseMaterial, SIGNAL(clicked()), this, SLOT(brMat()));
    connect(material, SIGNAL(textChanged(QString)), this, SLOT(updateFile(QString)));

    img = new QLabel("Image");
    vlay->addWidget(img, 2, 0);
    vlay->addWidget(image, 2, 1);
    vlay->addWidget(browseImage, 2, 2);
    connect(browseImage, SIGNAL(clicked()), this, SLOT(brImg()));
    connect(image, SIGNAL(textChanged(QString)), this, SLOT(updateFile(QString)));

    connect(previewScenes, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(updateEditFields(QListWidgetItem*, QListWidgetItem*)));
}

QHash<QString, PreviewScene> PreviewSceneEditor::getPreviews()    
{
    QHash<QString, PreviewScene> prevs;
    for(int i = 0; i < previewScenes->count(); i++)
        prevs.insert(previewScenes->item(i)->text(), previews.value(previewScenes->item(i)));

    return prevs;
}

void PreviewSceneEditor::updateEditFields(QListWidgetItem *current, QListWidgetItem *previous)    
{
    QString filename("preview/"+current->text()+".sce");
    QFileInfo fileInfo(filename);
    if(fileInfo.exists())
    {
        QFile file(filename);
        QTextStream stream(&file);
        file.open(QIODevice::ReadOnly);
        scene->setText(stream.readLine());
        material->setText(stream.readLine());
        image->setText(stream.readLine());
    }
    else
    {
        scene->clear();
        material->clear();
        image->clear();
    }

    FRG::previewScenes.updateCache();
}

void PreviewSceneEditor::updateFile(QString newval)    
{
    QListWidgetItem *current = previewScenes->currentItem();

    PreviewScene prevsce;
    prevsce.dir = current->text();
    prevsce.image = image->text();
    prevsce.material = material->text();
    prevsce.scene = scene->text();

    previews[current] = prevsce;
    
    QString filename("preview/"+current->text()+".sce");

    QFile file(filename);
    QTextStream stream(&file);
    file.open(QIODevice::WriteOnly);
    stream<<scene->text()<<"\n"<<material->text()<<"\n"<<image->text()<<"\n";
}

void PreviewSceneEditor::brScene()    
{
    QString text = QFileDialog::getOpenFileName(0, "", "preview/"+previewScenes->currentItem()->text());
    text.replace(QDir::currentPath()+"/preview/"+previewScenes->currentItem()->text()+"/", "");
    if(text!="") scene->setText(text);
}

void PreviewSceneEditor::brMat()    
{
    QString text = QFileDialog::getOpenFileName(0, "", "preview/"+previewScenes->currentItem()->text());
    text.replace(QDir::currentPath()+"/preview/"+previewScenes->currentItem()->text()+"/", "");
    if(text!="") material->setText(text);
}

void PreviewSceneEditor::brImg()    
{
    QString text = QFileDialog::getOpenFileName(0, "", "preview/"+previewScenes->currentItem()->text());
    text.replace(QDir::currentPath()+"/preview/"+previewScenes->currentItem()->text()+"/", "");
    if(text!="") image->setText(text);
}

void PreviewSceneEditor::show()    
{
    QDir previewDir("preview/");
    previewScenes->clear();
    QStringList entrylist = previewDir.entryList();
    entrylist.removeAll(".");
    entrylist.removeAll("..");
    QStringList prevDirs;
    foreach(QString str, entrylist)
        if(!str.endsWith(".sce"))
            prevDirs.append(str);

    foreach(QString prev, prevDirs)
        new QListWidgetItem(prev, previewScenes);

    QWidget::show();
}

PreviewSceneCache::PreviewSceneCache()
{
    updateCache();
}

void PreviewSceneCache::updateCache()    
{
    QDir dir("preview");
    QStringList entries = dir.entryList().filter(QRegExp(".+\.sce$"));
    previews.clear();

    foreach(QString filename, entries)
    {
        PreviewScene scene;
        QFile file("preview/"+filename);
        QTextStream stream(&file);
        file.open(QIODevice::ReadOnly);
        scene.scene = stream.readLine();
        scene.material = stream.readLine();
        scene.image = stream.readLine();
        QString dir = filename;
        dir.replace(".sce", "");
        scene.dir = dir;
        file.close();

        previews.insert(dir, scene);
    }
}

QList<QString> PreviewSceneCache::getPreviews()    
{
    updateCache();
    return previews.keys();
}

PreviewScene PreviewSceneCache::getScene(QString dirname)    
{
    return previews.value(dirname);
}

VShaderPreview::VShaderPreview(DShaderPreview *data)
    : VNode(data), img(0)
{
    preview = new QGraphicsPixmapItem(this);
    preview->setParentItem(this);
    preview->setZValue(preview->zValue() + .5);

    connect((QObject*)FRG::Space, SIGNAL(linkChanged()), (QObject*)this, SLOT(render()));
    connect((QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)), (QObject*)this, SLOT(render(DNode*)));

    contextMenu();
}

void VShaderPreview::contextMenu()    
{
    cxtM = new QMenu();
    QAction *prvAct = cxtM->addAction("Change Preview Scene");
    connect(prvAct, SIGNAL(triggered()), this, SLOT(changePreview()));
}

void VShaderPreview::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    cxtM->exec(event->screenPos());
}

void VShaderPreview::changePreview()    
{
    bool ok;
    QString newprev(QInputDialog::getItem(0, "Change Socket Type", "New Type", FRG::previewScenes.getPreviews(), 0, false, &ok));

    DShaderPreview* prev = static_cast<DShaderPreview*>(data);
    prev->setPrevScene(FRG::previewScenes.getScene(newprev));
    render();
}

void VShaderPreview::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    render();
}

void VShaderPreview::render()    
{
    DShaderPreview* prev = static_cast<DShaderPreview*>(data);
    prev->render();
}

void VShaderPreview::render(DNode* node)    
{
    DShaderPreview* prev = static_cast<DShaderPreview*>(data);
    if(prev->getAllInNodes().contains(node))
        prev->render();
}

void VShaderPreview::updatePreview()    
{
    QImage *oldimg = img;
    DShaderPreview* prev = static_cast<DShaderPreview*>(data);
    img = new QImage(prev->getImageFile());
    preview->setPixmap(QPixmap::fromImage(*img));
    if(oldimg) delete oldimg;
    updateNodeVis();
}

void VShaderPreview::NodeWidth()    
{
    setNodeWidth(104);
    if(img)
        setNodeWidth(img->width() + 4);
}

void VShaderPreview::NodeHeight(int numSockets)    
{
    VNode::NodeHeight(numSockets);
    if(img)
        setNodeHeight(img->height() + getNodeHeight());
}

void VShaderPreview::updateNodeVis()    
{
    NodeWidth();
    NodeHeight(4);
    drawName();
    recalcNodeVis();
    preview->setPos(2-getNodeWidth()/2, 4 + 4*(2 + SOCKET_HEIGHT) - getNodeHeight()/2);
}

DShaderPreview::DShaderPreview(bool raw)
    : prevID(++count)
{
    setNodeType(PREVIEW);
    shadername = "preview";
    shadername += QString::number(prevID);
    changeName(shadername);

    QDir libdir("preview");
    QDir appdir;

    if (!libdir.exists())
        appdir.mkdir("preview");

    if (!raw)
    {
        new DinSocket("Ci", COLOR, this);
        new DinSocket("Oi", COLOR, this);
        new DinSocket("P", POINT, this);
        new DinSocket("N", NORMAL, this);
    }

    renderprocess.setStandardErrorFile("render.log");

    QList<QString> previews = FRG::previewScenes.getPreviews();
    PreviewScene startsce;
    if(previews.isEmpty())
        return;
    if(previews.contains("default"))
        startsce = FRG::previewScenes.getScene("default");
    else
        startsce = FRG::previewScenes.getScene(previews.first());
    prevScene = startsce;
}

DShaderPreview::DShaderPreview(const DShaderPreview *preview)
    : OutputNode(preview), prevID(++count), prevScene(preview->getPrevScene())
{
    setNodeType(PREVIEW);
    shadername = "preview";
    shadername += QString::number(prevID);
    changeName(shadername);

    QString appdirstr(QCoreApplication::applicationDirPath());
    QDir appdir(appdirstr);
    QDir::setCurrent(appdirstr);
    QDir libdir("preview");

    if (!libdir.exists())
        appdir.mkdir("preview");

    renderprocess.setStandardErrorFile("render.log");
}

DShaderPreview::~DShaderPreview()
{
    --count;
}

PreviewScene DShaderPreview::getPrevScene() const
{
    return prevScene; 
}

void DShaderPreview::setPrevScene(PreviewScene scene)    
{
    prevScene = scene;
    createTmpPrevDir();
}

void DShaderPreview::createTmpPrevDir()    
{
    QDir::temp().mkdir(QString::number(prevID));
    QDir tmpdir(QDir::tempPath() + "/" + QString::number(prevID));
    QString tmppath = tmpdir.absolutePath();
    FRG::Utils::copyDir(QDir("preview/"+prevScene.dir), tmpdir);
    tmpScene.dir = tmppath +"/"+ prevScene.dir + "/";
    tmpScene.image = tmpScene.dir + prevScene.image;
    tmpScene.material = tmpScene.dir + prevScene.material;
    tmpScene.scene = tmpScene.dir + prevScene.scene;

    QString prevShadFile = tmpScene.dir;
    prevShadFile += shadername;
    prevShadFile += ".sl";
    setFileName(prevShadFile);
}

QString DShaderPreview::getImageFile()    
{
    return tmpScene.image;
}

const VShaderPreview* DShaderPreview::getPreviewVis()    
{
    return static_cast<const VShaderPreview*>(getNodeVis()); 
}

VNode * DShaderPreview::createNodeVis()    
{
    setNodeVis(new VShaderPreview(this));

    createTmpPrevDir();
    timer.setInterval(200);
    timer.connect(&renderprocess, SIGNAL(started()), &timer, SLOT(start()));
    timer.connect(&renderprocess, SIGNAL(finished(int, QProcess::ExitStatus)), &timer, SLOT(stop()));
    timer.connect(&timer, SIGNAL(timeout()), getPreviewVis(), SLOT(updatePreview()));
    timer.connect(&renderprocess, SIGNAL(finished(int, QProcess::ExitStatus)), getPreviewVis(), SLOT(updatePreview()));

    return const_cast<VNode*>(getNodeVis());
}

void DShaderPreview::deleteNodeVis()    
{
    OutputNode::deleteNodeVis();
    FRG::Utils::remove(QDir::tempPath() + "/" + QString::number(prevID));
}

void DShaderPreview::render()    
{
    if(getAllInNodes().isEmpty())return;
    writeCode();
    compile();
    writeMaterial();
    
    QStringList args;
    args << tmpScene.scene;
    renderprocess.setWorkingDirectory(tmpScene.dir);
    renderprocess.start("renderdl", args);
}

void DShaderPreview::writeMaterial()    
{
    QFile matrib(tmpScene.material);
    matrib.open(QIODevice::WriteOnly);
    QTextStream stream(&matrib);
    stream << "Surface \"" << getShaderName() << "\"";
    matrib.close();
}
