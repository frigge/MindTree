import MT

def testStringCache():
    value1 = "!1+"
    value2 = "+2+"
    value3 = "+3!"
    
    add = MT.createNode("Math.Add")
    valuenode = MT.createNode("Values.String Value")
    valuenode2 = MT.createNode("Values.String Value")

    valuenode.insockets[0].value = value1
    valuenode2.insockets[0].value = value2

    print("setting the input of valuenode to %s" % value1)

    MT.project.root.addNode(add)
    MT.project.root.addNode(valuenode)
    MT.project.root.addNode(valuenode2)

    pos = valuenode.pos
    valuenode.pos = (pos[0] - 150, pos[1] - 50)
    valuenode2.pos = (pos[0] - 150, pos[1] + 50)

    add.insockets[0].connected = valuenode.outsockets[0]
    add.insockets[1].connected = valuenode2.outsockets[0]
    add.insockets[2].value = value3

    cache = MT.cache.DataCache(add.outsockets[0])

    print("resulting value: %s" % cache.getOutput())
    print("expected value: %s" % value1 + value2 + value3)
    return (value1 + value2 + value3) == cache.getOutput()
