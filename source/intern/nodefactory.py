import MT, PyQt5, properties_editor
from properties_editor import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

class CustomNodeWidget(QWidget):
    name = ""

    def __init__(self, node, parent):
        QWidget.__init__(self, parent)
        self.node = node
        self.editor = parent

class NodeDecorator:
    type=""
    label=""
    base=""

    insockets=[]
    outsockets=[]
    customwidget = None

    def __init__(self, node, raw=False):
        node.name = self.label.split(".")[-1]

        if not raw:
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

MT.pytypes.__dict__["NodeDecorator"] = NodeDecorator
MT.pytypes.__dict__["CustomNodeWidget"] = CustomNodeWidget
