import MT
from . import node

from PyQt5.QtWidgets import *
from PyQt5.QtOpenGL import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *

from .nodebrowser import NodeBrowser

MT.__dict__["customNodeVisualizations"] = {}

class NodeSpace(QGraphicsScene):
    def __init__(self, space):
        self.space = space
        QGraphicsScene.__init__(self)
        self.setItemIndexMethod(QGraphicsScene.NoIndex)
        self.tmpLink = None
        self.createNetwork()

        self.nodes = {}
        self.links = {}

        setattr(MT, "graph", self)
        self.cb = MT.attachToSignal("createLink", self.drawLink)
        self.setSceneRect(-5000, -5000, 10000, 10000)

    def drawForeground(self, painter, rect):
        items = self.itemsBoundingRect()
        combinedrect = rect | items
        size = 100
        margin = 2
        if combinedrect == rect:
            return

        painter.setPen(Qt.NoPen)

        #draw bound rectangle
        pos = QPointF(rect.bottomRight() - QPointF(size + margin, size + margin))

        cwidth = combinedrect.width()
        cheight = combinedrect.height()

        rpos = QPointF(((rect.x() - combinedrect.x()) / cwidth),
                       ((rect.y() - combinedrect.y()) / cheight)) * size
        rpos += pos

        painter.setBrush(QBrush(QColor(50, 50, 50)))
        painter.drawRect(QRectF(pos, QSize(size, size)))

        #draw viewport rect
        width = (rect.width() / cwidth) * size
        height = (rect.height() / cheight) * size

        painter.setBrush(QBrush(QColor(80, 80, 80)))
        painter.drawRect(QRectF(rpos, QSize(width, height)))

        super().drawForeground(painter, rect)

        width_scale = width / cwidth
        height_scale = height / cheight

        nodewidth = max(4, node.NodeDesigner.width * width_scale)
        nodeheight = max(4, node.NodeDesigner.height * height_scale)
        nodesize = QSize(nodewidth, nodeheight)

        painter.setBrush(QBrush(QColor(50, 160, 50)))

        for item in self.items():
            if not type(item) is node.NodeItem:
                continue

            p = item.pos()
            rp = QPointF(((p.x() - combinedrect.x()) / cwidth),
                        ((p.y() - combinedrect.y()) / cheight)) * size
            rp += pos

            r = QRect(rp.toPoint(), nodesize)
            painter.drawRect(r)

    def clearCallbacks(self):
        del self.cb

    def changeSpace(self, space):
        self.space = space
        for node in self.nodes.values():
            self.removeItem(node)

        for link in self.links.values():
            self.removeItem(link)

        self.nodes = {}
        self.links = {}

        self.createNetwork()
        view = self.views()[0]
        view.widget.populateToolBar()

    def createNetwork(self):
        for n in self.space:
            self.addNode(n)

        for n in self.space:
            for s in n.insockets:
                if s.connected:
                    self.drawLink(s)

    def updateSceneRect(self):
        view = self.views()[0]
        ul = view.mapToScene(view.rect().topLeft())
        br = view.mapToScene(view.rect().bottomRight())
        viewRect = QRectF(ul, br)

        itemsrect = self.itemsBoundingRect()
        combinedrect = itemsrect | viewRect
        self.setSceneRect(combinedrect)

    def addNode(self, n):
        if n.type in MT.customNodeVisualizations:
            item = MT.customNodeVisualizations[n.type](n)
        else:
            item = node.NodeItem(n)

        pos = QPoint(n.pos[0], n.pos[1])

        self.addItem(item)
        item.setPos(pos)
        self.cacheNodeGeometry(n, item)
        return item

    def cacheNodeGeometry(self, node, item):
        self.nodes[node.ptr] = item
        for socket in node.insockets:
            for childNode in socket.childNodes:
                self.cacheNodeGeometry(childNode, item)

    def setViewedNode(self, node):
        for n in self.nodes.values():
            n.setViewed(False)
        node.setViewed(True)

    def removeNode(self, n):
        item = self.nodes[n.ptr]
        if item:
            self.removeItem(item)

            #remove all links
            for insocket in n.insockets:
                self.removeLink(insocket)

            for outsocket in n.outsockets:
                for insocket in outsocket.cntdSockets:
                    self.removeLink(insocket)

    def drawLink(self, insocket):
        link = node.NodeLink()
        link.start = self.nodes[insocket.node.ptr]
        link.end = self.nodes[insocket.connected.node.ptr]

        self.removeLink(insocket)

        self.links[insocket] = link
        self.addItem(link)

        for child in insocket.childNodes:
            for s in child.insockets:
                if s.connected:
                    self.drawLink(s)

    def removeLink(self, insocket):
        toremove = []
        for s in self.links.keys():
            if s == insocket:
                toremove.append(s)

        for s in toremove:
            self.removeItem(self.links.pop(s))

    def showTmpLink(self, socket):
        self.tmpLink = node.NodeLink(socket, socket.parentItem(), temp=True)
        self.addItem(self.tmpLink)

    def removeTmpLink(self):
        self.removeItem(self.tmpLink)
        self.tmpLink = None

    def mouseMoveEvent(self, event):
        if self.tmpLink:
            self.tmpLink.tmpEnd = event.scenePos()
            self.tmpLink.update()
        QGraphicsScene.mouseMoveEvent(self, event)

    def dragMoveEvent(self, event):
        if self.tmpLink:
            self.tmpLink.tmpEnd = event.scenePos()
            self.tmpLink.update()
        QGraphicsScene.dragMoveEvent(self, event)

    def mouseReleaseEvent(self, event):
        if self.tmpLink is not None:
            self.removeItem(self.tmpLink)
        self.tmpLink = None
        QGraphicsScene.mouseReleaseEvent(self, event)


class NodeGraph(QGraphicsView):

    def __init__(self, widget):
        QGraphicsView.__init__(self)

        self.setScene(NodeSpace(MT.project.root))
        self.widget = widget
        #QGL.setPreferredPaintEngine(QPaintEngine.OpenGL2)
        #glFmt = QGLFormat.defaultFormat()
        #glFmt.setSwapInterval(0)
        #glFmt.setRgba(True)
        #glFmt.setDoubleBuffer(True)
        #glFmt.setOverlay(False)
        #QGLFormat.setDefaultFormat(glFmt)
        #glFmt.setSampleBuffers(True)
        #glWidget = QGLWidget(glFmt)
        #glWidget.setAutoFillBackground(False)
        self.setViewportUpdateMode(QGraphicsView.FullViewportUpdate)
        #self.setViewport(glWidget)
        #self.setViewport(QGLWidget())
        self.setTransformationAnchor(QGraphicsView.AnchorUnderMouse)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setUpdatesEnabled(True)
        self.setDragMode(QGraphicsView.RubberBandDrag)

        self.cb = MT.attachToSignal("addNode", self.addNode)
        self.cb2 = MT.attachToSignal("removeNode", self.removeNode)
        self.cb3 = MT.attachToSignal("newProject", self.newGraph)

        self.space = MT.project.root
        self.setAcceptDrops(True)
        self.dragPos = None

        containerShortcut = QShortcut(self)
        deleteShortcut = QShortcut(self)
        browserShortcut = QShortcut(self)
        containerShortcut.setContext(Qt.WidgetShortcut)
        deleteShortcut.setContext(Qt.WidgetShortcut)
        browserShortcut.setContext(Qt.WidgetShortcut)
        containerShortcut.setKey("c")
        browserShortcut.setKey(" ")
        deleteShortcut.setKey(Qt.Key_Delete)
        containerShortcut.activated.connect(self.createContainer)
        deleteShortcut.activated.connect(self.deleteNode)
        browserShortcut.activated.connect(self.showNodeBrowser)
        self.nodeBrowser = None

    def showNodeBrowser(self):
        if self.nodeBrowser is None:
            self.nodeBrowser = NodeBrowser(self)

        self.nodeBrowser.show()
        self.nodeBrowser.move(QCursor.pos())

    def newGraph(self, project):
        self.scene().changeSpace(project.root)

    def deleteNode(self):
        for item in self.scene().selectedItems():
            self.scene().space.removeNode(item.data)

    def createContainer(self):
        items = self.scene().selectedItems()
        if len(items) == 0:
            return

        nodes = [n.data for n in items]

        MT.createContainer(nodes)


    def resizeEvent(self, event):
        QGraphicsView.resizeEvent(self, event)

    def dragEnterEvent(self, event):
        QGraphicsView.dragEnterEvent(self, event)
        event.acceptProposedAction()

    def dragMoveEvent(self, event):
        QGraphicsView.dragMoveEvent(self, event)
        event.acceptProposedAction()

    def dropEvent(self, event):
        QGraphicsView.dropEvent(self, event)
        nodeLabel = str(event.mimeData().text())
        if nodeLabel == "":
            return

        scenePos = self.mapToScene(event.pos())
        nw = node.NodeDesigner.width
        nh = node.NodeDesigner.height
        pos = (scenePos.x() - nw/2, scenePos.y() - nh/2)
        self.dropNode(nodeLabel, pos)

    def dropNode(self, label, pos):
        _node = MT.createNode(label)
        if _node is not None:
            self.scene().space.addNode(_node)
            _node.pos = pos
        return _node

    def addNode(self, n):
        if n.space.ptr != self.scene().space.ptr:
            return

        item = self.scene().addNode(n)
        for it in self.scene().items():
            it.setSelected(False)
        item.setSelected(True)

    def removeNode(self, n):
        self.scene().removeNode(n)

    def __del__(self):
        #MT.nodegraphs.remove(self)
        pass

    def drawBackground(self, painter, rect):
        painter.setBrush(QBrush(QColor(70, 70, 70)))
        painter.drawRect(self.sceneRect())

    def mousePressEvent(self, event):
        QGraphicsView.mousePressEvent(self, event)
        self.dragPos = event.pos()

    def getSceneViewRect(self):
        tl = self.mapToScene(self.rect().topLeft())
        br = self.mapToScene(self.rect().bottomRight())
        return QRectF(tl, br)

    def mouseMoveEvent(self, event):
        #handle dragging manually
        if (event.buttons() & Qt.MiddleButton
                or (event.modifiers() & Qt.AltModifier 
                    and event.buttons() & Qt.LeftButton)):

            self.setCursor(Qt.ClosedHandCursor)

            distance = event.pos() - self.dragPos
            rect = self.getSceneViewRect()

            rect.translate(-distance.x(), -distance.y())

            self.centerOn(rect.center())
            self.dragPos = event.pos()
        else:
            QGraphicsView.mouseMoveEvent(self, event)

    def mouseReleaseEvent(self, event):
        QGraphicsView.mouseReleaseEvent(self, event)
        self.setCursor(Qt.ArrowCursor)
        self.drag = False


class NodeGraphWidget(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        mainlay = QVBoxLayout()
        self.graphtool = QToolBar()
        mainlay.addWidget(self.graphtool)
        lay = QHBoxLayout()
        mainlay.addLayout(lay)
        self.setLayout(mainlay)
        #self.toolbar = QToolBar()
        #self.toolbar.setOrientation(core.Qt.Vertical)
        #lay.addWidget(self.toolbar)
        self.graph = NodeGraph(self)
        #self.nodeBrowser = NodeBrowser(self.graph)
        #self.nodeBrowser.initList()
        #self.nodeBrowser.resize(100, self.nodeBrowser.height())
        splitter = QSplitter(Qt.Horizontal)
        lay.addWidget(splitter)
        #splitter.addWidget(self.nodeBrowser)
        splitter.setStretchFactor(0, .25)
        splitter.setStretchFactor(1, 1)
        splitter.setOpaqueResize(False)
        splitter.addWidget(self.graph)
        lay.setSpacing(0)
        #lay.setContentsMargin(0, 0, 0, 0)

        self.populateToolBar()

    def populateToolBar(self):
        currGraph = self.graph.scene().space
        graphList = []
        self.graphtool.clear()
        while(True):
            graphList.append(currGraph)
            if not hasattr(currGraph, "parent"):
                break
            currGraph = currGraph.parent

        graphList.reverse()

        def getChangeFunc(graph):
            def change():
                self.graph.scene().changeSpace(graph)

            return change

        for graph in graphList:
            graphAction = self.graphtool.addAction(graph.name + "/")
            graphAction.triggered.connect(getChangeFunc(graph))

MT.gui.registerWindow("NodeGraph", NodeGraphWidget)
