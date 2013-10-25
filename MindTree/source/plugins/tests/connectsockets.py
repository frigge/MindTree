import MT

success = False
def testConnectSockets():
    add = MT.createNode("Math.Add")
    add2 = MT.createNode("Math.Add")

    MT.project.root.addNode(add)
    MT.project.root.addNode(add2)

    print("{0} is connected to {1}".format(add.insockets[0], add.insockets[0].connected))

    print("connecting insocket0 of Add to outsocket0 of Add1")

    def test(socket):
        global success
        print("connected successfully {0} to {1}".format(socket, socket.connected))
        success = True

    cb = MT.attachToSignal("createLink", test)
    add.insockets[0].connected = add2.outsockets[0]

    return success;
