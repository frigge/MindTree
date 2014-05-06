import MT
from PySide.QtGui import *
from PySide.QtOpenGL import *
from PySide.QtCore import *

from functools import partial


class NodeBrowser(QTreeWidget):
    def __init__(self, graph):
        QTreeWidget.__init__(self)
        self.resize(50, 50)
        self.tree = {}
        self.graph = graph
        self.setHeaderLabel("")
        self.setHeaderHidden(True)
        self.setDragEnabled(True)
        self.setAlternatingRowColors(True)

    def sizeHint(self):
        return QSize(100, 50)

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


