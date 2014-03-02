import MT

def testStringCache():
    add = MT.createNode("Math.Add")
    add.insockets[0].name = "value01"
    add.addInSocket("value02", "STRING")
    add.insockets[0].value = "bla"
    add.outsockets[0].type = "STRING"
    value3 = "blubb"
    add.insockets[1].value = "blubb"

    add2 = MT.createNode("Math.Add")
    value1 = "trallalla"
    add2.insockets[0].value = "trallalla"
    add2.insockets[0].name = "value01"
    add2.addInSocket("value02", "STRING")
    add2.insockets[1].value = "lalalla"
    value2 = "lalalla"

    add2.outsockets[0].type = "STRING"
    pos = add2.pos
    add2.pos = (pos[0] - 20, pos[1])

    MT.project.root.addNode(add)
    MT.project.root.addNode(add2)
    add.insockets[0].connected = add2.outsockets[0]

    cache = MT.cache.DataCache(add.outsockets[0])
    return (value1 + value2 + value3) == cache.data
