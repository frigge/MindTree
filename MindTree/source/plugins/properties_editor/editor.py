import MT
from PyQt4.QtGui import *
from PyQt4.QtCore import *

class StringEditor(QWidget):
    def __init__(self, socket):
        QLineEdit.__init__(self)
        self.socket = socket
        self.widget = QLineEdit()
        self.setLayout(QHBoxLayout())
        if socket.value != None:
            self.widget.setText(socket.value)
        self.layout().addWidget(self.widget)
        self.widget.textChanged.connect(lambda x: setattr(self.socket, "value", str(x)))

class DirEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = QLineEdit()
        self.setLayout(QHBoxLayout())
        if socket.value != None:
            self.widget.setText(socket.value)
        self.layout().addWidget(self.widget)
        self.browsebutton = QPushButton("...")
        self.layout().addWidget(self.browsebutton)
        self.browsebutton.pressed.connect(self.browseFilePath)
        self.widget.textChanged.connect(lambda x: setattr(self.socket, "value", str(x)))
        
        self.path = ""

    def browseFilePath(self):
        self.path = str(QFileDialog.getOpenFileName())
        print("thats what i get: "+self.path)
        self.widget.setText(self.path)

class FloatEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = QDoubleSpinBox()
        self.setLayout(QHBoxLayout())
        self.layout().addWidget(self.widget)
        self.widget.setValue(socket.value)
        self.widget.valueChanged.connect(lambda x: setattr(self.socket, "value", x))

class IntEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = QSpinBox()
        self.setLayout(QHBoxLayout())
        self.layout().addWidget(self.widget)
        self.widget.setValue(socket.value)
        self.widget.valueChanged.connect(lambda x: setattr(self.socket, "value", x))

class BoolEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = QCheckBox()
        self.setLayout(QHBoxLayout())
        self.layout().addWidget(QLabel(socket.name))
        self.layout().addWidget(self.widget)
        self.widget.setChecked(socket.value)
        self.widget.valueChanged.connect(lambda x: setattr(self.socket, "value", x))

class Editor(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.widget = QWidget()
        lay = QVBoxLayout()
        self.setLayout(lay)
        lay.addWidget(self.widget)
        self.widget.setLayout(QFormLayout())

        self.cb = MT.attachToSignal("selectionChanged", self.updateEditor)

    def sizeHint(self):
        return QSize(300, 100)

    def updateEditor(self, node):
        print("updating Properties Editor")
        self.layout().removeWidget(self.widget)
        self.widget = QWidget()
        self.widget.setLayout(QFormLayout())
        self.layout().addWidget(self.widget)
        for s in node.insockets:
            if s.connected == None:
                if s.type == "STRING":
                    self.widget.layout().addRow(s.name, StringEditor(s))
                if s.type == "DIRECTORY":
                    self.widget.layout().addRow(s.name, DirEditor(s))
                elif s.type == "FLOAT":
                    self.widget.layout().addRow(s.name, FloatEditor(s))
                elif s.type == "INTEGER":
                    self.widget.layout().addRow(s.name, IntEditor(s))
                elif s.type == "BOOLEAN":
                    self.widget.layout().addRow(s.name, BoolEditor(s))
    
MT.gui.registerWindow("PropertiesEditor", Editor)
