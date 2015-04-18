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

    valuenode.insockets[0].value = 10.
    MT.cache.DataCache.invalidate(valuenode)
    cache = MT.cache.DataCache(container.outsockets[0])
    expected_result = 10. + value2 + value3 + value4
    test.equal(cache.getOutput(), expected_result, "invalidation")

    return test.exit()

def testForLoopCache():
    test = TestCase()

    loop = MT.createNode("General.For")
    MT.project.root.addNode(loop)

    loop.insockets[0].value = 0
    loop.insockets[1].value = 10
    loop.insockets[2].value = 1

    add = MT.createNode("Math.Add")
    loop.graph.addNode(add)
    test.equal(len(loop.graph[2].insockets), 0, "outsockets of loop container before (direct) connection")
    loopsocket = loop.graph[1].outsockets[0]
    add.insockets[0].connected = loopsocket
    test.equal(len(loop.graph[2].insockets), 1, "outsockets of loop container before (indirect) connection")
    add.insockets[1].value = 1

    loop.graph[2].insockets[0].connected = add.outsockets[0]

    loop.insockets[3].value = 1

    test.equal(add.outsockets[0].type, "INTEGER", "output of Add")
    test.equal(add.insockets[0].type, "INTEGER", "1st input of Add")
    test.equal(add.insockets[1].type, "INTEGER", "2nd input of Add")
    test.equal(loop.outsockets[0].type, "INTEGER", "output of ForNode")
    test.equal(loop.insockets[3].type, "INTEGER", "input of startvalue into ForLoop")
    test.equal(loop.graph[1].outsockets[0].type, "INTEGER", "input of ForLoop Container")
    test.equal(loop.graph[2].insockets[0].type, "INTEGER", "output of ForLoop Container")
    test.equal(len(loop.graph), 4, "nodes inside loop container")
    test.equal(len(loop.graph[0].outsockets), 4, "static loop inputs")
    test.equal(len(loop.graph[1].outsockets), 2, "looped inputs")
    if not test.equal(len(loop.graph[2].insockets), 1, "loop outputs"):
        print("the sockets on this node: {}".format([s.name for s in loop.graph[2].insockets]))
    test.equal(len(loop.outsockets), 1, "number of outsockes on forloop node")
    test.equal(len(loop.insockets), 4, "number of insockets on forloop node")

    cache = MT.cache.DataCache(loop.outsockets[0])

    expected_result = 11

    test.equal(cache.getOutput(), expected_result, "Loop Result")
    return test.exit()

def testWhileLoopCache():
    loop = MT.createNode("General.While")
    MT.project.root.addNode(loop)

    test = TestCase()
    return test.exit()

def testForeachLoopCache():
    loop = MT.createNode("General.Foreach")
    listNode = MT.createNode("General.Create List")
    add = MT.createNode("Math.Add")
    value = MT.createNode("Values.Int Value")

    listNode.insockets[0].value = 12.5
    listNode.insockets[1].value = 10

    value.insockets[0].value = 2

    loop.insockets[0].connected = listNode.outsockets[0]

    test = TestCase()
    
    loopInputNode = loop.graph[1]

    test.equal(listNode.insockets[0].type, "FLOAT")
    test.equal(listNode.outsockets[0].type, "LIST:FLOAT")
    test.equal(len(loopInputNode.outsockets), 1, "inputs to the foreach node")

    add.insockets[0].connected = loopInputNode.outsockets[0]
    add.insockets[1].connected = value.outsockets[0]

    loop.graph[2].insockets[0].connected = add.outsockets[0]

    MT.project.root.addNode(loop)
    MT.project.root.addNode(listNode)

    loop.graph.addNode(add)
    loop.graph.addNode(value)

    cache = MT.cache.DataCache(loop.outsockets[0])

    output = cache.getOutput()
    test.equal(add.insockets[0].type, "FLOAT")
    test.equal(add.insockets[1].type, "INTEGER")
    test.equal(loop.graph[1].outsockets[0].type, "FLOAT")
    test.equal(len(output), 10)
    test.equal(cache.getOutput(), [14.5] * 10) 
    return test.exit()
