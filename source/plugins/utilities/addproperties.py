import MT, PyQt4
from PyQt4.QtGui import *
from PyQt4.QtCore import *

class CustomWidget(MT.pytypes.CustomNodeWidget):
    def __init__(self, node, parent=None):
        MT.pytypes.CustomNodeWidget.__init__(self, node, parent)

        self.setLayout(QFormLayout())

        self.name_edit = QLineEdit()
        self.type_edit = QLineEdit()

        button = QPushButton("Add Property")

        self.layout().addRow("Name: ", self.name_edit)
        self.layout().addRow("Type: ", self.type_edit)
        self.layout().addRow("", button)

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
