import MT, PySide, node, signaldecorator
from PySide.QtGui import *
from PySide.QtOpenGL import *
from PySide.QtCore import *


class NodeSpace(QGraphicsScene):
    def __init__(self, space):
        self.space = space
        QGraphicsScene.__init__(self)
        self.setItemIndexMethod(QGraphicsScene.NoIndex)
        self.tmpLink = None
        self.createNetwork()
        self.setSceneRect(0, 0, 500, 500)
        self.nodes = {}
        setattr(MT, "graph", self)
        self.cb = MT.attachToSignal("createLink", self.drawLink)

    def createNetwork(self):
        for n in self.space:
            addNode(n)
            for s in n.insockets:
                if s.cntdsocket: self.drawLink(s)

    def addNode(self, n):
        print(n.name + " added to the scene")
        item = node.NodeItem(n)
        self.addItem(item)
        self.nodes[n.name] =  item

    def removeNode(self, n):
        item = self.nodes[n.name]
        if item:
            self.removeItem(item)

    def drawLink(self, insocket):
        link = node.NodeLink()
        link.start = self.nodes[insocket.node.name]
        link.end = self.nodes[insocket.connected.node.name]
        self.addItem(link)

    def showTmpLink(self, socket):
        self.tmpLink = node.NodeLink(socket, socket.parentItem(), temp = True)
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
        if self.tmpLink != None:
            self.removeItem(self.tmpLink)
        self.tmpLink = None
        QGraphicsScene.mouseReleaseEvent(self, event)


class NodeGraph(QGraphicsView):
    
    def __init__(self):
        QGraphicsView.__init__(self)

        self.scene = NodeSpace(MT.project.root)
        self.setScene(self.scene)
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
        self.setViewport(QGLWidget())
        self.setTransformationAnchor(QGraphicsView.AnchorUnderMouse)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setUpdatesEnabled(True)
        self.setDragMode(QGraphicsView.RubberBandDrag)

        self.cb = MT.attachToSignal("addNode", self.addNode)
        self.cb2 = MT.attachToSignal("removeNode", self.removeNode)

        self.space = MT.project.root

    def addNode(self, n):
        self.scene.addNode(n)

    def removeNode(self, n):
        self.scene.removeNode(n)


    def __del__(self):
        #MT.nodegraphs.remove(self)
        pass

    def drawBackground(self, painter, rect):
        painter.setBrush(QBrush(QColor(30, 30, 30)))
        painter.drawRect(rect)

    def mousePressEvent(self, event):
        QGraphicsView.mousePressEvent(self, event)


class NodeBrowser(QToolBox):
    def __init__(self, graph):
        QToolBox.__init__(self)
        self.resize(50, 50)
        self.groups = {}
        self.graph = graph

    def sizeHint(self):
        return QSize(50, 50)

    def insertGroup(self, cats):
        if ".".join(cats) in self.groups:
            return
        
        catname = []
        currbox = self

        for c in cats:
            catname.append(c)
            if ".".join(catname) in self.groups:
                return
            currwidget = QWidget()
            currwidget.setLayout(QFormLayout())
            currbox.addItem(currwidget, c)
            self.groups[".".join(catname)] = currwidget
            if ".".join(catname) != ".".join(cats):
                box = QToolBox()
                currbox = box
                currwidget.layout().addWidget(box)

    def initList(self):
        nodeTypes = MT.getRegisteredNodes()
        for nt in nodeTypes:
            cats = nt.split(".")
            if len(cats) > 1:
                button = QPushButton(cats[-1])
                self.insertGroup(cats[:-1])
                self.groups[".".join(cats[:-1])].layout().addWidget(button)
                button.clicked.connect(self.createNode(nt))
            else:
                print("specify a category for: " + nt)

    def createNode(self, nodeType):
        def nodeSlot():
            node = MT.createNode(nodeType)
            self.graph.space.addNode(node)
        return nodeSlot


class NodeGraphWidget(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        lay = QHBoxLayout()
        self.setLayout(lay)
        #self.toolbar = QToolBar()
        #self.toolbar.setOrientation(core.Qt.Vertical)
        #lay.addWidget(self.toolbar)
        self.graph = NodeGraph()
        self.nodeBrowser = NodeBrowser(self.graph)
        self.nodeBrowser.initList()
        self.nodeBrowser.resize(100, self.nodeBrowser.height())
        splitter = QSplitter(Qt.Horizontal)
        lay.addWidget(splitter)
        splitter.addWidget(self.nodeBrowser)
        splitter.setStretchFactor(0, .25)
        splitter.setStretchFactor(1, 1)
        splitter.setOpaqueResize(False)
        splitter.addWidget(self.graph)
        lay.setSpacing(0)
        #lay.setContentsMargin(0, 0, 0, 0)

MT.gui.registerWindow("NodeGraph", NodeGraphWidget)
