import MT
from . import mttimeline
MT.__dict__["timeline"] = mttimeline
from .timelinewidget import *

import PySide
from PySide.QtCore import *
from PySide.QtGui import *
from PySide.QtSvg import *
import nodegraph

class TimelineNodeVis(nodegraph.node.NodeItem):
    def __init__(self, data, parent=None):
        nodegraph.node.NodeItem.__init__(self, 
                data, 
                parent, 
                options=False, 
                width=40)
        self._renderer = QSvgRenderer(":node_show.svg")
        self.setSharedRenderer(self._renderer)
        self.setElementId("node_timeline")

    def paint(self, painter, options, widget):
        QGraphicsSvgItem.paint(self, painter, options, widget)


MT.customNodeVisualizations["TIMELINE"] = TimelineNodeVis
