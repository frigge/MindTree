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

#include "QComboBox"
#include "QWidgetAction"

#include "iostream"
#include "viewport.h"
#include "data/nodes/data_node.h"
#include "../../render/rendertree.h"
#include "../../render/renderpass.h"
#include "../../render/render.h"
#include "../../render/benchmark.h"
#include "data/debuglog.h"

#include "viewport_widget.h"

using namespace MindTree;
ViewportViewer::ViewportViewer(DoutSocket *socket)
    : Viewer(socket)
{
    setWidget(new ViewportWidget(this));
    setupSettingsNode();
}

ViewportViewer::~ViewportViewer()
{
}

void ViewportViewer::setupSettingsNode()
{
    _settingsNode = std::make_shared<DNode>();
    _settingsNode->setType("SETPROPERTYMAP");
    new DoutSocket("Properties", "PROPERTYMAP", _settingsNode.get());

    DinSocket *s{nullptr};
    auto *viewportSettings = new DinSocket("Viewport Settings", "PROPERTYMAP", _settingsNode.get());
    auto node = std::make_shared<DNode>();
    node->setName("GL");
    node->setType("SETPROPERTYMAP");
    viewportSettings->addChildNode(node);

    s = new DinSocket("defaultLighting", "BOOLEAN", node.get());
    s->setProperty(true);
    s = new DinSocket("showpoints", "BOOLEAN", node.get());
    s->setProperty(true);
    s = new DinSocket("showedges", "BOOLEAN", node.get());
    s->setProperty(true);
    s = new DinSocket("showpolygons", "BOOLEAN", node.get());
    s->setProperty(true);
    s = new DinSocket("flatshading", "BOOLEAN", node.get());
    s->setProperty(false);
    s = new DinSocket("showgrid", "BOOLEAN", node.get());
    s->setProperty(true);
    s = new DinSocket("backgroundColor", "COLOR", node.get());
    float value = 70.0 / 255;
    s->setProperty(glm::vec4(value, value, value, 1.));

    auto *out = new DoutSocket("Properties", "PROPERTYMAP", node.get());
    viewportSettings->setCntdSocket(out);

    auto widget = dynamic_cast<ViewportWidget*>(getWidget());
    auto *viewport = widget->getViewport();
    createSettingsFromMap(_settingsNode.get(), viewport->getSettings());
}

void ViewportViewer::createSettingsFromMap(DNode *node, PropertyMap props)
{
    std::string first_prop = begin(props)->first;
    dbout(first_prop);
    auto prop_start = first_prop.find_first_of(":");
    std::string category_name = first_prop.substr(0, prop_start);
    dbout(category_name);
    node->setName(category_name);
    for(const auto &prop: props) {
        std::string prop_name = prop.first.substr(prop_start + 1);
        DinSocket *socket = new DinSocket(prop_name, prop.second.getType(), node);
        if(prop.second.getType() == "PROPERTYMAP") {
            auto n = std::make_shared<DNode>();
            auto *out = new DoutSocket("Properties", "PROPERTYMAP", n.get());
            socket->setCntdSocket(out);

            node->setName(prop.first);
            node->setType("SETPROPERTYMAP");
            socket->addChildNode(n);
            createSettingsFromMap(n.get(), prop.second.getData<PropertyMap>());
        }
        else {
            socket->setProperty(prop.second);
        }
    }
}

void ViewportViewer::update()
{
    auto widget = dynamic_cast<ViewportWidget*>(getWidget());
    auto *viewport = widget->getViewport();
    Property data = dataCache.getOutput(getStart());
    Property settings = settingsCache.getOutput(_settingsNode->getOutSockets()[0]);
    PropertyMap properties = settings.getData<PropertyMap>();

    GroupPtr grp;
    if(data.getType() == "GROUPDATA") {
        grp = data.getData<GroupPtr>();
    }
    else if(data.getType() == "TRANSFORMABLE") {
        auto obj = data.getData<AbstractTransformablePtr>();
        assert(obj);
        grp = std::make_shared<Group>();
        grp->addMember(obj);
    }
    for(auto prop : properties) {
        viewport->setOption(prop.first, prop.second);
        grp->setProperty(prop.first, prop.second);
    }
    viewport->setData(grp);
    widget->setCameras();
}

ViewportWidget::ViewportWidget(ViewportViewer *viewer)
    : _viewport(new Viewport(this)), _viewer(viewer)
{
    createMainLayout();
    createToolbar();
}

ViewportWidget::~ViewportWidget()
{
    delete _viewer;
}

void ViewportWidget::setCameras()
{
    auto cameras = _viewport->getCameras();

    //try to cache last camera
    QString lastcam = _camBox->currentText();
    _camBox->clear();
    QStringList list;
    list.append("Default");
    for(const auto &cam : cameras) {
        list.append(cam.c_str());
    }

    _camBox->addItems(list);

    int itemIndex = _camBox->findText(lastcam);
    _camBox->setCurrentIndex(itemIndex);
}

Viewport* ViewportWidget::getViewport()    
{
    return _viewport;
}

QSize ViewportWidget::sizeHint()    const
{
    return QSize(400, 800); 
}

void ViewportWidget::createMainLayout()    
{
    auto mainlayout = new QVBoxLayout();
    setLayout(mainlayout);
    _tools = new QToolBar;
    mainlayout->addWidget(_tools);
    mainlayout->addWidget(_viewport);
}

void ViewportWidget::resetViewport()
{
    layout()->addWidget(_viewport);
}

void ViewportWidget::createToolbar()    
{
    QAction *fullscreenAction = _tools->addAction("Fullscreen");

    QAction *overrideOutputAction = _tools->addAction("Override Output");
    overrideOutputAction->setCheckable(true);
    overrideOutputAction->setChecked(false);

    auto outputs = _viewport->getRenderTree()->getAllOutputs();
    _outputBox = new QComboBox();
    _outputBox->setEnabled(false);

    QStringList l;
    for(auto out : outputs)
        l.append(out.c_str());

    _outputBox->addItems(l);

    _camBox = new QComboBox();
    auto *camboxAction = new QWidgetAction(_tools);
    auto *outputBoxAction = new QWidgetAction(_tools);
    camboxAction->setDefaultWidget(_camBox);
    outputBoxAction->setDefaultWidget(_outputBox);
    _tools->addAction(camboxAction);
    _tools->addAction(outputBoxAction);

    connect(_outputBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setOutput(QString)));
    connect(_camBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setCamera(QString)));

    connect(overrideOutputAction, SIGNAL(toggled(bool)), this, SLOT(setOverrideOutput(bool)));
    connect(fullscreenAction, SIGNAL(triggered(bool)), this, SLOT(setFullscreen()));
}

void ViewportWidget::setFullscreen()
{
    _viewport->setParent(nullptr);

    _viewport->setWindowState(_viewport->windowState() | Qt::WindowFullScreen);
    _viewport->show();
    dbout("fullscreen");
}

void ViewportWidget::setCamera(QString cam)
{
    _viewport->changeCamera(cam.toStdString());
}

void ViewportWidget::setOverrideOutput(bool value)
{
    _outputBox->setEnabled(value);
    if(!value) {
        _viewport->clearOverrideOutput();
    }
}

void ViewportWidget::setOutput(QString out)
{
    _viewport->clearOverrideOutput();
    _viewport->setOverride(out.toStdString());
}
