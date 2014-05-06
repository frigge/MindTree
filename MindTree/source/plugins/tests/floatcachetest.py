import MT

def testFloatCache():
    '''Creating two add nodes, connecting them and trying to get a value from it'''
    add = MT.createNode("Math.Add")
    add.insockets[0].name = "add.in01"
    add.insockets[0].value = 3.
    add.addInSocket("add.in02", "FLOAT")
    add.outsockets[0].type = "FLOAT"
    add.insockets[1].value = 12.

    add2 = MT.createNode("Math.Add")
    add2.insockets[0].value = 5.

    add2.insockets[0].name = "add2.in01"
    add2.addInSocket("add2.in02", "FLOAT")
    add2.insockets[1].value = 8.

    add2.outsockets[0].type = "FLOAT"
    pos = add2.pos
    add2.pos = (pos[0] - 20, pos[1])

    MT.project.root.addNode(add)
    MT.project.root.addNode(add2)
    add.insockets[0].connected = add2.outsockets[0]

    cache = MT.cache.DataCache(add.outsockets[0])
    try:
        print("resulting value: %d" % cache.getData(0))
    except:
        print("resulting datatype is: %s" % str(type(cache.getData(0))))

    print("expected value: %d" % (12+5+8))
    return (12+5+8) == cache.getOutput()

def testfloatvalue():
    return True
