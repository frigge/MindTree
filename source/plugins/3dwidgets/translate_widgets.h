#ifndef TRANSLATE_WIDGETS_H
#define TRANSLATE_WIDGETS_H

#include "widgets.h"

class TranslateXWidget : public TranslateWidget
{
public:
    TranslateXWidget();
    virtual ~TranslateXWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateYWidget : public TranslateWidget
{
public:
    TranslateYWidget();
    virtual ~TranslateYWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateZWidget : public TranslateWidget
{
public:
    TranslateZWidget();
    virtual ~TranslateZWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateXYPlaneWidget : public TranslateWidget
{
public:
    TranslateXYPlaneWidget();
    virtual ~TranslateXYPlaneWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateXZPlaneWidget : public TranslateWidget
{
public:
    TranslateXZPlaneWidget();
    virtual ~TranslateXZPlaneWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateYZPlaneWidget : public TranslateWidget
{
public:
    TranslateYZPlaneWidget();
    virtual ~TranslateYZPlaneWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateScreenPlaneWidget : public Widget3D
{
public:
    TranslateScreenPlaneWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();

protected:
    void mouseDraged(glm::vec3 point);
};
#endif
