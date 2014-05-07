import MT

def setupGui(gui_dict, parent=""):
    print("setting up GUI")
    if type(gui_dict.values()[0]) == str:
        other = MT.gui.showWindow(gui_dict.values()[0])
        gui_dict.pop(gui_dict.keys()[0])
    else:
        other = parent
    for key, value in gui_dict.iteritems():
        if type(value) == dict:
            setupGui(value, other)
        elif type(value) == tuple and other != "":
            if key == "TAB":
                other = MT.gui.showTabbedWindow(value, other)
            elif key == "VERTICAL" or key == "HORIZONTAL":
                other = MT.gui.showSplitWindow(value[0], other, key, value[1])

