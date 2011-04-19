#include "QLayout"
#include "QtGui"

#include "newnodeeditor.h"
#include "scenewidgetcontainer.h"
#include "shader_space.h"

SlotTypeEditor::SlotTypeEditor()
{
    insertItem(0, "Color");
    insertItem(1, "Float");
    insertItem(2, "String");
    insertItem(3, "Point");
    insertItem(4, "Normal");
    insertItem(5, "Vector");
    insertItem(6, "Variable");

    setItemData(0,  COLOR);
    setItemData(1, FLOAT);
    setItemData(2, STRING);
    setItemData(3, POINT);
    setItemData(4, NORMAL);
    setItemData(5, VECTOR);
    setItemData(6, VARIABLE);
}

RemoveButton::RemoveButton(int index)
{
    setIndex(index);
    setText("X");
    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));
}

RemoveButton::~RemoveButton()
{

}

void RemoveButton::emitClicked()
{
    emit clicked(index);
}

void RemoveButton::setIndex(int index)
{
    this->index = index;
}

SocketEditor::SocketEditor()
{
    QStringList inputheader;
    inputheader<<"Name"<<"Type"<<"Token"<<"Dynamic"<<"Remove";
    setColumnCount(6);
    setHeaderLabels(inputheader);
    setAlternatingRowColors(true);
    header()->resizeSection(0, 90);
    header()->resizeSection(1, 70);
    header()->resizeSection(2, 40);
    header()->resizeSection(3, 40);
    header()->resizeSection(5, 40);
}

SocketEditor::~SocketEditor()
{

}

void SocketEditor::addSocket()
{
    int index = topLevelItemCount();
    QTreeWidgetItem *newitem = new QTreeWidgetItem();
    newitem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable |
                      Qt::ItemIsEnabled);
    addTopLevelItem(newitem);
    setItemWidget(newitem, 1, new SlotTypeEditor);
    setItemWidget(newitem, 2, new QCheckBox());
    setItemWidget(newitem, 3, new QCheckBox());
    RemoveButton *rembutton = new RemoveButton(index);
    connect(rembutton, SIGNAL(clicked(int)), this, SLOT(removeSocket(int)));
    setItemWidget(newitem, 4,rembutton);
}

void SocketEditor::removeSocket(int index)
{
    QTreeWidgetItem *item = takeTopLevelItem(index);
    delete item;
    refreshIndices();
}

void SocketEditor::refreshIndices()
{
    QTreeWidgetItem *item = topLevelItem(0);
    for(int i = 0; item; i++)
    {
        RemoveButton *button = (RemoveButton*)itemWidget(item, 5);
        button->setIndex(i);
        item = itemBelow(item);
    }
}

NewNodeEditor::NewNodeEditor(QPointF atPos, QWidget *parent) :
    QWidget(parent)
{
    resize(700, 400);
    nodeedit_proxy = new QGraphicsProxyWidget;
    nodeedit_proxy->setWidget(this);
    nodeedit_proxy->setFlag(QGraphicsItem::ItemAcceptsInputMethod, true);
    nodeedit_proxy->setFlag(QGraphicsItem::ItemIsFocusable, true);
    nodeedit_proxy->setFlag(QGraphicsItem::ItemIsSelectable, true);
    container = new SceneWidgetContainer;
    container->setProxy(nodeedit_proxy);
    container->setPos(atPos);

    setUpdatesEnabled(true);
    createLayout();
}

NewNodeEditor::~NewNodeEditor()
{
    delete inputsockets;
    delete outputsockets;
    delete func_name;
    delete node_name;
}

void NewNodeEditor::setScene(QGraphicsScene *scene)
{
    space = scene;
    space->addItem(container);
}

void NewNodeEditor::createLayout()
{
    grid = new QGridLayout;

    QLabel *name_label = new QLabel("Node Name:");
    node_name = new QLineEdit;
    node_name->setText("New Node");
    grid->addWidget(name_label, 0, 0);
    grid->addWidget(node_name, 0, 1);

    QLabel *func_label = new QLabel("Function Name:");
    func_name = new QLineEdit;
    grid->addWidget(func_label, 1, 0);
    grid->addWidget(func_name, 1, 1);

    QPushButton *add_in_button = new QPushButton("add Input");
    grid->addWidget(add_in_button, 3, 0);
    QPushButton *add_out_button = new QPushButton("add Output");
    grid->addWidget(add_out_button, 3, 1);

    inputsockets = new SocketEditor;
    grid->addWidget(inputsockets, 4, 0);

    outputsockets = new SocketEditor;
    grid->addWidget(outputsockets, 4, 1);

    QPushButton *add_to_lib = new QPushButton("Add to Library");
    grid->addWidget(add_to_lib, 5, 0, 1, 2);

    setLayout(grid);

    connect(add_in_button, SIGNAL(clicked()), inputsockets, SLOT(addSocket()));
    connect(add_out_button, SIGNAL(clicked()), outputsockets, SLOT(addSocket()));
    connect(add_to_lib, SIGNAL(clicked()), this, SLOT(addtoLib()));
}

void NewNodeEditor::update()
{
    QWidget::update();
}

void NewNodeEditor::addtoLib()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QString filename;
    filename.append("nodes/");
    filename.append(node_name->text());
    filename.append(".node");
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    qint16 outs, ins;
    outs = outputsockets->topLevelItemCount();
    ins = inputsockets->topLevelItemCount();

    NType nodetype;
    nodetype = FUNCTION;

    out<<node_name->text()<<(qint16)0<<nodetype<<QPointF(0, 0);
    out<<ins<<outs;
    QTreeWidgetItem *item = inputsockets->topLevelItem(0);
    while(item)
    {
        QComboBox *sockettype = dynamic_cast<QComboBox*>(inputsockets->itemWidget(item, 1));
        QCheckBox *isToken = dynamic_cast<QCheckBox*>(inputsockets->itemWidget(item, 2));
        QCheckBox *isVar = dynamic_cast<QCheckBox*>(inputsockets->itemWidget(item, 3));
        QString socketname(item->text(0));
        socket_type stype = (socket_type)sockettype->itemData(sockettype->currentIndex()).toInt();

        out<<(qint16)0<<isVar->isChecked()<<(socket_dir)IN;
        out<<isToken->isChecked()<<socketname<<stype;
        item = inputsockets->itemBelow(item);
    }
    item = outputsockets->topLevelItem(0);
    while(item)
    {
        QComboBox *sockettype = dynamic_cast<QComboBox*>(outputsockets->itemWidget(item, 1));
        QCheckBox *isToken = dynamic_cast<QCheckBox*>(outputsockets->itemWidget(item, 2));
        QCheckBox *isVar = dynamic_cast<QCheckBox*>(outputsockets->itemWidget(item, 3));
        QString socketname(item->text(0));
        socket_type stype = (socket_type)sockettype->itemData(sockettype->currentIndex()).toInt();

        out<<(qint16)0<<isVar->isChecked()<<(socket_dir)OUT;
        out<<isToken->isChecked()<<socketname<<stype;
        item = outputsockets->itemBelow(item);
    }

    out<<func_name->text();
    file.close();
    Shader_Space *shaderspace = dynamic_cast<Shader_Space *>(space);
    if(shaderspace->nodelib)
        shaderspace->nodelib->update();
}
