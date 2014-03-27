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

    def sizeHint(self):
        return QSize(50, 50)

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
            currItem = self
            for c in nt.split("."):
                if currItem is self:
                    for i in range(self.topLevelItemCount()):
                        if c == self.topLevelItem(i).text(0):
                            currItem = self.topLevelItem(i)
                            break

                    if currItem is not self:
                        continue

                    item = QTreeWidgetItem([c])
                    self.addTopLevelItem(item)
                    currItem = item
                    continue

                for i in range(currItem.childCount()):
                    if c == currItem.child(i).text(0):
                        currItem = currItem.child(i)
                        continue

                item = QTreeWidgetItem([c, nt])
                currItem.addChild(item)
                currItem = item

    def createNode(self, nodeType):
        def nodeSlot():
            node = MT.createNode(nodeType)
            self.graph.space.addNode(node)
        return nodeSlot


