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

    success = (equal(len(MT.project.root), 4)
            and contains("Add", MT.project.root[0].name)
            and contains("Float Value", MT.project.root[1].name)
            and contains("Int Value", MT.project.root[2].name)
            and contains("Container", MT.project.root[3].name)
            and contains("String Value", MT.project.root[3].graph[0].name)
            and contains("Vector3D", MT.project.root[3].graph[1].name)
            and contains("Vector2D", MT.project.root[3].graph[2].name)
            and contains("Color Value", MT.project.root[3].graph[3].name)
            and equal(MT.project.root[2].insockets[0].value, 14)
            and floatEqual(MT.project.root[1].insockets[0].value, 5.7, 0.00001)
            )
    return success
