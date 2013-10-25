import MT

def addProc(cache):
    data = 0.0
    print("this node is %s" % cache.node.name)
    for socket in cache.node.insockets:
        cache.cache(socket)
        print("reading socket "+socket.name+ ": "+str(cache.data))
        print("socket is connected to %s" % socket.connected)
        if cache.data != None:
            data += cache.data
    cache.data = data

def testFloatCache():
    add = MT.createNode("Math.Add")
    add.insockets[0].name = "add.in01"
    add.insockets[0].value = 3
    add.addInSocket("add.in02", "FLOAT")
    add.outsockets[0].type = "FLOAT"
    add.insockets[1].value = 12
    print("setting socket value to 12")

    add2 = MT.createNode("Math.Add")
    add2.insockets[0].value = 5
    print("setting socket value to 5")

    add2.insockets[0].name = "add2.in01"
    add2.addInSocket("add2.in02", "FLOAT")
    add2.insockets[1].value = 8
    print("setting socket value to 8")
    print("--------------------------")

    add2.outsockets[0].type = "FLOAT"
    pos = add2.pos
    add2.pos = (pos[0] - 20, pos[1])

    MT.project.root.addNode(add)
    MT.project.root.addNode(add2)
    print("connecting {0} over {1} to {2} over {3}".format(add.name, add.insockets[0].name, add2.name, add2.outsockets[0].name))
    add.insockets[0].connected = add2.outsockets[0]
    MT.cache.DataCache.addProcessor(addProc, "ADD", "FLOAT")
    print("start caching ...")
    print("")

    cache = MT.cache.DataCache(add.outsockets[0])
    print("12 + 5 + 8 should be %d" % (12+5+8))
    print("but we get %d" % cache.data)
    return (12+5+8) == cache.data
