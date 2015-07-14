import MT
from PySide.QtGui import *
from PySide.QtOpenGL import *
from PySide.QtCore import *

from functools import partial

class NodeBrowser(QWidget):
    def __init__(self, graph):
        super().__init__(graph, Qt.Popup)
        #QWidget.__init__(self, parent, Qt.Popup)
        self.setLayout(QVBoxLayout())
        self.browser = NodeBrowserTree(graph)
        self.browser.initList()
        self.node_filter = QLineEdit()
        self.layout().addWidget(self.node_filter)
        self.layout().addWidget(self.browser)

        self.node_filter.textChanged.connect(self.filter)
        self.node_filter.installEventFilter(self)

        dropCenterShortcut = QShortcut(self)
        dropConnectShortcut = QShortcut(self)
        dropCenterShortcut.setContext(Qt.WidgetWithChildrenShortcut)
        dropConnectShortcut.setContext(Qt.WidgetWithChildrenShortcut)
        dropCenterShortcut.setKey(QKeySequence(Qt.Key_Return))
        dropConnectShortcut.setKey(QKeySequence(Qt.Key_Shift, Qt.Key_Return))
        dropCenterShortcut.activated.connect(self.browser.dropNodeCenter)
        dropConnectShortcut.activated.connect(self.browser.dropNodeConnect)

    def show(self):
        self.node_filter.setFocus()
        super().show()

    def eventFilter(self, obj, event):
        if obj is self.node_filter:
            if (event.type() == QEvent.Type.KeyPress
                and (event.key() == Qt.Key_Up
                     or event.key() == Qt.Key_Down)):
                self.browser.keyPressEvent(event)
                return True
        return super().eventFilter(obj, event)

    def filter(self, text):
        it  = QTreeWidgetItemIterator(self.browser)
        item = it.value()
        first = True
        while item is not None:
            if text.lower() in item.text(1).lower():
                if first and item.childCount() == 0:
                    item.setSelected(True)
                    first = False
                item.setHidden(False)
                parent = item.parent()
                while parent:
                    parent.setHidden(False)
                    parent.setExpanded(True)
                    parent = parent.parent()
            else:
                item.setHidden(True)

            if not text:
                item.setHidden(False)
                parent = item.parent()
                while parent:
                    parent.setExpanded(False)
                    parent = parent.parent()
            it += 1
            item = it.value()

    def sizeHint(self):
        return QSize(100, 200)

    def initList(self):
        self.browser.initList()

class NodeBrowserTree(QTreeWidget):
    def __init__(self, graph, parent=None):
        super().__init__()
        self.resize(100, 200)
        self.tree = {}
        self.graph = graph
        self.setHeaderLabel("")
        self.setHeaderHidden(True)
        self.setDragEnabled(True)

    def dropNodeCenter(self):
        node = self.selectedItems()[0].text(1)
        self.graph.dropNode(node,
                            self.graph.mapToScene(self.graph.rect().center()))

    def dropNodeConnect(self):
        print("hello world")
        last_node = self.graph.scene().selectedItems()[0]
        newPos = last_node.pos() + QPoint(0, 40)
        node = self.selectedItems()[0].text(1)
        self.graph.dropNode(node, newPos)

        self.graph.scene().nodes[node]


    def sizeHint(self):
        return QSize(100, 200)

    def mousePressEvent(self, event):
        QTreeWidget.mousePressEvent(self, event)
        self.pressPos = event.pos()

    def mouseMoveEvent(self, event):
        if event.buttons() & Qt.LeftButton and len(self.selectedItems()) > 0:
            if ((event.pos() - self.pressPos).manhattanLength()
                > QApplication.startDragDistance()):
                drag = QDrag(self)
                mimedata = QMimeData()

                #figure out node name
                item = self.selectedItems()[0]

                #set drag data
                mimedata.setText(item.text(1))
                drag.setMimeData(mimedata)

                #fire off
                drag.exec_()
        else:
            QTreeWidget.mouseMoveEvent(self, event)

    def dragEnterEvent(self, event):
        QTreeWidget.dragEnterEvent(self, event)

    def initList(self):
        nodeTypes = MT.getRegisteredNodes()

        for nt in nodeTypes:
            currItem = self.invisibleRootItem();
            for c in nt.split("."):
                exists = False
                for i in range(currItem.childCount()):
                    child = currItem.child(i)
                    if child is None: continue
                    if c == child.text(0):
                        currItem = child
                        exists = True
                        continue

                if not exists:
                    item = QTreeWidgetItem([c, nt])
                    currItem.addChild(item)
                    currItem = item

    def createNode(self, nodeType):
        def nodeSlot():
            node = MT.createNode(nodeType)
            self.graph.space.addNode(node)
        return nodeSlot


