import MT, os, PyQt5, sys

from . import methodupdater

from PyQt5 import QtCore
from PyQt5 import QtWidgets

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

class ModuleWatcher:
    def __init__(self, module, path):
        self.module = module
        if  module.__file__.endswith(".py"):
            self.modulefile = module.__file__
        else:
            self.modulefile = module.__file__[:-1]
            module.__file__ = self.modulefile
        self.time = os.path.getmtime(self.modulefile)
        print("start watching", module)

    def watch(self):
        if self.time < os.path.getmtime(self.modulefile):
            self.time = os.path.getmtime(self.modulefile)
            print("reloading", self.module)
            methodupdater.updateModule(self.module, self.pypath)

class ModuleWatcherWindow(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        lay = QVBoxLayout()
        hlay = QHBoxLayout()
        vlay = QVBoxLayout()
        self.setLayout(lay)
        lay.addLayout(hlay)
        
        self.module_list = QTreeWidget()
        self.watched_list = QTreeWidget()
        self.watchers = {}
        self.instance_path = {}
        hlay.addWidget(self.module_list)
        hlay.addLayout(vlay)
        vlay.addWidget(self.watched_list)

        self.edit = QLineEdit()
        vlay.addWidget(self.edit)
        self.edit.textChanged.connect(self.updateInstancePath)
        self.watched_list.itemSelectionChanged.connect(self.updateEdit)

        button = QPushButton("Watch")
        button.clicked.connect(self.startWatching)
        lay.addWidget(button)

        modules = []
        
        oldcwd = os.getcwd()
        os.chdir("../plugins")
        plugdir = os.getcwd()
        os.chdir(oldcwd)
        for p, m in sys.modules.iteritems():
            if m and "__file__" in m.__dict__.keys() and plugdir in m.__file__:
                modules.append(p)

        for i, m in enumerate(modules):
            self.module_list.addTopLevelItem(QTreeWidgetItem([m]))

    def updateInstancePath(self, text):
        try:
            mod = self.watched_list.selectedItems()[0].text(0)
        except IndexError:
            return
        self.instance_path[mod] = text
        self.watchers[mod].pypath = text

    def updateEdit(self):
        self.edit.setText(self.watched_list.selectedItems()[0].text(0))

    def paintEvent(self, event):
        QWidget.paintEvent(self, event)
        for w in self.watchers.values():
            w.watch()

    def refillWatched(self):
        self.watched_list.clear()
        for w in self.watchers.values():
            self.watched_list.addTopLevelItem(QTreeWidgetItem([w.module.__name__]))

    def startWatching(self):
        t = str(self.module_list.selectedItems()[0].text(0))
        self.watchers[t] = ModuleWatcher(sys.modules[t], "")
        self.refillWatched()
        
            
MT.gui.registerWindow("ModuleWatcher", ModuleWatcherWindow)
