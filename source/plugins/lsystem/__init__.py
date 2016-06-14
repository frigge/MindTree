import MT

class LSystemNodeDecorator(MT.pytypes.NodeDecorator):
    label = "LSystem"
    type = "LSYSTEMNODE"
    insockets = [
        ("Axion", "STRING"),
        ("Rules", "LIST:STRING"),
        ("Transformations", "LIST:TRANSFORMABLE")
    ]
    outsockets = [("Skeleton", "TRANSFORMABLE")]

MT.registerNode(LSystemNodeDecorator)
