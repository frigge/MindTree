import prim3d

import MT

class CubeNodeFactory(MT.pytypes.NodeFactory):
    type="CUBE"
    label="Objects.Primitives.Cube"
    insockets = [ ("Scale", "FLOAT", 1.)]
    outsockets = [("Cube", "SCENEOBJECT")]

class PlaneNodeFactory(MT.pytypes.NodeFactory):
    type="PLANE"
    label="Objects.Primitives.Plane"
    insockets = [ ("Scale", "FLOAT", 1.)]
    outsockets = [("Plane", "SCENEOBJECT")]

MT.registerNode(CubeNodeFactory)
MT.registerNode(PlaneNodeFactory)
