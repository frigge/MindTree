import object, MT

class GroupObjectsNodeFactory(MT.pytypes.NodeFactory):
    label = "Objects.Group"
    type = "GROUP"
    insockets = [("+", "VARIABLE")]
    outsockets = [("Group", "GROUPDATA")]

    def __init__(self, node):
        MT.pytypes.NodeFactory.__init__(self, node)
        node.setDynamicInSockets()

class TransformObjectNodeFactory(MT.pytypes.NodeFactory):
    label = "Objects.Transform"
    type = "TRANSFORM"
    insockets = [
            ("Object", "SCENEOBJECT"),
            ("Translate", "VECTOR3D"),
            ("Rotate", "VECTOR3D"),
            ("Scale", "VECTOR3D", (1.0, 1.0, 1.0)),
            ]
    outsockets = [("Object", "SCENEOBJECT")]


MT.registerNode(GroupObjectsNodeFactory)
MT.registerNode(TransformObjectNodeFactory)
