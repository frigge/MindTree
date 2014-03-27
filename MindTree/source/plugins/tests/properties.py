import MT

def testSocketProperties():
    value = MT.createNode("Values.Float Value")
    value.insockets[0].value = 2.5
    cache = MT.cache.DataCache(value.outsockets[0])
    success = cache.getData(0) == 2.5

    value = MT.createNode("Values.Int Value")
    value.insockets[0].value = 2
    cache = MT.cache.DataCache(value.outsockets[0])
    success = success and cache.getData(0) == 2

    value = MT.createNode("Values.Color Value")
    value.insockets[0].value = (1, 0, 0, 1)
    cache = MT.cache.DataCache(value.outsockets[0])
    success = success and cache.getData(0) == (1, 0, 0, 1)

    value = MT.createNode("Values.String Value")
    value.insockets[0].value = "bla bla HUBSGHDJ"
    cache = MT.cache.DataCache(value.outsockets[0])
    success = success and cache.getData(0) == "bla bla HUBSGHDJ"
    return success
