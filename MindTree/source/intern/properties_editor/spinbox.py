import PyQt4, sys, math
from PyQt4.QtGui import *
from PyQt4.QtCore import *

class ScrollWidget(QWidget):
    def __init__(self, parent, startPos):
        QWidget.__init__(self, parent, Qt.Popup)
        self.selectedCell = 3
        self.startPos = startPos

    def sizeHint(self):
        return QSize(70, 150)

    def mouseReleaseEvent(self, event):
        self.hide()

    def mouseMoveEvent(self, event):
        QWidget.mouseMoveEvent(self, event)
        mpos = event.pos()
        cellheight = self.rect().height()/7

        self.selectedCell = mpos.y() / cellheight
        currpos = self.mapToGlobal(event.pos())
        delta = currpos.x() - self.startPos.x()
        power = self.selectedCell - 3
        step = math.pow(10, power)

        delta *= step

        oldval = self.parent().parent()._value
        self.parent().parent().setValue(oldval + delta)
        self.startPos = currpos

        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setPen(Qt.NoPen)
        painter.setBrush(QBrush(QColor(Qt.black)))
        painter.drawRect(self.rect())

        selcol = QColor(75, 75, 75)
        cellheight = self.rect().height()/7

        for i in range(7):
            power = i - 3
            step = math.pow(10, power)

            if i == self.selectedCell:
                painter.setBrush(QBrush(QColor(selcol)))
            else:
                if i % 2 == 0:
                    col = QColor(95, 95, 95)
                else:
                    col = QColor(90, 90, 90)
                painter.setBrush(QBrush(col))

            painter.setPen(Qt.NoPen)
            painter.drawRect(0, cellheight * i, self.rect().width(), cellheight)
            value = self.parent().parent()._value
            text = str(value) if power == 0 else str(step)

            if i == self.selectedCell:
                pen = QPen()
                pen.setWidth(2)
                pen.setColor(QColor(Qt.black))
                painter.setPen(pen)
            else:
                pen = QPen()
                pen.setWidth(2)
                pen.setColor(QColor(Qt.white))
                painter.setPen(pen)

            painter.drawText(0, cellheight * i, self.rect().width(), self.rect().height(), Qt.AlignLeft, text)

class ScrollTrigger(QWidget):
    def __init__(self, parent):
        QWidget.__init__(self, parent)
        self.scrollwidget = None
        self.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)

    def sizeHint(self):
        return QSize(20, 20)

    def mousePressEvent(self, event):
        self.scrollwidget = ScrollWidget(self, self.mapToGlobal(event.pos()))
        self.scrollwidget.show()
        dx = 0
        dy = self.scrollwidget.height() / 2
        self.scrollwidget.move(event.globalPos() - QPoint(dx, dy))

    def mouseReleaseEvent(self, event):
        if self.scrollwidget is not None:
            self.scrollwidget.hide()
            self.scrollwidget = None

    def paintEvent(self, event):
        painter = QPainter(self)

        painter.setPen(Qt.NoPen)
        painter.setBrush(QColor(90, 90, 90))
        painter.drawRect(self.rect())

        MARGIN = 4
        middle = self.rect().center()
        top = self.rect().top()
        bottom = self.rect().bottom()

        right = self.rect().width()

        painter.setBrush(QColor(Qt.white))
        path = QPainterPath(QPoint(middle.x(), MARGIN))
        path.lineTo(MARGIN, middle.y() - MARGIN)
        path.lineTo(right - MARGIN, middle.y() - MARGIN)

        painter.drawPath(path)

class SpinBox(QWidget):
    valueChanged = pyqtSignal(float)

    def __init__(self, default):
        QWidget.__init__(self)
        lay = QHBoxLayout()
        lay.setSpacing(0)
        lay.setMargin(0)
        self._value = default
        self._type = type(default)

        self.setLayout(lay)
        self.lineedit = QLineEdit()
        self.scrollbutton = ScrollTrigger(self)

        self.lineedit.setText(str(default))

        if self._type == int:
            self.lineedit.setValidator(QDoubleValidator())
        elif self._type == float:
            self.lineedit.setValidator(QIntValidator())

        self.lineedit.setAlignment(Qt.AlignRight)
        lay.addWidget(self.scrollbutton)
        lay.addWidget(self.lineedit)
        self.lineedit.textChanged.connect(self._updateValue)

    def _updateValue(self, txt):
        self.setValue(self._type(txt))

    def setValue(self, val):
        if val == self._value: return

        self._value = val
        self.lineedit.setText(str(val))
        self.valueChanged.emit(val)

    def value(self):
        return self._value

if __name__ == "__main__":
    app = QApplication(sys.argv)
    box = SpinBox(0.0)
    box.show()

    app.exec_()
