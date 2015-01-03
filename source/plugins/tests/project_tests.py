from .utils import *
import MT, os, datetime

def testNewProject():
    node = MT.createNode("Math.Add")
    node1 = MT.createNode("Values.Float Value")
    node2 = MT.createNode("Values.Int Value")

    MT.project.root.addNode(node)
    MT.project.root.addNode(node1)
    MT.project.root.addNode(node2)

    MT.newProject()

    try:
        print(node.name)
    except ReferenceError:
        return True
    return False

def testSaveProject():
    node = MT.createNode("Math.Add")
    node1 = MT.createNode("Values.Float Value")
    node2 = MT.createNode("Values.Int Value")

    MT.project.root.addNode(node)
    MT.project.root.addNode(node1)
    MT.project.root.addNode(node2)

    now = datetime.datetime.now()
    MT.project.filename = "projectTest" + now.strftime("%d%m%y%H%M%S") + ".mt"
    MT.project.save()

    return os.path.exists(MT.project.filename)

def testLoadProject():
    MT.newProject()
    node = MT.createNode("Math.Add")
    node1 = MT.createNode("Values.Float Value")
    node2 = MT.createNode("Values.Int Value")
    node3 = MT.createNode("General.Container")

    node.insockets[0].connected = node1.outsockets[0]

    node.pos = (0, 0)
    node1.pos = (0, 20)
    node2.pos = (0, 40)
    node3.pos = (0, 60)

    node4 = MT.createNode("Values.String Value")
    node5 = MT.createNode("Values.Vector3D")
    node6 = MT.createNode("Values.Vector2D")
    node7 = MT.createNode("Values.Color Value")

    node4.pos = (0, 0)
    node5.pos = (0, 20)
    node6.pos = (0, 40)
    node7.pos = (0, 60)

    MT.project.root.addNode(node)
    MT.project.root.addNode(node1)
    MT.project.root.addNode(node2)
    MT.project.root.addNode(node3)

    node3.graph.addNode(node4)
    node3.graph.addNode(node5)
    node3.graph.addNode(node6)
    node3.graph.addNode(node7)

    containerSize = len(MT.project.root[3].graph)

    node1.insockets[0].value = 5.7
    node2.insockets[0].value = 14

    now = datetime.datetime.now()
    filename =  "projectTest.mt"
    MT.project.filename = filename
    print("saving project: " + filename)
    MT.project.save()

    print()
    MT.newProject()
    print("open project: " + filename)
    MT.openProject(filename)

    node = MT.project.root[0]
    node1 = MT.project.root[1]
    node2 = MT.project.root[2]
    node3 = MT.project.root[3]

    node4 = MT.project.root[3].graph[2]
    node5 = MT.project.root[3].graph[3]
    node6 = MT.project.root[3].graph[4]
    node7 = MT.project.root[3].graph[5]

    success = (equal(len(MT.project.root), 4)
            and contains("Add", node.name)
            and contains("Float Value", node1.name)
            and contains("Int Value", node2.name)
            and contains("Container", node3.name)
            and equal((0, 0), node.pos)
            and equal((0, 20), node1.pos)
            and equal((0, 40), node2.pos)
            and equal((0, 60), node3.pos)
            and equal((0, 0), node4.pos)
            and equal((0, 20), node5.pos)
            and equal((0, 40), node6.pos)
            and equal((0, 60), node7.pos)
            and equal(node.insockets[0].connected, node1.outsockets[0])
            and equal(containerSize, 6)
            and equal(len(node3.graph), 6)
            and contains("String Value", node4.name)
            and contains("Vector3D", node5.name)
            and contains("Vector2D", node6.name)
            and contains("Color Value", node7.name)
            and equal(node2.insockets[0].value, 14)
            and floatEqual(node1.insockets[0].value, 5.7, 0.00001)
            )
    return success
