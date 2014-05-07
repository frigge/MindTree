import MT, PyQt4, properties_editor
from properties_editor import *
from PyQt4.QtGui import *
from PyQt4.QtCore import *

class CustomNodeWidget(QWidget):
    def __init__(self, node, parent=None):
        QWidget.__init__(self, parent)
        self.node = node
        self.editor = parent

class NodeFactory:
    type=""
    label=""

    insockets=[]
    outsockets=[]
    customwidget = None

    def __init__(self, node):
        node.name = self.label.split(".")[-1]

        for s in self.insockets:
            socket = node.addInSocket(s[0], s[1])
            try:
                socket.value = s[2]
            except IndexError:
                pass

        for s in self.outsockets:
            socket = node.addOutSocket(*s)

        if self.customwidget is not None:
            Editor.customWidgets[self.type] = self.customwidget

MT.pytypes.__dict__["NodeFactory"] = NodeFactory
MT.pytypes.__dict__["CustomNodeWidget"] = CustomNodeWidget
