import MT
from PyQt4.QtGui import *

class SyncInputsWidget(MT.pytypes.CustomNodeWidget):
    name = "Add"
    
    def __init__(self, node, parent):
        MT.pytypes.CustomNodeWidget.__init__(self, node, parent)

        self.setLayout(QHBoxLayout())

        button = QPushButton("sync")

        self.layout().setMargin(0)
        self.layout().setSpacing(0)
        self.layout().addWidget(button)

        button.clicked.connect(self.sync)

    def sync(self):
        axioms = []
        axiom_array_node = None
        transform_array_node = None
        for child in self.node.insockets[1].childNodes:
            if child.type == "ARRAYNODE":
                axiom_array_node = child
                axioms = [s.value for s in child.insockets if s.value is not None
                          and s.type != "VARIABLE"]
        for child in self.node.insockets[2].childNodes:
            if child.type == "ARRAYNODE":
                transform_array_node = child

        print(axioms)
        for a in axioms:

            if not a in [s.name for s in transform_array_node.insockets]:
                transform_array_node.addInSocket(a, "TRANSFORMABLE")

class LSystemNodeDecorator(MT.pytypes.NodeDecorator):
    label = "LSystem"
    type = "LSYSTEMNODE"
    insockets = [
        ("Axion", "STRING"),
        ("Rules", "LIST:STRING"),
        ("Transformations", "LIST:TRANSFORMABLE"),
        ("Iterations", "INTEGER")
    ]
    outsockets = [("Skeleton", "TRANSFORMABLE")]
    #customwidget = SyncInputsWidget

MT.registerNode(LSystemNodeDecorator)

