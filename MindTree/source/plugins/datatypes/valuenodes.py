import MT

class BoolNodeDecorator(MT.pytypes.NodeDecorator):
    type="BOOLVALUE"
    label="Values.Bool Value"
    insockets = [("Value", "BOOLEAN")]
    outsockets = [("Value", "BOOLEAN")]

class FloatNodeDecorator(MT.pytypes.NodeDecorator):
    type="FLOATVALUE"
    label="Values.Float Value"
    insockets = [("Value", "FLOAT")]
    outsockets = [("Value", "FLOAT")]

class IntNodeDecorator(MT.pytypes.NodeDecorator):
    type="INTVALUE"
    label="Values.Int Value"
    insockets = [("Value", "INTEGER")]
    outsockets = [("Value", "INTEGER")]

class StringNodeDecorator(MT.pytypes.NodeDecorator):
    type="STRINGVALUE"
    label="Values.String Value"
    insockets = [("Value", "STRING")]
    outsockets = [("Value", "STRING")]

class ColorNodeDecorator(MT.pytypes.NodeDecorator):
    type="COLORVALUE"
    label="Values.Color Value"
    insockets = [("Value", "COLOR")]
    outsockets = [("Value", "COLOR")]

class Vector3DNodeDecorator(MT.pytypes.NodeDecorator):
    type="VECTOR3DVALUE"
    label="Values.Vector3D"
    insockets = [("Value", "VECTOR3D")]
    outsockets = [("Value", "VECTOR3D")]

class Vector2DNodeDecorator(MT.pytypes.NodeDecorator):
    type="VECTOR3DVALUE"
    label="Values.Vector2D"
    insockets = [("Value", "VECTOR2D")]
    outsockets = [("Value", "VECTOR2D")]

def registerNodes():
    MT.registerNode(BoolNodeDecorator)
    MT.registerNode(FloatNodeDecorator)
    MT.registerNode(IntNodeDecorator)
    MT.registerNode(StringNodeDecorator)
    MT.registerNode(ColorNodeDecorator)
    MT.registerNode(Vector3DNodeDecorator)
    MT.registerNode(Vector2DNodeDecorator)

registerNodes()
