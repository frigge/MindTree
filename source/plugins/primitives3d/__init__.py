from . import prim3d
import datatypes

import MT

class PrimitiveNodeDecoratorBase(MT.pytypes.NodeDecorator):
    base = "General.Container"
    datanode = ""

    def __init__(self, node, raw=False):
        super().__init__(node, raw)
        if raw:
            return

        data = MT.createNode(self.datanode)
        obj = MT.createNode("Objects.Object")
        node.graph.addNode(data)
        node.graph.addNode(obj)

        obj.insockets[0].connected = node.graph[0].outsockets[-1]
        obj.insockets[1].connected = node.graph[0].outsockets[-1]
        obj.insockets[2].connected = data.outsockets[0]
        obj.insockets[3].connected = node.graph[0].outsockets[-1]

        for s in data.insockets:
            default = s.value
            s.connected = node.graph[0].outsockets[-1]
            node.insockets[-1].value = default

        node.graph[1].insockets[0].connected = obj.outsockets[0]

        node.graph[2].pos = (-70, 80)
        node.graph[3].pos = (70, 80)
        node.graph[1].pos = (70, 160)

class CylinderDataNodeDecorator(MT.pytypes.NodeDecorator):
    type="CYLINDER"
    label="Objects.Data.Cylinder"
    insockets = [ ("Sides", "INTEGER", 3),
                  ("Cap Ends", "BOOLEAN", False)]
    outsockets = [("Cylinder", "OBJECTDATA")]

class CubeDataNodeDecorator(MT.pytypes.NodeDecorator):
    type="CUBE"
    label="Objects.Data.Cube"
    insockets = [ ("Scale", "FLOAT", 1.)]
    outsockets = [("Cube", "OBJECTDATA")]

class PlaneDataNodeDecorator(MT.pytypes.NodeDecorator):
    type="PLANE"
    label="Objects.Data.Plane"
    insockets = [ ("Size", "FLOAT", 1.)]
    outsockets = [("Plane", "OBJECTDATA")]

class IcosphereDataNodeDecorator(MT.pytypes.NodeDecorator):
    type="CREATEICOSPHEREDATA"
    label="Objects.Data.Icosphere"
    insockets = [ ("Scale", "FLOAT", 1.), ("Subdivisions", "INTEGER", 1)]
    outsockets = [("Sphere", "OBJECTDATA")]

class CubeNodeDecorator(PrimitiveNodeDecoratorBase):
    label="Objects.Primitive.Cube"
    datanode = "Objects.Data.Cube"

class PlaneNodeDecorator(PrimitiveNodeDecoratorBase):
    label="Objects.Primitives.Plane"
    datanode = "Objects.Data.Plane"

class IcosphereNodeDecorator(PrimitiveNodeDecoratorBase):
    label="Objects.Primitives.Icosphere"
    datanode = "Objects.Data.Icosphere"

class CylinderNodeDecorator(PrimitiveNodeDecoratorBase):
    label="Objects.Primitives.Cylinder"
    datanode = "Objects.Data.Cylinder"

class MeshDataNodeDecorator(MT.pytypes.NodeDecorator):
    type="MESHING"
    label="Objects.Data.Mesh"
    insockets = [ ("Skeleton", "TRANSFORMABLE"),
                  ("Merge Joints", "BOOLEAN", False)]
    outsockets = [("Cylinder", "OBJECTDATA")]

class MeshNodeDecorator(PrimitiveNodeDecoratorBase):
    label="Objects.Primitives.Mesh"
    datanode = "Objects.Data.Mesh"

MT.registerNode(MeshDataNodeDecorator)

MT.registerNode(CubeDataNodeDecorator)
MT.registerNode(PlaneDataNodeDecorator)
MT.registerNode(CylinderDataNodeDecorator)
MT.registerNode(CubeNodeDecorator)
MT.registerNode(PlaneNodeDecorator)
MT.registerNode(IcosphereDataNodeDecorator)
MT.registerNode(IcosphereNodeDecorator)
MT.registerNode(CylinderNodeDecorator)

MT.registerNode(MeshNodeDecorator)
