import MT

class ReadTextNode(MT.pytypes.NodeFactory):
    type = "TEXTREAD"
    label = "IO.Read Text"
    insockets = [("Filename", "DIRECTORY"),
                ("Watch File", "BOOLEAN")]
    outsockets = [("File Content", "STRING")]

MT.registerNode(ReadTextNode)
