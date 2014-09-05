import MT
from .utils import *

def testContainerCache():
    '''Creating two add nodes, connecting them and trying to get a value from it'''
    add = MT.createNode("Math.Add")
    valuenode = MT.createNode("Values.Float Value")
    valuenode2 = MT.createNode("Values.Float Value")
    valuenode3 = MT.createNode("Values.Float Value")
    container = MT.createNode("General.Container")
    output = container.graph[1]
    inputnode = container.graph[0]

    value1 = 5.
    value2 = 8.
    value3 = 12.
    value4 = 23.

    add.insockets[0].name = "add.in01"

    valuenode.insockets[0].name = "valuenode.in01"
    valuenode.insockets[0].value = value1
    valuenode2.insockets[0].name = "valuenode2.in01"
    valuenode2.insockets[0].value = value2
    valuenode3.insockets[0].value = value4

    MT.project.root.addNode(container)
    MT.project.root.addNode(valuenode3)
    container.graph.addNode(add)
    container.graph.addNode(valuenode)
    container.graph.addNode(valuenode2)
    pos = valuenode.pos
    valuenode.pos = (pos[0] - 150, pos[1] - 50)
    valuenode2.pos = (pos[0] - 150, pos[1] + 50)

    add.insockets[0].connected = valuenode.outsockets[0]
    add.insockets[1].connected = valuenode2.outsockets[0]
    add.insockets[2].connected = inputnode.outsockets[0]

    output.insockets[0].connected = add.outsockets[0]
    container.insockets[0].connected = valuenode3.outsockets[0]
    add.insockets[3].value = value3

    cache = MT.cache.DataCache(container.outsockets[0])

    expected_result = value1 + value2 + value3 + value4

    test = TestCase()
    test.equal(cache.getOutput(), expected_result)
    test.equal(container.insockets[0].type, valuenode3.outsockets[0].type)
    test.equal(add.insockets[0].type, valuenode.outsockets[0].type)
    test.equal(add.insockets[1].type, valuenode2.outsockets[0].type)
    test.equal(add.insockets[2].type, inputnode.outsockets[0].type)
    test.equal(inputnode.outsockets[0].type, container.insockets[0].type)
    test.equal(container.insockets[0].type, "FLOAT")
    test.equal(add.insockets[0].type, "FLOAT")
    test.equal(add.insockets[1].type, "FLOAT")
    test.equal(add.insockets[2].type, "FLOAT")
    test.equal(inputnode.outsockets[0].type, "FLOAT")
    return test.exit()
