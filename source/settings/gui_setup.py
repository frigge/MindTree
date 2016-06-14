import MT
graph = MT.gui.showWindow("NodeGraph")
propedit = MT.gui.showSplitWindow("PropertiesEditor", graph, "HORIZONTAL", 0.25)
timeline = MT.gui.showSplitWindow("Timeline", graph, "VERTICAL", 0.1)
console = MT.gui.showTabbedWindow("Console", timeline)

MT.gui.__dict__["viewerPosition"] = graph
MT.gui.__dict__["viewerOrientation"] = "HORIZONTAL"
