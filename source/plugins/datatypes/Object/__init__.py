import MT
from . import object

class GroupObjectsNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Group"
    type = "GROUP"
    outsockets = [("Group", "GROUPDATA")]

    def __init__(self, node, raw=False):
        super().__init__(node, raw)
        node.setDynamicInSockets()

class ObjectNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Object"
    type = "OBJECTNODE"

    insockets = [
            ("Data", "OBJECTDATA"),
            ("Transform", "MAT4"),
            ("Material", "MATERIAL"),
            ("Child(ren)", "GROUPDATA")
        ]
    outsockets = [("Object", "TRANSFORMABLE")]

class CreateTransformationNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Transfomation"
    type = "TRANSFORM"
    insockets = [
            ("Translate", "VECTOR3D"),
            ("Rotate", "VECTOR3D"),
            ("Scale", "VECTOR3D", (1.0, 1.0, 1.0)),
            ]
    outsockets = [("Transformation", "MAT4")]

class TransformObjectNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Transform"
    type = "TRANSFORMOBJECT"
    insockets = [
            ("Object", "TRANSFORMABLE"),
            ("Transform", "MAT4"),
            ]
    outsockets = [("Object", "TRANSFORMABLE")]

class ParentNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Parent"
    type = "PARENTOBJECT"
    insockets = [
            ("Parent", "TRANSFORMABLE"),
            ("Child(ren)", "VARIABLE")
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
    type = "SPOTLIGHT"

    insockets = [
            ("Intensity", "FLOAT", 1.0),
            ("Color", "COLOR", (1, 1, 1, 1)),
            ("Cone Angle", "FLOAT", 45.0),
            ("Square Attenuation", "BOOLEAN", False),
            ("Shadow Mapping", "BOOLEAN", False),
            ("Shadow Map Size", "INTEGER", 512),
            ("Shadow Map Bias", "FLOAT", 0.05),
            ("Near Clipping", "FLOAT", 1.),
            ("Far Clipping", "FLOAT", 10.)
            ]
    outsockets = [
            ("Object", "TRANSFORMABLE")]

class DistantLightNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Lights.Distant Light"
    type = "DISTANTLIGHT"

    insockets = [
            ("Intensity", "FLOAT", 1.0),
            ("Color", "COLOR", (1, 1, 1, 1))
            ]
    outsockets = [
            ("Object", "TRANSFORMABLE")]

class EmptyNodeDecortator(MT.pytypes.NodeDecorator):
    label = "Objects.Empty"
    type = "EMPTY"

    outsockets = [
            ("Object", "TRANSFORMABLE")]

class CameraNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Camera"
    type = "CAMERA"

    insockets = [
            ("FOV", "FLOAT", 45),
            ("Resolution", "INTVEC2", (1024, 1024)),
            ("Near Clipping", "FLOAT", 0.1),
            ("Far Clipping", "FLOAT", 100)
            ]
    outsockets = [ ("Camera", "TRANSFORMABLE") ]

class MaterialInstanceNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Material.Default"
    type = "DEFAULTMATERIAL"

    insockets = [
            ("Diffuse Color", "COLOR", (1, 1, 1, 1)),
            ("Diffuse Intensity", "FLOAT", 0.8),
            ("Specular Intensity", "FLOAT", 0.8)
            ]
    outsockets = [("Material", "MATERIAL")]

class SetMaterialInstanceNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Material.Set Material"
    type = "SETMATERIALINSTANCE"

    insockets = [
            ("Object", "TRANSFORMABLE"),
            ("Material", "MATERIAL")
            ]
    outsockets = [("Object", "TRANSFORMABLE")]



MT.registerNode(GroupObjectsNodeDecorator)
MT.registerNode(TransformObjectNodeDecorator)
MT.registerNode(ParentNodeDecorator)
MT.registerNode(PointLightNodeDecorator)
MT.registerNode(SpotLightNodeDecorator)
MT.registerNode(DistantLightNodeDecorator)
MT.registerNode(MaterialInstanceNodeDecorator)
MT.registerNode(SetMaterialInstanceNodeDecorator)
MT.registerNode(EmptyNodeDecortator)
MT.registerNode(CameraNodeDecorator)
MT.registerNode(ObjectNodeDecorator)
MT.registerNode(CreateTransformationNodeDecorator)
