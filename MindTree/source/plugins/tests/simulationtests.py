import MT
import utils

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

    success = False
    simNode.insockets[0].value = 1
    cache = DataCache(simNode.outsockets[0])
    success = success and equal(cache.getOutput(0), 1)

    simNode.insockets[0].value = 2
    cache = DataCache(simNode.outsockets[0])
    success = success and equal(cache.getOutput(0), 2)

    simNode.insockets[0].value = 3
    cache = DataCache(simNode.outsockets[0])
    success = success and equal(cache.getOutput(0), 3)

    simNode.insockets[0].value = 4
    cache = DataCache(simNode.outsockets[0])
    success = success and equal(cache.getOutput(0), 4)

    simNode.insockets[0].value = 5
    cache = DataCache(simNode.outsockets[0])
    success = success and equal(cache.getOutput(0), 5)
    return success

