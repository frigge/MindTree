import MT
from . import object

class GroupObjectsNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Group"
    type = "GROUP"
    outsockets = [("Group", "GROUPDATA")]

    def __init__(self, node, raw=False):
        super().__init__(node, raw)
        node.setDynamicInSockets()

class TransformObjectNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Transform"
    type = "TRANSFORM"
    insockets = [
            ("Object", "TRANSFORMABLE"),
            ("Translate", "VECTOR3D"),
            ("Rotate", "VECTOR3D"),
            ("Scale", "VECTOR3D", (1.0, 1.0, 1.0)),
            ]
    outsockets = [("Object", "TRANSFORMABLE")]

class PointLightNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Lights.Point Light"
    type = "POINTLIGHT"

    insockets = [
            ("Intensity", "FLOAT", 1.0),
            ("Color", "COLOR", (1, 1, 1, 1))
            ]
    outsockets = [
            ("Object", "TRANSFORMABLE")]

class SpotLightNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Lights.Spot Light"
    type = "POINTLIGHT"

    insockets = [
            ("Intensity", "FLOAT", 1.0),
            ("Color", "COLOR", (1, 1, 1, 1)),
            ("Cone Angle", "FLOAT", 45.0),
            ("Square Attenuation", "BOOLEAN", False)
            ]
    outsockets = [
            ("Object", "TRANSFORMABLE")]

class DistantLightNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Lights.Distant Light"
    type = "POINTLIGHT"

    insockets = [
            ("Intensity", "FLOAT", 1.0),
            ("Color", "COLOR", (1, 1, 1, 1))
            ]
    outsockets = [
            ("Object", "TRANSFORMABLE")]


MT.registerNode(GroupObjectsNodeDecorator)
MT.registerNode(TransformObjectNodeDecorator)
MT.registerNode(PointLightNodeDecorator)
MT.registerNode(SpotLightNodeDecorator)
MT.registerNode(DistantLightNodeDecorator)
