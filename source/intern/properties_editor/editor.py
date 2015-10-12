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
        self.layout().setSpacing(0)
        self.layout().setMargin(0)
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
        self.layout().setSpacing(0)
        self.layout().setMargin(0)
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
        self.layout().setSpacing(0)
        self.layout().setMargin(0)

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
        self.layout().setSpacing(0)
        self.layout().setMargin(0)
        if socket.value != None:
            self.widget.setValue(socket.value)
        self.widget.valueChanged.connect(lambda x: setattr(self.socket, "value", x))

class BoolEditor(QWidget):
    def __init__(self, socket):
        QWidget.__init__(self)
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
        self.layout().setSpacing(0)
        self.layout().setMargin(0)
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
        self.layout().setSpacing(0)
        self.layout().setMargin(0)
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
        self.xspin.setRange(-100000, 100000)
        self.yspin.setRange(-100000, 100000)

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

class EditorRow(QWidget):
    def __init__(self, socket, parent=None):
        super().__init__(parent)
        self.setLayout(QHBoxLayout())
        self.layout().setMargin(0)
        self.layout().setSpacing(0)
        self.socket = socket
        plusButton = QPushButton("+")
        plusButton.pressed.connect(self.expand)
        self.layout().addWidget(plusButton)
        self.widget = Editor.editors[socket.type](socket)
        self.layout().addWidget(self.widget)
        self.expanded = False

    def expand(self):
        if not self.expanded:
            self.layout().removeWidget(self.widget)
            for n in self.socket.childNodes:
                self.view = EditorView()
                self.layout().addWidget(self.view)
                self.view.updateEditor(n)
            self.expanded = True
        else:
            self.layout().removeWidget(self.view)
            self.layout().addWidget(self.widget)
            self.expanded = False
        self.parent().update()

class EditorView(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setLayout(QFormLayout())
        self.layout().setMargin(0)
        self.layout().setSpacing(0)

    def addEditor(self, s):
        if s.type in Editor.editors:
            self.layout().addRow(s.name, EditorRow(s))

    def updateEditor(self, node):
        for s in node.insockets:
            if s.connected == None:
                self.addEditor(s)

        type_ = node.type
        customwidget = None;
        if type_ in Editor.customWidgets:
            customwidget = Editor.customWidgets[type_]

        if customwidget is not None:
            print("adding customwidget: %s to layout" % customwidget)
            widget = customwidget(node, self)
            self.layout().addRow("", widget)

class Editor(QWidget):
    customWidgets = {}

    editors = {
        "STRING" : StringEditor,
        "DIRECTORY" : DirEditor,
        "FLOAT" : FloatEditor,
        "INTEGER" : IntEditor,
        "BOOLEAN" : BoolEditor,
        "COLOR" : ColorEditor,
        "INTVECTOR2D" : IntVector2DEditor,
        "VECTOR2D" : Vector2DEditor,
        "VECTOR3D" : Vector3DEditor
    }

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.widget = QScrollArea()
        self.editor = EditorView()
        lay = QVBoxLayout()
        lay.setMargin(0)
        lay.setSpacing(0)
        self.setLayout(lay)
        lay.addWidget(self.widget)
        self.widget.setWidget(self.editor)

        self.cb = MT.attachToSignal("selectionChanged", self.updateEditor)

    def updateEditor(self, node):
        self.editor = EditorView()
        self.editor.updateEditor(node);
        self.widget.setWidget(self.editor)
        self.editor.show()

MT.gui.registerWindow("PropertiesEditor", Editor)
