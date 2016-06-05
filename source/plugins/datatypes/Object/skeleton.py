import MT

class JointNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Joint"
    type = "JOINTNODE"
    outsockets = [("Joint", "TRANSFORMABLE")]
    insockets = [("Transformation", "MAT4"),
                 ("Children", "LIST:TRANSFORMABLE")]

MT.registerNode(JointNodeDecorator)
