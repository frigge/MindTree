import MT

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

    print("resulting value: %d" % cache.getOutput())
    print("expected value: %d" % (12+5+8))
    return (12+5+8) == cache.getOutput()

def testfloatvalue():
    return True
