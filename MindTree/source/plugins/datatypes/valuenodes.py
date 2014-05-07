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

class StringNodeFactory(MT.pytypes.NodeFactory):
    type="STRINGVALUE"
    label="Values.String Value"
    insockets = [("Value", "STRING")]
    outsockets = [("Value", "STRING")]

class ColorNodeFactory(MT.pytypes.NodeFactory):
    type="COLORVALUE"
    label="Values.Color Value"
    insockets = [("Value", "COLOR")]
    outsockets = [("Value", "COLOR")]

class Vector3DNodeFactory(MT.pytypes.NodeFactory):
    type="VECTOR3DVALUE"
    label="Values.Vector3D"
    insockets = [("Value", "VECTOR3D")]
    outsockets = [("Value", "VECTOR3D")]

def registerNodes():
    MT.registerNode(FloatNodeFactory)
    MT.registerNode(IntNodeFactory)
    MT.registerNode(StringNodeFactory)
    MT.registerNode(ColorNodeFactory)
    MT.registerNode(Vector3DNodeFactory)

registerNodes()
