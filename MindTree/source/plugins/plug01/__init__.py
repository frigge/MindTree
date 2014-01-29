import MT

class TestNode:
    type="SOMETYPE"
    label="Some Label"

class TestNode2:
    type="SOMEOTHERTYPE"
    label="SOMEOTHERLABEL"

MT.registerNode("Lottis Nodes", "Toller Node", TestNode)
MT.registerNode("Lottis Nodes", "Toller Node", TestNode2)
