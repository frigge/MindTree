import MT
from properties_editor import *
from nodefactory import *

class AddArrayElementWidget(MT.pytypes.CustomNodeWidget):
    name = "Add"
    
    def __init__(self, node, parent):
        MT.pytypes.CustomNodeWidget.__init__(self, node, parent)

        self.setLayout(QHBoxLayout())
        button = QPushButton("Add")

        self.layout().setMargin(0)
        self.layout().setSpacing(0)
        self.layout().addWidget(button)

        button.clicked.connect(self.add)

    def add(self):
        t = self.node.outsockets[0].type
        t = t[t.find(":")+1:]
        self.node.insockets[-1].type = t
        self.node.insockets[-1].name = str(len(self.node.insockets))
        self.node.addInSocket("+", "VARIABLE")
        self.editor.updateEditor()

Editor.customWidgets["ARRAYNODE"] = AddArrayElementWidget
