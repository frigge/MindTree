import MT

def testSocketProperties():
    value = MT.createNode("Values.Float Value")
    value.insockets[0].value = 2.5

    cache = MT.cache.DataCache(value.outsockets[0])

    return cache.data == 2.5
