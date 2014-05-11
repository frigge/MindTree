import MT

class FloatToVector3DDecorator(MT.pytypes.NodeDecorator):
    label = "Values.Converters.FloatToVector3D"
    type = "FLOATTOVECTOR3D"
    insockets = [
            ("X", "FLOAT"),
            ("Y", "FLOAT"),
            ("Z", "FLOAT")
            ]
    outsockets = [("Vector", "VECTOR3D")]

class FloatToVector2DDecorator(MT.pytypes.NodeDecorator):
    label = "Values.Converters.FloatToVector2D"
    type = "FLOATTOVECTOR2D"
    insockets = [
            ("X", "FLOAT"),
            ("Y", "FLOAT")
            ]
    outsockets = [("Vector", "VECTOR2D")]

class FloatToColorDecorator(MT.pytypes.NodeDecorator):
    label = "Values.Converters.FloatToColor"
    type = "FLOATTOCOLOR"
    insockets = [
            ("R", "FLOAT"),
            ("G", "FLOAT"),
            ("B", "FLOAT"),
            ("A", "FLOAT")
            ]
    outsockets = [("Color", "COLOR")]

class ColorToFloatDecorator(MT.pytypes.NodeDecorator):
    label = "Values.Converters.ColorToFloat"
    type = "COLORTOFLOAT"
    label = "Values.Converters.FloatToColor"
    type = "FLOATTOCOLOR"
    insockets = [("Color", "COLOR")]
    outsockets = [
            ("R", "FLOAT"),
            ("G", "FLOAT"),
            ("B", "FLOAT"),
            ("A", "FLOAT")
            ]

class Vector3DToFloatDecorator(MT.pytypes.NodeDecorator):
    label = "Values.Converters.Vector3DToFloat"
    type = "VECTOR3DTOFLOAT"
    insockets = [("Vector", "VECTOR3D")]
    outsockets = [
            ("X", "FLOAT"),
            ("Y", "FLOAT"),
            ("Z", "FLOAT")
            ]

class Vector2DToFloatDecorator(MT.pytypes.NodeDecorator):
    label = "Values.Converters.Vector2DToFloat"
    type = "VECTOR2DTOFLOAT"
    insockets = [("Vector", "VECTOR2D")]
    outsockets = [
            ("X", "FLOAT"),
            ("Y", "FLOAT")
            ]

MT.registerNode(FloatToVector3DDecorator)
MT.registerNode(FloatToVector2DDecorator)
MT.registerNode(FloatToColorDecorator)
MT.registerNode(Vector3DToFloatDecorator)
MT.registerNode(Vector2DToFloatDecorator)
MT.registerNode(ColorToFloatDecorator)
