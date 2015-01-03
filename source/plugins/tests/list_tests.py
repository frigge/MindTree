from .utils import *
import MT

def testCreateList():
    createListNode = MT.createNode("General.Create List")
    floatValueNode = MT.createNode("Values.Float Value")
    intValueNode = MT.createNode("Values.Int Value")

    MT.project.root.addNode(createListNode)
    MT.project.root.addNode(floatValueNode)
    MT.project.root.addNode(intValueNode)
    
    floatValueNode.insockets[0].value = 5.0
    intValueNode.insockets[0].value = 10

    createListNode.insockets[0].connected = floatValueNode.outsockets[0]
    createListNode.insockets[1].connected = intValueNode.outsockets[0]

    cache = MT.cache.DataCache(createListNode.outsockets[0])

    test = TestCase()
    test.equal(cache.getOutput(), [5, 5, 5, 5, 5, 5, 5, 5, 5, 5])
    test.equal(floatValueNode.insockets[0].value, 5.0)
    test.equal(intValueNode.insockets[0].value, 10)

    return test.exit()
