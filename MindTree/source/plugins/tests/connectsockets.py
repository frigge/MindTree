import MT
from .utils import *

def testConnectSockets():
    add = MT.createNode("Math.Add")
    add2 = MT.createNode("Math.Add")

    MT.project.root.addNode(add)
    MT.project.root.addNode(add2)

    add.insockets[0].connected = add2.outsockets[0]

    test = TestCase()
    test.equal(add.insockets[0].connected.ptr, add2.outsockets[0].ptr)

    return test.exit();

def testInheritSocketTypeMath():
    add = MT.createNode("Math.Add")
    flnode = MT.createNode("Values.Float Value")

    add.insockets[0].connected = flnode.outsockets[0]

    test = TestCase()
    test.equal(add.insockets[0].type, flnode.outsockets[0].type, "connected")
    test.equal(add.insockets[0].type, add.outsockets[0].type, "in to out")

    return test.exit()

def testInheritSocketTypeContainer():
    test = TestCase()
    container = MT.createNode("General.Container")
    floatValue = MT.createNode("Values.Float Value")
    intValue = MT.createNode("Values.Int Value")

    MT.project.root.addNode(container)
    container.graph.addNode(floatValue)

    container.graph[1].insockets[0].connected = floatValue.outsockets[0]
    intValue.insockets[0].connected = container.graph[0].outsockets[0]

    test.equal(container.graph[1].insockets[0].type, "FLOAT")
    test.equal(container.outsockets[0].type, "FLOAT")
    test.equal(container.graph[0].outsockets[0].type, "INTEGER", "input sockets of container graph")
    test.equal(container.insockets[0].type, "INTEGER", "input of container itself")
    return test.exit()

def testInheritSocketTypeLoop():
    test = TestCase()
    return test.exit()

def groupTestInheritSocketType():
    success = True
    for fn in [testInheritSocketTypeLoop, testInheritSocketTypeMath, testInheritSocketTypeContainer]:
        _success = fn()
        print("testing: {} ... {}".format(fn.__name__, "passed" if _success else "failed"))
        success = success and _success

    return success
