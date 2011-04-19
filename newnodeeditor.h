#ifndef NEWNODEEDITOR_H
#define NEWNODEEDITOR_H

#include <QWidget>
#include "QTreeWidget"
#include "QComboBox"
#include "QLineEdit"

#include "node.h"
#include "scenewidgetcontainer.h"

class SlotTypeEditor : public QComboBox
{
    Q_OBJECT
public:
    SlotTypeEditor();
};

class RemoveButton : public QPushButton
{
    Q_OBJECT
public:
    RemoveButton(int);
    ~RemoveButton();
    void setIndex(int);

protected:
    int index;

public slots:
    void emitClicked();

signals:
    void clicked(int);
};

class SocketEditor : public QTreeWidget
{
    Q_OBJECT
public:
    SocketEditor();
    ~SocketEditor();

public slots:
    void addSocket();
    void removeSocket(int);

private:
    void refreshIndices();
};

class NewNodeEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NewNodeEditor(QPointF atPos, QWidget *parent = 0);
    ~NewNodeEditor();
    void setScene(QGraphicsScene *scene);
    void update();

signals:

public slots:
    void addtoLib();

private:
    void createLayout();
    QGraphicsScene *space;
    QGraphicsProxyWidget *nodeedit_proxy;
    SceneWidgetContainer *container;
    SocketEditor *inputsockets, *outputsockets;
    QLineEdit *func_name;
    QLineEdit *node_name;
    QGridLayout *grid;

};

#endif // NEWNODEEDITOR_H
