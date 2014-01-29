import PySide, MT
from PySide.QtGui import *
from PySide.QtCore import *
from PySide.QtOpenGL import *

class Test(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        lay = QVBoxLayout()
        self.setLayout(lay)
        self.button = QPushButton("open View")
        self.button.clicked.connect(self.openView)
        lay.addWidget(self.button)

    def openView(self):
        scene = QGraphicsScene()
        scene.addItem(QGraphicsRectItem(0, 0, 10, 10))
        view = QGraphicsView(scene)
        view.setViewport(QGLWidget())
        print("open view")
        view.show()


MT.gui.registerWindow("OpenGLTest", Test)
