import MT, PySide
from PySide.QtCore import *
from PySide.QtGui import *

class FloatViewerGUI(QLabel): 
    def __init__(self, viewer):
        self.viewer = viewer
        QLabel.__init__(self)
        self.setStyleSheet("""
        * { 
            background: black;
            color: white;
        }
        """)

class FloatViewer(MT.pytypes.Viewer):
    def __init__(self, socket):
        MT.pytypes.Viewer.__init__(self, socket)
        self.viewer = FloatViewerGUI(self)
        self.setWidget(self.viewer)

    def update(self, s):
        data = self.cache.getOutput(self.socket)
        self.viewer.setText(str(data))

MT.gui.registerViewer("FloatViewer", "FLOAT", FloatViewer)
