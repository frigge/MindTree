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

    def show(self):
        self.node_filter.setFocus()
        super().show()

    def filter(self, text):
        it  = QTreeWidgetItemIterator(self.browser)
        item = it.value()
        while item is not None:
            if text.lower() in item.text(1).lower():
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

    def sizeHint(self):
        return QSize(100, 200)

    def mousePressEvent(self, event):
        QTreeWidget.mousePressEvent(self, event)
        self.pressPos = event.pos()

    def keyReleaseEvent(self, event):
        QTreeWidget.keyReleaseEvent(self, event)
        if event.key() == Qt.Key_Return:
            node = self.selectedItems()[0].text(1)
            self.graph.dropNode(node, self.graph.mapToScene(self.graph.rect().center()))

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


