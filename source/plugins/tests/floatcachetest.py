import MT
from .utils import *

def testFloatCache():
    '''Creating two add nodes, connecting them and trying to get a value from it'''
    add = MT.createNode("Math.Add")
    valuenode = MT.createNode("Values.Float Value")
    valuenode2 = MT.createNode("Values.Float Value")

    add.insockets[0].name = "add.in01"

    valuenode.insockets[0].name = "valuenode.in01"
    valuenode.insockets[0].value = 5.
    valuenode2.insockets[0].name = "valuenode2.in01"
    valuenode2.insockets[0].value = 8.

    MT.project.root.addNode(add)
    MT.project.root.addNode(valuenode)
    MT.project.root.addNode(valuenode2)
    pos = valuenode.pos
    valuenode.pos = (pos[0] - 150, pos[1] - 50)
    valuenode2.pos = (pos[0] - 150, pos[1] + 50)

    add.insockets[0].connected = valuenode.outsockets[0]
    add.insockets[1].connected = valuenode2.outsockets[0]
    add.insockets[2].value = 12.

    cache = MT.cache.DataCache(add.outsockets[0])

    test = TestCase()
    test.equal(len(MT.project.root), 3, "root space size")
    test.equal(len(MT.project.root[0].outsockets), 1, "add outsockets")
    test.equal(len(MT.project.root[0].insockets), 3, "add insockets")
    test.equal(len(MT.project.root[1].outsockets), 1, "float value 1 outsockets")
    test.equal(len(MT.project.root[1].insockets), 1, "float value 1 insockets")
    test.equal(len(MT.project.root[2].outsockets), 1, "float value 2 outsockets")
    test.equal(len(MT.project.root[2].insockets), 1, "float value 2 insockets")
    test.equal(MT.project.root[1].insockets[0].value, 5., "float value 1")
    test.equal(MT.project.root[2].insockets[0].value, 8., "float value 2")
    test.equal(MT.project.root[0].insockets[2].value, 12., "add input value")

    print("resulting value: %d" % cache.getOutput())
    print("expected value: %d" % (12+5+8))
    test.equal((12+5+8), cache.getOutput(), "cache output")
    return test.exit()
