from . import prim3d

import MT

class CubeNodeDecorator(MT.pytypes.NodeDecorator):
    type="CUBE"
    label="Objects.Primitives.Cube"
    insockets = [ ("Scale", "FLOAT", 1.)]
    outsockets = [("Cube", "TRANSFORMABLE")]

class PlaneNodeDecorator(MT.pytypes.NodeDecorator):
    type="PLANE"
    label="Objects.Primitives.Plane"
    insockets = [ ("Scale", "FLOAT", 1.)]
    outsockets = [("Plane", "TRANSFORMABLE")]

MT.registerNode(CubeNodeDecorator)
MT.registerNode(PlaneNodeDecorator)
