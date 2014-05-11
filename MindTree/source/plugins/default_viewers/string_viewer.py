import MT, PySide
from PySide.QtCore import *
from PySide.QtGui import *

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

    def update(self, s):
        print("updating viewer: s:"+str(s)+" , self.socket:"+str(self.socket))
        data = self.cache.getOutput(self.socket)
        self.viewer.setText(data)

MT.gui.registerViewer("StringViewer", "STRING", StringViewer)
