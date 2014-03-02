from floatcachetest import *
from stringcachetest import *
from connectsockets import *
from properties import *
from cpp_tests import *
from tollertest import *

def testAll():
    print("testing all tests ...")

    tests = [t for t in dir(__import__(__name__)) if t.startswith("test") and not t == "testAll"]
    numberOfTests = len(tests)
    successfulTests = 0

    for t in tests:
        print("\n-------------------------------------\n")
        print("testing %s\n" % t)
        print("___")
        success = eval(t).__call__()
        print("___")
        if success:
            successfulTests += 1
            print("%s passed" % t)
        else:
            print("%s failed" % t)
        print("\n-------------------------------------\n")

    print("%d out of %d tests were successful" % (successfulTests, numberOfTests))

    return successfulTests == numberOfTests
