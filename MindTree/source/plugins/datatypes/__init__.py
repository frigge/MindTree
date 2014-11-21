from . import Object
from . import valuenodes
from . import generic_processor
from . import converternodes
import MT

class SinNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Math.Sin"
    type = "SIN"
    insockets = [("Value", "FLOAT")]
    outsockets = [("Result", "FLOAT")]

class IfNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Condition.If"
    type = "IFNODE"
    insockets = [("Condition", "BOOLEAN"),
            ("Then", "VARIABLE"),
            ("Else", "VARIABLE")]
    outsockes = [("Output", "VARIABLE")]

class AndNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Condition.And"
    type = "ANDNODE"
    outsockets = [("Output", "BOOLEAN")]

class OrNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Condition.Or"
    type = "ORNODE"
    outsockets = [("Output", "BOOLEAN")]

class NotNodeDecorator(MT.pytypes.NodeDecorator):
    label = "Condition.Or"
    type = "ORNODE"
    insockets = [("Input", "BOOLEAN")]
    outsockets = [("Output", "BOOLEAN")]

MT.addCompatibility("FLOAT", "INTEGER")

MT.registerNode(SinNodeDecorator)
MT.registerNode(IfNodeDecorator)
MT.registerNode(AndNodeDecorator)
MT.registerNode(OrNodeDecorator)
MT.registerNode(NotNodeDecorator)
