import prim3d

import MT

class CubeNodeFactory(MT.pytypes.NodeFactory):
    type="CUBE"
    label="Objects.Primitives.Cube"
    insockets = [
            ("Scale", "FLOAT", 1.),
            ("Point Color", "COLOR", (1, 0, 0, 1)),
            ("Point Size", "INTEGER", 2),
            ("Edge Color", "COLOR", (1, 1, 1, 1)),
            ("Poly Color", "COLOR", (1, 1, 1, 1)),
            ("Show Vertex Normals", "BOOLEAN", False),
            ("Show Face Normals", "BOOLEAN", False),
            ("Vertex Normal Width", "INTEGER", 1),
            ("Vertex Normal Color", "COLOR", (1, 1, 1, 1)),
            ("Vertex Normal Size", "FLOAT", 1.)
            ]
    outsockets = [("Cube", "GROUPDATA")]

MT.registerNode(CubeNodeFactory)
