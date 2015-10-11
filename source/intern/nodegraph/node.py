import PySide, MT
from PySide.QtGui import *
from PySide.QtCore import *
from PySide.QtSvg import *

class NodeDesigner:
    width = 40
    height = 25
    view_renderer = QSvgRenderer(":node_show.svg")
    @staticmethod
    def paint_outsocket(socket, painter):
        painter.setPen(Qt.NoPen)
        painter.setBrush(socket.color)

        painter.drawRect(0, 0, socket.width, socket.height)


class NodeLink(QGraphicsItem):
    def __init__(self, start=None, end=None, temp = False):
        QGraphicsItem.__init__(self)
        self.temp = temp
        self.start = start
        self.end = end
        self.tmpEnd = end.mapToScene(end.boundingRect().center()) if end else QPointF()
        self.setZValue(self.zValue() - 1)
        self.hover = False
        self.drop = False
        self.setAcceptHoverEvents(True)
        self.setAcceptsHoverEvents(True)
        self.setAcceptDrops(True)

    def paint(self, painter, options, widget):
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setBrush(Qt.NoBrush)
        pen = QPen()
        drawArrow = False

        color = QColor(255, 255, 255)
        if self.temp:
            pen.setStyle(Qt.DotLine)
            pen.setColor(color)
        else:
            drawArrow = True
            pen.setStyle(Qt.SolidLine)
            if self.hover:
                color = QColor(150, 255, 150, 255)
                pen.setColor(color)
                pen.setWidth(4)
            elif self.drop:
                color = QColor(200, 255, 200, 255)
                pen.setColor(color)
                pen.setWidth(5)
            else:
                color = QColor(150, 255, 150, 180)
                pen.setColor(color)
                pen.setWidth(2)

        painter.setPen(pen)
        painter.drawPath(self.createPath())

        if drawArrow:
            painter.setBrush(QBrush(color))
            painter.drawPath(self.createArrow())

    def shape(self):
        stroker = QPainterPathStroker()
        stroker.setWidth(30)
        return stroker.createStroke(self.createPath())

    def dropEvent(self, event):
        nodeLabel = str(event.mimeData().text())
        event.mimeData().setText("")
        self.drop = False

        if nodeLabel == "":
            return

        _node = MT.createNode(nodeLabel)

        if _node is not None:
            self.scene().space.addNode(_node)
            scenePos = self.mapToScene(event.pos())
            nw = NodeDesigner.width
            nh = NodeDesigner.height
            _node.pos = (scenePos.x() - nw/2, scenePos.y() - nh/2)

            #link it
            #find sockets this corresponds to
            insocket = None
            for socket in self.start.data.insockets:
                if (socket.connected is not None
                        and socket.connected.node.ptr == self.end.data.ptr):
                    insocket = socket
                    break

            out = insocket.connected

            #find new matching insocket and link
            insockets = MT.getCompatibleSockets(out, node_)

            if len(insockets) == 1:
                insockets[0].connected = out
            else:
                menu = QMenu()
                for socket in insockets:
                    action = menu.addAction(socket.name)
                    def connect():
                        socket.connected = out

                    action.triggered.connect(connect)
                menu.exec_(event.screenPos())

            outsockets = list(filter(lambda o: MT.isCompatible(insocket, o), _node.outsockets))

            if len(outsockets) == 1:
                insocket.connected = outsockets[0]
            else:
                menu = QMenu()
                for socket in outsockets:
                    action = menu.addAction(socket.name)
                    def connect():
                        insocket.connected = socket

                    action.triggered.connect(connect)
                menu.exec_(event.screenPos())

    def dragEnterEvent(self, event):
        QGraphicsItem.dragEnterEvent(self, event)
        self.drop = True
        event.acceptProposedAction()
        self.update()

    def dragMoveEvent(self, event):
        QGraphicsItem.dragMoveEvent(self, event)
        self.drop = True
        event.acceptProposedAction()
        self.update()

    def dragLeaveEvent(self, event):
        self.drop = False
        self.update()

    def createArrow(self):
        TRIANGLESIZE = 10

        if self.start:
            if self.start.parentItem():
                spos = self.start.parentItem().mapToScene(self.start.parentItem().boundingRect().center())
            else:
                spos = self.start.mapToScene(self.start.boundingRect().center())
        else:
            spos = QPointF()

        if self.temp:
            epos = self.tmpEnd
        elif self.end:
            epos = self.end.mapToScene(self.end.boundingRect().center())
        else:
            epos = QPointF()

        direction = QVector2D(epos) - QVector2D(spos)
        middle = QVector2D(spos) + direction * 0.5

        perpvec = QVector2D(direction.y(), -direction.x()).normalized()

        p1vec = middle + perpvec * (-TRIANGLESIZE/2)
        p2vec = middle + perpvec * (TRIANGLESIZE/2)
        p3vec = middle + direction.normalized() * -TRIANGLESIZE

        p1 = QPointF(p1vec.x(), p1vec.y())
        p2 = QPointF(p2vec.x(), p2vec.y())
        p3 = QPointF(p3vec.x(), p3vec.y())


        path = QPainterPath(p1)
        path.lineTo(p2)
        path.lineTo(p3)
        path.lineTo(p1)
        return path


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
        self.setPos(0, -30)

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
        self.parentItem().nameBG.width = self.boundingRect().width()
        self.parentItem().nameBG.height = self.boundingRect().height()

class NodeOutSocket(QGraphicsItem):

    def __init__(self, parent):
        QGraphicsItem.__init__(self, parent)
        self.setFlag(QGraphicsItem.ItemIsMovable, False)
        self.setFlag(QGraphicsItem.ItemIsSelectable, False)
        self.width = 21
        self.height = 25
        self.setPos(parent.width, 0)
        self.out_color = QColor(50, 160, 50)
        self.over_color = QColor(137, 255, 132)
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

class NodeOptionsButton(QGraphicsSvgItem):
    def __init__(self, node):
        QGraphicsSvgItem.__init__(self, node)
        self.node = node
        self.setSharedRenderer(NodeDesigner.view_renderer)
        self.setElementId("node_view")
        self.setAcceptHoverEvents(True)
        self.setPos(-21, 0)

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
        self.setElementId("node_view_pressed")
        QGraphicsSvgItem.mousePressEvent(self, event)
        self.createMenu()

        if self.menu == None:
            self.showViewer()
        else:
            self.menu.exec_(event.screenPos())
        self.scene().setViewedNode(self.node)

    def mouseReleaseEvent(self, event):
        if self.node.viewed:
            self.setElementId("node_view_viewed")
        else:
            self.setElementId("node_view")
        self.update()
        QGraphicsSvgItem.mouseReleaseEvent(self, event)

    def hoverEnterEvent(self, event):
        if self.node.viewed:
            self.setElementId("node_view_viewed_hover")
        else:
            self.setElementId("node_view_hover")

        QGraphicsSvgItem.hoverEnterEvent(self, event)

    def hoverLeaveEvent(self, event):
        if self.node.viewed:
            self.setElementId("node_view_viewed")
        else:
            self.setElementId("node_view")
        QGraphicsSvgItem.hoverLeaveEvent(self, event)

    def boundingRect(self):
        return QRectF(0, 0, 21, 25)

class NodeNameBG(QGraphicsItem):
    def __init__(self, parent=None):
        QGraphicsItem.__init__(self, parent)
        self.width = 10
        self.height = 10
        self.setPos(0, -30)

    def paint(self, painter, options, widget):
        painter.setBrush(QBrush(QColor(20, 20, 20, 100)))
        painter.setPen(Qt.NoPen)
        painter.drawRoundedRect(0, 0, self.width, self.height, 8, 8)

    def boundingRect(self):
        return QRectF(0, 0, self.width, self.height)

class NodeItem(QGraphicsSvgItem):
    def __init__(self,
            data,
            parent=None,
            options=True,
            width=NodeDesigner.width,
            height=NodeDesigner.height):
        QGraphicsSvgItem.__init__(self, parent)
        self.data = data

        self.width = width
        self.height = height

        self.viewed = False

        self._viewSockets = False
        self._visibleSockets = []

        self.setFlag(QGraphicsItem.ItemIsMovable, True)
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)

        self.setFlag(QGraphicsItem.ItemSendsGeometryChanges, True)

        self.setAcceptHoverEvents(True)
        self.setAcceptsHoverEvents(True)
        self.setAcceptDrops(True)

        self.nameBG = NodeNameBG(self)
        self.name = NodeName(self)
        self.nameBG.width = self.name.boundingRect().width()
        self.nameBG.height = self.name.boundingRect().height()

        self.out = NodeOutSocket(self)
        if options:
            self.nodeOptions = NodeOptionsButton(self)
        self.posCB = MT.attachToBoundSignal(data, "nodePositionChanged", self.updatePositionFromData)

    def __del__(self):
        print("deleting node vis")

    def setViewed(self, viewed):
        self.viewed = viewed
        if viewed:
            self.nodeOptions.setElementId("node_view_viewed")
        else:
            self.nodeOptions.setElementId("node_view")

    def setPos(self, pos):
        for item in self.scene().items():
            if (item is not self
                and item.boundingRect().contains(pos)):
                self.data.pos = (pos.x() + NodeDesigner.width + 10, pos.y())
        super().setPos(pos)

    def updatePositionFromData(self):
        if (self.pos().x(), self.pos().y()) == self.data.pos:
            return

        p = QPoint(self.data.pos[0], self.data.pos[1])
        self.setPos(p)

    def itemChange(self, change, value):
        if change == self.ItemSelectedChange:
            self.data.selected = bool(value)
            if self.data.pos != (self.pos().x(), self.pos().y()):
                self.data.pos = (self.pos().x(), self.pos().y())

        return QGraphicsSvgItem.itemChange(self, change, value)

    def hoverEnterEvent(self, event):
        QGraphicsSvgItem.hoverEnterEvent(self, event)

    def hoverLeaveEvent(self, event):
        QGraphicsSvgItem.hoverLeaveEvent(self, event)

    def paint(self, painter, option, widget):
        painter.setRenderHint(QPainter.Antialiasing)
        if self.isSelected():
            brush = QBrush(QColor(40, 55, 70, 255), Qt.SolidPattern)
            painter.setBrush(brush)
        else:
            brush = QBrush(QColor(50, 50, 50, 255), Qt.SolidPattern)
            painter.setBrush(brush)

        painter.setPen(Qt.NoPen)
        painter.drawRect(0, 0, self.width, self.height)

    def boundingRect(self):
        return QRectF(0, 0, self.width, self.height)

    def mouseDoubleClickEvent(self, event):
        if type(self.data) == MT.pytypes.ContainerNode:
            self.scene().changeSpace(self.data.graph)

    def mouseMoveEvent(self, event):
        snap = False
        QGraphicsSvgItem.mouseMoveEvent(self, event)

        if event.buttons() & Qt.LeftButton:
            ownpos = self.scene().nodes[self.data.ptr].pos()
            for s in self.data.insockets:
                if s.connected is not None:
                    otherpos = self.scene().nodes[s.connected.node.ptr].pos()
                    if abs(ownpos.x() - otherpos.x()) < 10:
                        ownpos.setX(otherpos.x())
                        snap = True
                    if abs(ownpos.y() - otherpos.y()) < 10:
                        ownpos.setY(otherpos.y())
                        snap = True

            for os in self.data.outsockets:
                for s in os.cntdSockets:
                    otherpos = self.scene().nodes[s.node.ptr].pos()
                    if abs(ownpos.x() - otherpos.x()) < 10:
                        ownpos.setX(otherpos.x())
                        snap = True
                    if abs(ownpos.y() - otherpos.y()) < 10:
                        ownpos.setY(otherpos.y())
                        snap = True


        if snap:
            self.setPos(ownpos)

    def toggleViewSockets(self):
        self._viewSockets = not self._viewSockets
        if self._viewSockets:
            self.height += 50
            for i, insocket in enumerate(self.data.insockets):
                socket = QGraphicsEllipseItem(0, 0, 10, 10, self)
                socket.setBrush(QBrush(QColor(80, 80, 255)))
                socket.setPen(Qt.NoPen)
                socket.setPos(0, i * 10 + 5)
                self._visibleSockets.append(socket)

            for i, outsocket in enumerate(self.data.outsockets):
                socket = QGraphicsEllipseItem(0, 0, 10, 10, self)
                socket.setPos(self.width - 10, i * 10 + 5)
                socket.setBrush(QBrush(QColor(80, 80, 255)))
                socket.setPen(Qt.NoPen)
                self._visibleSockets.append(socket)

        else:
            self.height -= 50
            for socket in self._visibleSockets:
                socket.setParentItem(None)
                self.scene().removeItem(socket)

    def contextMenuEvent(self, event):
        menu = QMenu()
        action = menu.addAction("delete")
        action.triggered.connect(self.delete)
        action = menu.addAction("toggle View Sockets")
        action.triggered.connect(self.toggleViewSockets)
        menu.exec_(event.screenPos())

    def dragEnterEvent(self, event):
        QGraphicsSvgItem.dragEnterEvent(self, event)
        event.acceptProposedAction()

    def dragMoveEvent(self, event):
        QGraphicsSvgItem.dragMoveEvent(self, event)
        event.acceptProposedAction()

    def dropEvent(self, event):
        if self.scene().tmpLink:
            out = self.scene().tmpLink.outsocket
            self.scene().removeTmpLink()

            compSockets = MT.getCompatibleSockets(out, self.data)

            if len(compSockets) > 1:
                menu = QMenu()

                def action_triggered_cb(s):
                    def action_triggered():
                        s.connected = out
                    return action_triggered

                for name, s in compSockets.items():
                    action = menu.addAction(name)
                    action.triggered.connect(action_triggered_cb(s))
                menu.exec_(event.screenPos())

            elif len(compSockets) == 1:
                s = compSockets.values()[0]
                s.connected = out

    def delete(self):
        space = self.data.space
        space.removeNode(self.data)
