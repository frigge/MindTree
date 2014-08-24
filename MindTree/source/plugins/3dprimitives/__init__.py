from . import prim3d

import MT

class CubeNodeDecorator(MT.pytypes.NodeDecorator):
    type="CUBE"
    label="Objects.Primitives.Cube"
    insockets = [ ("Scale", "FLOAT", 1.)]
    outsockets = [("Cube", "SCENEOBJECT")]

class PlaneNodeDecorator(MT.pytypes.NodeDecorator):
    type="PLANE"
    label="Objects.Primitives.Plane"
    insockets = [ ("Scale", "FLOAT", 1.)]
    outsockets = [("Plane", "SCENEOBJECT")]

MT.registerNode(CubeNodeDecorator)
MT.registerNode(PlaneNodeDecorator)
