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

class Export3D(MT.pytypes.NodeDecorator):
    type="EXPORT3D"
    label="Objects.Export"
    insockets = [ ("Filepath", "SAVEFILE"), ("Scene", "TRANSFORMABLE") ]
    outsockets = [("Export", "ACTION")]

MT.registerNode(Import3D)
MT.registerNode(Export3D)
    
