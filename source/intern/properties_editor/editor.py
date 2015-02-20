import MT
from PyQt4.QtGui import *
from PyQt4.QtCore import *
from .spinbox import *

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
        self.widget.setText(self.path)

class FloatEditor(QWidget):

    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = SpinBox(0.0)
        self.setLayout(QHBoxLayout())
        self.layout().addWidget(self.widget)

        if socket.value != None:
            self.widget.setValue(socket.value)
        self.widget.valueChanged.connect(lambda x: setattr(self.socket, "value", x))


class IntEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = QSpinBox()
        self.widget.setRange(-100000, 100000)

        self.setLayout(QHBoxLayout())
        self.layout().addWidget(self.widget)
        if socket.value != None:
            self.widget.setValue(socket.value)
        self.widget.valueChanged.connect(lambda x: setattr(self.socket, "value", x))

class BoolEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = QCheckBox()
        self.setLayout(QHBoxLayout())
        self.layout().addWidget(self.widget)
        if socket.value != None:
            self.widget.setCheckState(Qt.Checked if socket.value else Qt.Unchecked)
        self.widget.stateChanged.connect(self.setChecked)

    def setChecked(self, val):
        bval = val == 2
        self.socket.value = bval

class ColorButton(QPushButton):
    colorChanged = pyqtSignal(QColor)

    def __init__(self):
        QPushButton.__init__(self, "")
        self.setFlat(True)
        self.clicked.connect(self.pickColor)
        self.color = QColor(255, 255, 255, 255)

    def setColor(self, color):
        self.color = color
        self.colorChanged.emit(color)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setBrush(QBrush(self.color))
        painter.drawRect(self.rect())

    def pickColor(self):
        dialog =  QColorDialog(self.color)
        dialog.currentColorChanged.connect(self.setColor)
        dialog.setOptions(QColorDialog.ShowAlphaChannel)
        dialog.exec_()

class ColorEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket
        self.widget = ColorButton()
        self.setLayout(QHBoxLayout())
        self.layout().addWidget(self.widget)
        if socket.value != None:
            value = socket.value
            col = QColor(value[0]*255, value[1]*255, value[2]*255, value[3]*255)
            self.widget.setColor(col)
        self.widget.colorChanged.connect(self.applyColor)

    def applyColor(self, color):
        self.socket.value = (float(color.redF()), 
                             float(color.greenF()), 
                             float(color.blueF()),
                             float(color.alphaF()));

class IntVector2DEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket

        lay = QHBoxLayout()
        self.setLayout(lay)
        self.xspin = QSpinBox()
        self.yspin = QSpinBox()

        lay.addWidget(self.xspin)
        lay.addWidget(self.yspin)
        lay.setMargin(0)
        lay.setSpacing(0)

        if socket.value != None:
            value = socket.value
            self.xspin.setValue(value[0])
            self.yspin.setValue(value[1])
            
        self.xspin.valueChanged.connect(self.setVector)
        self.yspin.valueChanged.connect(self.setVector)

    def setVector(self):
        x = self.xspin.value()
        y = self.yspin.value()
        self.socket.value = (x, y)


class Vector2DEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket

        lay = QHBoxLayout()
        self.setLayout(lay)
        self.xspin = SpinBox(0.0)
        self.yspin = SpinBox(0.0)

        lay.addWidget(self.xspin)
        lay.addWidget(self.yspin)
        lay.setMargin(0)
        lay.setSpacing(0)

        if socket.value != None:
            value = socket.value
            self.xspin.setValue(value[0])
            self.yspin.setValue(value[1])
            
        self.xspin.valueChanged.connect(self.setVector)
        self.yspin.valueChanged.connect(self.setVector)

    def setVector(self):
        x = self.xspin.value()
        y = self.yspin.value()
        self.socket.value = (x, y)


class Vector3DEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
        self.socket = socket

        lay = QHBoxLayout()
        self.setLayout(lay)
        self.xspin = SpinBox(0.0)

        self.yspin = SpinBox(0.0)
        self.zspin = SpinBox(0.0)

        lay.addWidget(self.xspin)
        lay.addWidget(self.yspin)
        lay.addWidget(self.zspin)
        lay.setMargin(0)
        lay.setSpacing(0)

        if socket.value != None:
            value = socket.value
            self.xspin.setValue(value[0])
            self.yspin.setValue(value[1])
            self.zspin.setValue(value[2])
            
        self.xspin.valueChanged.connect(self.setVector)
        self.yspin.valueChanged.connect(self.setVector)
        self.zspin.valueChanged.connect(self.setVector)

    def setVector(self):
        x = self.xspin.value()
        y = self.yspin.value()
        z = self.zspin.value()
        self.socket.value = (x, y, z)

class Editor(QWidget):
    customWidgets = {}

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.widget = QWidget()
        lay = QVBoxLayout()
        lay.setMargin(0)
        lay.setSpacing(0)
        self.setLayout(lay)
        lay.addWidget(self.widget)
        self.widget.setLayout(QFormLayout())
        self.widget.layout().setMargin(0)
        self.widget.layout().setSpacing(0)

        self.cb = MT.attachToSignal("selectionChanged", self.updateEditor)

    def sizeHint(self):
        return QSize(300, 100)

    def updateEditor(self, node):
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
                elif s.type == "COLOR":
                    self.widget.layout().addRow(s.name, ColorEditor(s))
                elif s.type == "INTVECTOR2D":
                    self.widget.layout().addRow(s.name, IntVector2DEditor(s))
                elif s.type == "VECTOR2D":
                    self.widget.layout().addRow(s.name, Vector2DEditor(s))
                elif s.type == "VECTOR3D":
                    self.widget.layout().addRow(s.name, Vector3DEditor(s))

        type_ = node.type
        customwidget = None;
        if type_ in Editor.customWidgets:
            customwidget = Editor.customWidgets[type_]

        if customwidget is not None:
            print("adding customwidget: %s to layout" % customwidget)
            widget = customwidget(node, self)
            self.widget.layout().addRow("", widget)
    
MT.gui.registerWindow("PropertiesEditor", Editor)
