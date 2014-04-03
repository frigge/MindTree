import MT

class AddPropertiesNodeFactory(MT.pytypes.NodeFactory):
    type="ADDPROPERTIES"
    label="Objects.Add Properties"
    insockets = [("Object(s)", "GROUPDATA"),
            ("Property Name", "STRING"),
            ("Property Value", "VARIABLE")]
    outsockets = [("Object(s)", "GROUPDATA")]

class FilterObjectsNodeFactory(MT.pytypes.NodeFactory):
    type="FILTEROBJECTS"
    label="Objects.Filter"
    insockets = [("Objects", "GROUPDATA"),
            ("Name Regex", "STRING")]
    outsockets = [("Objects", "GROUPDATA")]


def registerNodes():
    MT.registerNode(AddPropertiesNodeFactory)
    MT.registerNode(FilterObjectsNodeFactory)

registerNodes()
