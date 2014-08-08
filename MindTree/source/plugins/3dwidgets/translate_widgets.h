#ifndef TRANSLATE_WIDGETS_H
#define TRANSLATE_WIDGETS_H

#include "widgets.h"

class TranslateXWidget : public TranslateWidget
{
public:
    TranslateXWidget();
    virtual ~TranslateXWidget();
    MindTree::GL::Widget3dRenderer* createRenderer();
};

class TranslateYWidget : public TranslateWidget
{
public:
    TranslateYWidget();
    virtual ~TranslateYWidget();
    MindTree::GL::Widget3dRenderer* createRenderer();
};

class TranslateZWidget : public TranslateWidget
{
public:
    TranslateZWidget();
    virtual ~TranslateZWidget();
    MindTree::GL::Widget3dRenderer* createRenderer();
};

class TranslateXYPlaneWidget : public TranslateWidget
{
public:
    TranslateXYPlaneWidget();
    virtual ~TranslateXYPlaneWidget();
    MindTree::GL::Widget3dRenderer* createRenderer();
};

class TranslateXZPlaneWidget : public TranslateWidget
{
public:
    TranslateXZPlaneWidget();
    virtual ~TranslateXZPlaneWidget();
    MindTree::GL::Widget3dRenderer* createRenderer();
};

class TranslateYZPlaneWidget : public TranslateWidget
{
public:
    TranslateYZPlaneWidget();
    virtual ~TranslateYZPlaneWidget();
    MindTree::GL::Widget3dRenderer* createRenderer();
};

class TranslateScreenPlaneWidget : public Widget3D
{
public:
    TranslateScreenPlaneWidget();
    MindTree::GL::Widget3dRenderer* createRenderer();

protected:
    void mouseDraged(glm::vec3 point);
};
#endif
