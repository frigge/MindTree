import MT, PyQt5
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

class StringViewerGUI(QScrollArea): 
    def __init__(self, viewer):
        self.viewer = viewer
        QScrollArea.__init__(self)
        self.label = QLabel("")
        self.setWidget(self.label)
        self.setStyleSheet("""
        * { 
            background: black;
            color: white;
        }
        """)

    def setText(self, text):
        self.label.setText(text)
        self.label.adjustSize()

    def resizeEvent(self, event):
        self.label.setFixedWidth(self.width())

class StringViewer(MT.pytypes.Viewer):
    def __init__(self, socket):
        MT.pytypes.Viewer.__init__(self, socket)
        self.viewer = StringViewerGUI(self)
        self.setWidget(self.viewer)

    def update(self):
        data = self.cache.getOutput(self.socket)
        self.viewer.setText(data)

MT.gui.registerViewer("StringViewer", "STRING", StringViewer)
