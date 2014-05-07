import PyQt4, MT, math, mttimeline
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class TimelineScrubWidget(QWidget):
    def __init__(self, timeline, parent=None):
        self.timeline = timeline
        QWidget.__init__(self, parent)
        self.value =  self.timeline.frame

        self.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)

    def setValue(self, value):
        self.value = value
        self.update()

    def mousePressEvent(self, event):
        width = self.width()
        visibleFrames = self.timeline.visibleEnd - self.timeline.visibleStart
        framewidth = float(width)/visibleFrames
        targetFrame = event.pos().x() / framewidth
        self.timeline.setFrame(math.ceil(targetFrame))

    def mouseMoveEvent(self, event):
        if event.buttons() & Qt.LeftButton:
            width = self.width()
            visibleFrames = self.timeline.visibleEnd - self.timeline.visibleStart
            framewidth = float(width)/visibleFrames
            targetFrame = event.pos().x() / framewidth
            self.timeline.setFrame(math.ceil(targetFrame))



    def sizeHint(self):
        return QSize(20, 10)

    def resizeEvent(self, event):
        QWidget.resizeEvent(self, event)

    def paintEvent(self, event):
        width = self.width()
        height = self.rect().height()
        visibleFrames = self.timeline.visibleEnd - self.timeline.visibleStart
        framewidth = float(width)/visibleFrames

        painter = QPainter(self)
        painter.setPen(Qt.NoPen)
        painter.setBrush(QBrush(QColor(115, 115, 115)))
        painter.drawRect(self.rect())

        framecolor = QColor(95, 95, 95)
        altframecolor = QColor(90, 90, 90)
        alt = False
        for i in range(1, visibleFrames+1):
            painter.setBrush(QBrush(altframecolor if alt else framecolor))
            painter.drawRect(framewidth * (i - 1), 0, framewidth * i, height)
            alt = not alt

        painter.setBrush(QColor(50, 90, 190))
        painter.setPen(Qt.NoPen)
        painter.drawRect((self.timeline.frame - 1) * framewidth, 0, framewidth, height)


class TimelineWidget(QWidget):
    def __init__(self, parent=None):
        self.start = 0
        self.end = 100
        self.frame = 1
        self.isPlaying = False
        self.visibleStart = self.start
        self.visibleEnd = self.end
        QWidget.__init__(self)
        self.setLayout(QVBoxLayout())
        self.layout().setSpacing(0)
        self.layout().setMargin(0)
        self.scrubwidget = TimelineScrubWidget(self)
        self.layout().addWidget(self.scrubwidget)

        start = QSpinBox()
        start.setRange(0, 99999999)
        end = QSpinBox()
        end.setRange(0, 99999999)
        visiblestart = QSpinBox()
        visiblestart.setRange(0, 99999999)
        visibleend = QSpinBox()
        visibleend.setRange(0, 99999999)
        self.frameBox = QSpinBox()
        self.frameBox.valueChanged.connect(self.setFrame_)
        self.frameBox.setRange(self.start, self.end)
        self.playbutton = QPushButton("Play")
        self.playbutton.clicked.connect(self.play)
        stop = QPushButton("stop")
        stop.clicked.connect(self.stop)

        start.setValue(self.start)
        end.setValue(self.end)
        self.frameBox.setValue(self.frame)
        visiblestart.setValue(self.visibleStart)
        visibleend.setValue(self.visibleEnd)


        lay = QHBoxLayout()
        self.layout().addLayout(lay)
        lay.addWidget(start)
        lay.addWidget(visiblestart)
        lay.addStretch()
        lay.addWidget(self.playbutton)
        lay.addWidget(stop)
        lay.addWidget(self.frameBox)
        lay.addStretch()
        lay.addWidget(visibleend)
        lay.addWidget(end)

        self.cb = MT.attachToSignal("frameChanged", self.setFrame)

    def play(self):
        if not self.isPlaying:
            self.playbutton.setText("Pause")
        else:
            self.playbutton.setText("Play")
        mttimeline.playpause()
        self.isPlaying = not self.isPlaying

    def stop(self):
        mttimeline.stop()
        self.isPlaying = False
        self.playbutton.setText("Play")


    def setFrame_(self, frame):
        print("spinbox changed to %d" % frame)

    def setFrame(self, frame):
        print("setting frame ... to %d" % frame)
        self.frame = frame
        mttimeline.setFrame(int(frame))
        self.scrubwidget.setValue(frame)
        self.frameBox.blockSignals(True)
        self.frameBox.setValue(frame)
        self.frameBox.blockSignals(False)


MT.gui.registerWindow("Timeline", TimelineWidget)
