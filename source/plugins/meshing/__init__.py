import MT

class SubdNodeDecorator(MT.pytypes.NodeDecorator):
    type="SUBDIVISIONSURFACE"
    label="Objects.Data.Subdivision"
    insockets = [ ("Mesh", "OBJECTDATA"),
                  ("Iterations", "INTEGER", 1)]

    outsockets = [("Subd", "OBJECTDATA")]

MT.registerNode(SubdNodeDecorator)
