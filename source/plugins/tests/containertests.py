import MT
from .utils import *

class TestContainer(TestCase):
    def _createNetwork(self):
        self.add = MT.createNode("Math.Add")
        self.add2 = MT.createNode("Math.Add")
        self.add3 = MT.createNode("Math.Add")

        self.valuenode = MT.createNode("Values.Float Value")
        self.valuenode2 = MT.createNode("Values.Float Value")
        self.valuenode3 = MT.createNode("Values.Float Value")

        MT.project.root.addNode(self.valuenode3)
        MT.project.root.addNode(self.add)
        MT.project.root.addNode(self.add2)
        MT.project.root.addNode(self.add3)
        MT.project.root.addNode(self.valuenode)
        MT.project.root.addNode(self.valuenode2)
        pos = self.valuenode.pos
        self.valuenode.pos = (pos[0] - 150, pos[1] - 50)
        self.valuenode2.pos = (pos[0] - 150, pos[1] + 50)

        self.add.insockets[0].connected = self.valuenode.outsockets[0]
        self.add.insockets[1].connected = self.valuenode2.outsockets[0]
        self.add.insockets[2].connected = self.valuenode3.outsockets[0]

        self.add2.insockets[0].connected = self.add.outsockets[0]
        self.add3.insockets[0].connected = self.add2.outsockets[0]

    def _createContainer(self):
        MT.createContainer([self.valuenode, self.valuenode2, self.add, self.add2])

    def testCreateContainer(self):
        self._createNetwork()
        self._createContainer()

        self.equal(len(MT.project.root), 3)
        self.equal(self.add.insockets[0].connected, self.valuenode.outsockets[0])
        self.equal(self.add.insockets[1].connected, self.valuenode2.outsockets[0])
        self.equal(self.add2.insockets[0].connected, self.add.outsockets[0])

        container = MT.project.root[len(MT.project.root) - 1]
        self.equal(container.name, "Container")
        self.equal(container.insockets[0].connected, self.valuenode3.outsockets[0])
        self.equal(self.add3.insockets[0].connected, container.outsockets[0])
        
        return self.exit()

def testCreateContainer():
    test = TestContainer()
    return test.testCreateContainer()
