import MT

class CreatePCInPlane(MT.pytypes.NodeDecorator):
    insockets = [
        ("Width", "FLOAT", 100.0),
        ("Height", "FLOAT", 100.0),
        ("Number of Points", "FLOAT", 1000.0),
        ]
    outsockets = [ ("Pointcloud", "TRANSFORMABLE") ]
    type = "CREATEPCPLANE"
    label = "Objects.Create Pointcloud in Plane"

MT.registerNode(CreatePCInPlane)
