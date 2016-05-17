import MT

class JointNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Joint"
    type = "JOINTNODE"
    outsockets = [("Joint", "JOINT")]
    insockets = [("Transformation", "MAT4"),
                 ("Children", "LIST:JOINT")]
    
class SkeletonNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Objects.Skeleton"
    type = "SKELETONNODE"
    outsockets = [("Skeleton", "SKELETON")]
    insockets  = [("RootJoint", "JOINT")]
