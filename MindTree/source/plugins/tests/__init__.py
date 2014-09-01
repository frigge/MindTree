from .floatcachetest import *
from .stringcachetest import *
from .connectsockets import *
from .properties import *
from .cpp_tests import *
from .timelinetests import *
from .project_tests import *
from .datacachetests import *

def testAll():
    print("testing all tests ...")

    tests = [t for t in dir(__import__(__name__)) if t.startswith("test") and not t == "testAll"]
    numberOfTests = len(tests)
    successfulTests = 0
    failedTests = []

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
            failedTests.append(t)
            print("%s failed" % t)
        print("\n-------------------------------------\n")

    print("%d out of %d tests were successful" % (successfulTests, numberOfTests))

    print("\nfailed Tests:\n")
    for t in failedTests:
        print(t)

    print("___")

    return successfulTests == numberOfTests
