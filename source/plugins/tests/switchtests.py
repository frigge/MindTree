import MT
from .utils import *

def testSwitchNode():
    value1 = MT.createNode("Values.Float Value")
    value2 = MT.createNode("Values.Float Value")
    value3 = MT.createNode("Values.Float Value")
    
    value1.insockets[0].value = 1
    value2.insockets[0].value = 3
    value3.insockets[0].value = 6

    switch = MT.createNode("General.Switch")

    MT.project.root.addNode(value1)
    MT.project.root.addNode(value2)
    MT.project.root.addNode(value3)
    MT.project.root.addNode(switch)

    switch.insockets[1].connected = value1.outsockets[0]
    switch.insockets[2].connected = value2.outsockets[0]
    switch.insockets[3].connected = value3.outsockets[0]

    test = TestCase()

    print("Test1:")
    switch.insockets[0].value = 0
    cache = MT.cache.DataCache(switch.outsockets[0])
    test.equal(cache.getOutput(), value1.insockets[0].value)

    print("Test2:")
    switch.insockets[0].value = 1
    MT.cache.DataCache.invalidate(switch)
    cache = MT.cache.DataCache(switch.outsockets[0])
    test.equal(cache.getOutput(), value2.insockets[0].value)

    print("Test3:")
    switch.insockets[0].value = 2
    MT.cache.DataCache.invalidate(switch)
    cache = MT.cache.DataCache(switch.outsockets[0])
    test.equal(cache.getOutput(), value3.insockets[0].value)

    return test.exit()
