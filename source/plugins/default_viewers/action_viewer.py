import MT, PyQt5
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

class ActionViewer(MT.pytypes.Viewer):
    def __init__(self, socket):
        MT.pytypes.Viewer.__init__(self, socket)
        self.label = QLabel("Action")
        self.widget = QWidget()
        self.widget.setLayout(QHBoxLayout())
        self.widget.layout().addWidget(self.label)
        self.setWidget(self.widget)

    def update(self):
        self.label.setText("Executing ...")
        data = self.cache.getOutput(self.socket)
        self.label.setText("Done ...")

MT.gui.registerViewer("ActionViewer", "ACTION", ActionViewer)
