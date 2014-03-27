import PySide, MT
from PySide.QtGui import *
from PySide.QtCore import *

class NodeDesigner:
    @staticmethod
    def paint_outsocket(socket, painter):
        painter.setBrush(socket.color)

        if socket.hover:
            pen = QPen()
            pen.setStyle(Qt.SolidLine)
            pen.setWidth(3)
            pen.setColor(QColor(200, 255, 200, 255))
            painter.setPen(pen)
        else:
            painter.setPen(Qt.NoPen)

        painter.drawEllipse(0, 0, socket.width, socket.height)


class NodeLink(QGraphicsItem):
    def __init__(self, start=None, end=None, temp = False):
        QGraphicsItem.__init__(self)
        self.temp = temp
        self.start = start
        self.end = end
        self.tmpEnd = end.mapToScene(end.boundingRect().center()) if end else QPointF()
        self.setZValue(self.zValue() - 1)
        self.hover = False
        self.setAcceptHoverEvents(True)

    def paint(self, painter, options, widget):
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setBrush(Qt.NoBrush)
        pen = QPen()
        if self.temp:
            pen.setStyle(Qt.DotLine)
            pen.setColor(QColor(255, 255, 255))
        else:
            pen.setStyle(Qt.SolidLine)
            if self.hover:
                pen.setColor(QColor(255, 255, 255, 255))
                pen.setWidth(3)
            else:
                pen.setColor(QColor(255, 255, 255, 180))
                pen.setWidth(1.5)
        painter.setPen(pen)
        painter.drawPath(self.createPath())

    def shape(self):
        stroker = QPainterPathStroker()
        stroker.setWidth(20)
        return stroker.createStroke(self.createPath())

    def createPath(self):
        if self.start:
            if self.start.parentItem():
                spos = self.start.parentItem().mapToScene(self.start.parentItem().boundingRect().center())
            else:
                spos = self.start.mapToScene(self.start.boundingRect().center())
        else:
            spos = QPointF()
        path = QPainterPath(spos)
        if self.temp:
            epos = self.tmpEnd
        elif self.end:
            epos = self.end.mapToScene(self.end.boundingRect().center())
        else:
            epos = QPointF()
        path.lineTo(epos)
        return path

    def hoverEnterEvent(self, event):
        self.hover = True
        QGraphicsItem.hoverEnterEvent(self, event)

    def hoverLeaveEvent(self, event):
        self.hover = False
        QGraphicsItem.hoverLeaveEvent(self, event)

    def boundingRect(self):
        if self.start:
            spos = self.start.mapToScene(self.start.boundingRect().center())
        else:
            spos = QPointF
        if self.end:
            epos = self.end.mapToScene(self.end.boundingRect().center())
        else:
            epos = self.tmpEnd
        x = spos.x() if spos.x() < epos.x() else epos.x()
        y = spos.y() if spos.y() < epos.y() else epos.y()
        width = abs(spos.x() - epos.x())
        height = abs(spos.y() - epos.y())
        return QRectF(x, y, width, height)


class NodeName(QGraphicsTextItem):
    def __init__(self, parent):
        QGraphicsTextItem.__init__(self, parent.data.name, parent)

        self.setFlag(QGraphicsItem.ItemIsMovable, False)
        self.setFlag(QGraphicsItem.ItemIsSelectable, False)
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setDefaultTextColor(QColor(255, 255, 255))
        self.setPos(0, -20)

    def mouseDoubleClickEvent(self, event):
        if self.textInteractionFlags() == Qt.NoTextInteraction:
            self.setTextInteractionFlags(Qt.TextEditorInteraction)
        QGraphicsTextItem.mouseDoubleClickEvent(self, event)

    def focusOutEvent(self, event):
        QGraphicsTextItem.focusOutEvent(self, event)

        if self.toPlainText() != "":
            text = str(self.toPlainText())
            self.parentItem().data.name = text
        else:
            self.setPlainText(sef.parentItem().data.name)

        self.setTextInteractionFlags(Qt.NoTextInteraction)
        self.textCursor().clearSelection()

class NodeOutSocket(QGraphicsItem):

    def __init__(self, parent):
        QGraphicsItem.__init__(self, parent)
        self.setFlag(QGraphicsItem.ItemIsMovable, False)
        self.setFlag(QGraphicsItem.ItemIsSelectable, False)
        self.width = 15
        self.height = 15
        self.setPos((parent.width - self.width) / 2, parent.height + 5)
        self.out_color = QColor(140, 140, 140)
        self.over_color = QColor(200, 200, 200)
        self.color = self.out_color
        self.setAcceptHoverEvents(True)

        self.hover = False

    def paint(self, painter, option, widget):
        painter.setRenderHint(QPainter.Antialiasing)
        NodeDesigner.paint_outsocket(self, painter)

    def boundingRect(self):
        return QRectF(0, 0, self.width, self.height)

    def hoverEnterEvent(self, event):
        self.color = self.over_color
        QGraphicsItem.hoverEnterEvent(self, event)
        self.hover = True

    def hoverLeaveEvent(self, event):
        self.color = self.out_color
        QGraphicsItem.hoverLeaveEvent(self, event)
        self.hover = False

    def mousePressEvent(self, event):
        if event.button() != Qt.LeftButton:
            QGraphicsItem.mousePressEvent(self, event)
            return

        self.dragStartPos = event.screenPos()
        self.scene().showTmpLink(self)
        if len(self.parentItem().data.outsockets) > 1:
            menu = QMenu()
            for s in self.parentItem().data.outsockets:
                action = menu.addAction(s.name)
                def triggered():
                    self.scene().tmpLink.outsocket = s
                action.triggered.connect(triggered)
        else:
            self.scene().tmpLink.outsocket = self.parentItem().data.outsockets[0]


    def mouseMoveEvent(self, event):
        if (event.pos() - self.dragStartPos).manhattanLength() < QApplication.startDragDistance():
            QGraphicsItem.mouseMoveEvent(self, event)
            return
        QGraphicsItem.mouseMoveEvent(self, event)

        drag = QDrag(event.widget())
        drag.setMimeData(QMimeData())
        drag.exec_()

class NodeOptionsButton(QGraphicsItem):
    def __init__(self, node):
        QGraphicsItem.__init__(self, node)
        self.node = node
        self.createMenu()
        self.visible = False

    def createMenu(self):
        viewers = MT.gui.getViewers()
        sockets = [s for s in self.node.data.outsockets if s.type in viewers.keys()]
        if len(sockets) == 1 and len(viewers[sockets[0].type]) == 1:
            self.menu = None
            def showViewer():
                MT.gui.showViewer(sockets[0], 0)
            self.showViewer = showViewer
        else:
            self.menu = QMenu()
            for s in sockets:
                for i, viewer in enumerate(viewers[s.type]):
                    def showViewer(i):
                        def _():
                            MT.gui.showViewer(s, i)
                        return _
                    act = self.menu.addAction(viewer)
                    act.triggered.connect(showViewer(i))

    def mousePressEvent(self, event):
        QGraphicsItem.mousePressEvent(self, event)
        if self.menu == None:
            self.showViewer()
        else:
            self.menu.exec_(event.screenPos())

    def boundingRect(self):
        return QRectF(0, 0, 10, 10)

    def paint(self, painter, option, widget):
        if not self.visible:
            return

        painter.setBrush(QColor(255, 255, 255))
        painter.drawRect(0, 0, 10, 10)

    def show(self):
        self.visible = True

    def hide(self):
        self.visible = False


class NodeItem(QGraphicsItem):

    def __init__(self, data, parent=None):
        QGraphicsItem.__init__(self, parent)
        self.data = data

        self.setFlag(QGraphicsItem.ItemIsMovable, True)
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)

        self.setFlag(QGraphicsItem.ItemSendsGeometryChanges, True)

        self.setAcceptHoverEvents(True)
        self.setAcceptsHoverEvents(True)
        self.setAcceptDrops(True)

        self.name = NodeName(self)

        self.width = 60
        self.height = 25
        self.out = NodeOutSocket(self)
        self.nodeOptions = NodeOptionsButton(self)
        self.posCB = MT.attachToBoundSignal(data, "nodePositionChanged", self.updatePositionFromData)

    def __del__(self):
        print("deleting node vis")

    def updatePositionFromData(self):
        if (self.pos().x(), self.pos().y()) != self.data.pos:
            self.setPos(*self.data.pos)

    def itemChange(self, change, value):
        if change == self.ItemSelectedChange:
            self.data.selected = bool(value)
            if self.data.pos != (self.pos().x(), self.pos().y()):
                self.data.pos = (self.pos().x(), self.pos().y())

        return QGraphicsItem.itemChange(self, change, value)

    def showOptionsButton(self):
        self.nodeOptions.show()

    def hideOptionsButton(self):
        self.nodeOptions.hide()

    def hoverEnterEvent(self, event):
        QGraphicsItem.hoverEnterEvent(self, event)
        self.showOptionsButton()

    def hoverLeaveEvent(self, event):
        QGraphicsItem.hoverLeaveEvent(self, event)
        self.hideOptionsButton()

    def paint(self, painter, option, widget):
        painter.setRenderHint(QPainter.Antialiasing)
        outline = QPen()
        if self.isSelected():
            outline.setColor(QColor(180, 120, 50, 255))
            outline.setWidth(3)
        else:
            outline.setColor(QColor(50, 50, 50, 255))
            outline.setWidth(1)

        brush = QBrush(QColor(50, 50, 50, 255), Qt.SolidPattern)
        painter.setPen(outline)
        painter.setBrush(brush)
        painter.drawRoundedRect(0, 0, self.width, self.height, 1, 1)

    def boundingRect(self):
        return QRectF(0, 0, self.width, self.height)

    def contextMenuEvent(self, event):
        menu = QMenu()
        action = menu.addAction("delete")
        action.triggered.connect(self.delete)
        menu.exec_(event.screenPos())

    def dragEnterEvent(self, event):
        QGraphicsItem.dragEnterEvent(self, event)
        event.acceptProposedAction()

    def dragMoveEvent(self, event):
        QGraphicsItem.dragMoveEvent(self, event)
        event.acceptProposedAction()

    def dropEvent(self, event):
        if self.scene().tmpLink:
            out = self.scene().tmpLink.outsocket
            self.scene().removeTmpLink()
            if len(self.data.insockets) > 1:
                menu = QMenu()

                def action_triggered_cb(s):
                    def action_triggered():
                        print("connect {0}.{1} to {2}.{3}".format(s.node.name, s.name, out.node.name, out.name))
                        s.connected = out
                    return action_triggered

                for s in self.data.insockets:
                    action = menu.addAction(s.name)
                    action.triggered.connect(action_triggered_cb(s))
                menu.exec_(event.screenPos())
            elif len(self.data.insockets) == 1:
                self.data.insockets[0].connected = out

    def delete(self):
        space = self.data.space
        space.removeNode(self.data)

