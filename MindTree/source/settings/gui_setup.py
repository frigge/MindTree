import MT
graph = MT.gui.showWindow("NodeGraph")
propedit = MT.gui.showSplitWindow("PropertiesEditor", graph, "HORIZONTAL", 0.25)
timeline = MT.gui.showSplitWindow("Timeline", graph, "VERTICAL", 0.1)
console = MT.gui.showTabbedWindow("Console", timeline)
