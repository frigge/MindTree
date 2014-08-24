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

    MT.project.root.addNode(node)
    MT.project.root.addNode(node1)
    MT.project.root.addNode(node2)

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

    success = (equal(len(MT.project.root), 3)
            and contains("Add", MT.project.root[0].name)
            and contains("Float Value", MT.project.root[1].name)
            and contains("Int Value", MT.project.root[2].name)
            and equal(MT.project.root[2].insockets[0].value, 14)
            and floatEqual(MT.project.root[1].insockets[0].value, 5.7, 0.00001)
            )
    return success
