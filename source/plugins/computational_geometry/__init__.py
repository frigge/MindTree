import MT

class CreatePCInPlane(MT.pytypes.NodeDecorator):
    insockets = [
        ("Width", "FLOAT", 100.0),
        ("Height", "FLOAT", 100.0),
        ("Number of Points", "INTEGER", 1000),
        ]
    outsockets = [ ("Pointcloud", "TRANSFORMABLE") ]
    type = "CREATEPCPLANE"
    label = "Objects.Create Pointcloud in Plane"

class ScatterPointsOnSurface(MT.pytypes.NodeDecorator):
    type = "SCATTERSURFACE"
    label = "Objects.Scatter Surface"
    insockets = [
        ("Object", "TRANSFORMABLE"),
        ("count", "INTEGER", 100)
    ]
    outsockets = [ ("Pointcloud", "TRANSFORMABLE") ]

MT.registerNode(CreatePCInPlane)
MT.registerNode(ScatterPointsOnSurface)
