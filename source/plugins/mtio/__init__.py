from . import textio
from . import readtextnode
from . import objio
import MT

class Import3D(MT.pytypes.NodeDecorator):
    type="IMPORT3D"
    label="Objects.Import"
    insockets = [ ("Filepath", "DIRECTORY"),
                  ("Transformation", "MAT4")]
    outsockets = [("Scene", "TRANSFORMABLE")]

MT.registerNode(Import3D)
    
