import MT

def createContainer(nodes):
    space = nodes[0].space

    inconnections = {}
    outconnections = {}

    container = MT.createNode("General.Container")
    ci = container.graph[0]
    co = container.graph[1]

    middle = (0, 0)
    for n in nodes:
        middle = (middle[0] + n.pos[0], middle[1] + n.pos[1])

        #find ingoing node connections
        for socket in n.insockets:
            if not socket.connected:
                continue

            cntd = socket.connected
            if not cntd.node in nodes:
                inconnections[socket] = cntd

        #find outgoing node connections
        for socket in n.outsockets:
            if len(socket.cntdSockets) == 0:
                continue
            for s in socket.cntdSockets:
                if not s.node in nodes:
                    outconnections[s] = socket

        #remove nodes from current space
        space.removeNode(n)

        container.graph.addNode(n)

    space.addNode(container)

    #reconnect incoming and outgoing connections
    for isocket, osocket in inconnections.items():
        isocket.connected = ci.outsockets[-1]
        container.insockets[-1].connected = osocket

    for isocket, osocket in outconnections.items():
        co.insockets[-1].connected = osocket
        isocket.connected = container.outsockets[-1]

    middle = (middle[0] / 2, middle[1] / 2)

    container.pos = middle

def extractContainer(node):
    pass

MT.__dict__["createContainer"] = createContainer
MT.__dict__["extractContainer"] = extractContainer
