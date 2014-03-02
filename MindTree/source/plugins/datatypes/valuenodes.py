import MT

class FloatNodeFactory(MT.pytypes.NodeFactory):
    type="FLOATVALUE"
    label="Values.Float Value"
    insockets = [("Value", "FLOAT")]
    outsockets = [("Value", "FLOAT")]

class IntNodeFactory(MT.pytypes.NodeFactory):
    type="INTVALUE"
    label="Values.Int Value"
    insockets = [("Value", "INTEGER")]
    outsockets = [("Value", "INTEGER")]

def floatvalue(cache):
    cache.cache(cache.node.insockets[0])

def registerNodes():
    MT.registerNode(FloatNodeFactory)
    MT.registerNode(IntNodeFactory)

registerNodes()
MT.cache.DataCache.addProcessor(floatvalue, "FLOATVALUE", "FLOAT")
