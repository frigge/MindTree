#ifndef TRANSLATE_WIDGETS_H
#define TRANSLATE_WIDGETS_H

#include "widgets.h"

namespace MindTree {
class TranslateWidget : public Widget3D
{
public:
    enum Axis {
        X, Y, Z, XY, XZ, YZ
    };

    TranslateWidget(Axis axis);

protected:
    void mouseDraged(glm::vec3 point);

private:
    void updateValue(glm::vec3 distance);

    Axis _axis;
};

class TranslateXWidget : public TranslateWidget
{
public:
    TranslateXWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();

protected:
	bool hoverEntered(glm::vec3 p) override;

};

class TranslateYWidget : public TranslateWidget
{
public:
    TranslateYWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();

protected:
	bool hoverEntered(glm::vec3 p) override;
};

class TranslateZWidget : public TranslateWidget
{
public:
    TranslateZWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();

protected:
	bool hoverEntered(glm::vec3 p) override;
};

class TranslateXYPlaneWidget : public TranslateWidget
{
public:
    TranslateXYPlaneWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateXZPlaneWidget : public TranslateWidget
{
public:
    TranslateXZPlaneWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();
};

class TranslateYZPlaneWidget : public TranslateWidget
{
public:
    TranslateYZPlaneWidget();
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

class SwitchWidget : public Widget3D
{
public:
    SwitchWidget();
    MindTree::GL::ShapeRendererGroup* createRenderer();

protected:
    void mouseReleased();
};
}
#endif
