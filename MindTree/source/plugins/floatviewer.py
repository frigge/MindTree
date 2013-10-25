import MT
import PySide
from PySide.QtCore import *
from PySide.QtGui import *

class FloatViewerGUI(QWidget):
    def __init__(self, viewer):
        self.viewer = viewer
        QWidget.__init__(self)
        self.setLayout(QHBoxLayout())
        self.label = QLabel("0.0")
        self.layout().addWidget(self.label)

    def __del__(self):
        print("FloatViewerGUI deleted")

class FloatViewer(MT.pytypes.Viewer):
    def __init__(self, socket):
        print("Viewer created starting at: " + str(socket))
        MT.pytypes.Viewer.__init__(self, socket)
        self.viewer = FloatViewerGUI(self)
        self.setWidget(self.viewer)
        self.socket = socket

    def __del__(self):
        print("FloatViewer deleted")

    def update(self, s):
        print("updating viewer: s:"+str(s)+" , self.socket:"+str(self.socket))
        cache = MT.cache.DataCache(self.socket)
        data = cache.data
        self.viewer.label.setText(str(data))

MT.gui.registerViewer("FloatViewer", "Float", FloatViewer)
