import MT
from .utils import *

def testSimulation():
    simNode = MT.createNode("General.Simulation")
    add = MT.createNode("Math.Add")
    startValue = MT.createNode("Values.Int Value")

    MT.project.root.addNode(simNode)
    MT.project.root.addNode(startValue)
    simNode.graph.addNode(add)

    innode = simNode.graph[0]
    outnode = simNode.graph[1]

    add.insockets[0].connected = innode.outsockets[0]
    simNode.insockets[1].connected = startValue.outsockets[0]
    add.insockets[1].value = 10

    outnode.insockets[0].connected = add.outsockets[0]

    test = TestCase()

    test.equal(simNode.insockets[1].type, "INTEGER")
    test.equal(innode.outsockets[0].type, "INTEGER")
    test.equal(add.insockets[0].type, "INTEGER")
    test.equal(add.outsockets[0].type, "INTEGER")
    test.equal(outnode.insockets[0].type, "INTEGER", "outnode")
    test.equal(simNode.outsockets[0].type, "INTEGER", "simnode output")
    simNode.insockets[0].value = 1
    cache = MT.cache.DataCache(simNode.outsockets[0])
    test.equal(cache.getOutput(), 1, "Simulation Step: 1")

    simNode.insockets[0].value = 2
    cache = MT.cache.DataCache(simNode.outsockets[0])
    test.equal(cache.getOutput(), 2, "Simulation Step: 2")

    simNode.insockets[0].value = 3
    cache = MT.cache.DataCache(simNode.outsockets[0])
    test.equal(cache.getOutput(), 3, "Simulation Step: 3")

    simNode.insockets[0].value = 4
    cache = MT.cache.DataCache(simNode.outsockets[0])
    test.equal(cache.getOutput(), 4, "Simulation Step: 4")

    simNode.insockets[0].value = 5
    cache = MT.cache.DataCache(simNode.outsockets[0])
    test.equal(cache.getOutput(), 5, "Simulation Step: 5")
    return test.exit()

