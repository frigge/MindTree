import MT, PySide
from PySide.QtCore import *
from PySide.QtGui import *

class StringViewerGUI(QWidget): 
    def __init__(self, viewer):
        self.viewer = viewer
        StringViewerGUI.viewers.append(self)
        QWidget.__init__(self)
        lay = QVBoxLayout()
        self.setLayout(lay)
        self.label = QLabel("")
        lay.addWidget(self.label)

class StringViewer(MT.pytypes.Viewer):
    def __init__(self, socket):
        MT.pytypes.Viewer.__init__(self, socket)
        self.viewer = StringViewerGUI(self)
        self.setWidget(self.viewer)
        self.socket = socket

    def update(self, s):
        print("updating viewer: s:"+str(s)+" , self.socket:"+str(self.socket))
        cache = MT.cache.DataCache(self.socket)
        data = cache.data
        self.viewer.label.setText(str(data))

MT.gui.registerViewer("StringViewer", "String", StringViewer)
