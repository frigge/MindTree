import MT, PyQt5
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

class CustomWidget(MT.pytypes.CustomNodeWidget):
    name = "Add Property"
    
    def __init__(self, node, parent):
        MT.pytypes.CustomNodeWidget.__init__(self, node, parent)

        self.setLayout(QHBoxLayout())

        self.name_edit = QLineEdit("Name")
        self.type_edit = QLineEdit("Type")

        button = QPushButton("Add Property")

        self.layout().setMargin(0)
        self.layout().setSpacing(0)
        self.layout().addWidget(self.name_edit)
        self.layout().addWidget(self.type_edit)
        self.layout().addWidget(button)

        button.clicked.connect(self.addProperty)

    def addProperty(self):
        self.node.addInSocket(self.name_edit.text(), self.type_edit.text())
        self.editor.updateEditor(self.node)

class AddPropertiesNodeDecorator(MT.pytypes.NodeDecorator):
    type="ADDPROPERTIES"
    label="Objects.Add Properties"
    insockets = [("Object(s)", "GROUPDATA")]
    outsockets = [("Object(s)", "GROUPDATA")]
    customwidget = CustomWidget

class FilterObjectsNodeDecorator(MT.pytypes.NodeDecorator):
    type="FILTEROBJECTS"
    label="Objects.Filter"
    insockets = [("Objects", "GROUPDATA"),
            ("Name Regex", "STRING")]
    outsockets = [("Objects", "GROUPDATA")]


def registerNodes():
    MT.registerNode(AddPropertiesNodeDecorator)
    MT.registerNode(FilterObjectsNodeDecorator)

registerNodes()
