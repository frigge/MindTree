import MT

class NodeFactory:
    type=""
    label=""

    insockets=[]
    outsockets=[]

    def __init__(self, node):
        node.name = self.label.split(".")[-1]

        for s in self.insockets:
            socket = node.addInSocket(s[0], s[1])
            try:
                socket.value = s[2]
            except IndexError:
                pass

        for s in self.outsockets:
            socket = node.addOutSocket(*s)

MT.pytypes.__dict__["NodeFactory"] = NodeFactory
