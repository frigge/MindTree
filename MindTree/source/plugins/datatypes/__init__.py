import Object
import valuenodes
import generic_processor
import converternodes
import MT

class SinNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Math.Sin"
    type = "SIN"
    insockets = [("Value", "FLOAT")]
    outsockets = [("Result", "FLOAT")]

MT.addCompatibility("FLOAT", "INTEGER")
MT.registerNode(SinNodeDecorator)
