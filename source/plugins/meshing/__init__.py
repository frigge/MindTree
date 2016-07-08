import MT

class SubdNodeDecorator(MT.pytypes.NodeDecorator):
    type="SUBDIVISIONSURFACE"
    label="Objects.Data.Subdivision"
    insockets = [ ("Mesh", "OBJECTDATA"),
                  ("Iterations", "INTEGER", 1)]

    outsockets = [("Subd", "OBJECTDATA")]

class FilterPolygonDecorator(MT.pytypes.NodeDecorator):
    type="FILTERPOLYGON"
    label="Objects.Data.Filter Polygon"
    insockets = [ ("Mesh", "OBJECTDATA"),
                  ("Name", "STRING"),
                  ("Upper Limit", "FLOAT", 1.0),
                  ("Lower Limit", "FLOAT", 0.0)]

    outsockets = [("Filtered", "OBJECTDATA")]

MT.registerNode(SubdNodeDecorator)
