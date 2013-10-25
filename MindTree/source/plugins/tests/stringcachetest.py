import MT

def addProc(cache):
    data = ""
    for socket in cache.node.insockets:
        cache.cache(socket)
        print("reading socket "+socket.name+ ": "+str(cache.data))
        try:
            data += cache.data
        except TypeError:
            pass
        print("data is now "+str(data))
    cache.data = data

def testStringCache():
    add = MT.createNode("Math.Add")
    add.insockets[0].name = "value01"
    add.addInSocket("value02", "String")
    add.insockets[0].value = "bla"
    add.outsockets[0].type = "String"
    value3 = "blubb"
    add.insockets[1].value = "blubb"

    add2 = MT.createNode("Math.Add")
    value1 = "trallalla"
    add2.insockets[0].value = "trallalla"
    add2.insockets[0].name = "value01"
    add2.addInSocket("value02", "String")
    add2.insockets[1].value = "lalalla"
    value2 = "lalalla"

    add2.outsockets[0].type = "String"
    pos = add2.pos
    add2.pos = (pos[0] - 20, pos[1])

    MT.project.root.addNode(add)
    MT.project.root.addNode(add2)
    add.insockets[0].connected = add2.outsockets[0]

    MT.cache.DataCache.addProcessor(addProc, "ADD", "String")

    cache = MT.cache.DataCache(add.outsockets[0])
    return (value1 + value2 + value3) == cache.data
