import MT
from . import object

class GroupObjectsNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Group"
    type = "GROUP"
    insockets = [("+", "VARIABLE")]
    outsockets = [("Group", "GROUPDATA")]

    def __init__(self, node):
        MT.pytypes.NodeDecorator.__init__(self, node)
        node.setDynamicInSockets()

class TransformObjectNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Transform"
    type = "TRANSFORM"
    insockets = [
            ("Object", "SCENEOBJECT"),
            ("Translate", "VECTOR3D"),
            ("Rotate", "VECTOR3D"),
            ("Scale", "VECTOR3D", (1.0, 1.0, 1.0)),
            ]
    outsockets = [("Object", "SCENEOBJECT")]


MT.registerNode(GroupObjectsNodeDecorator)
MT.registerNode(TransformObjectNodeDecorator)
