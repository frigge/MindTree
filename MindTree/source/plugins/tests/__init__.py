from .floatcachetest import *
from .stringcachetest import *
from .connectsockets import *
from .properties import *
from .cpp_tests import *
from .timelinetests import *
from .project_tests import *

def all():
    print("testing all tests ...")

    tests = [t for t in dir(__import__(__name__)) if t.startswith("test")]
    numberOfTests = len(tests)
    failedTests = []

    for t in tests:
        print("\n_____________________________________")
        print("running test: %s\n" % t)
        success = False
        try:
            success = eval(t).__call__()
        except:
            pass
        if success:
            print("%s passed" % t)
        else:
            failedTests.append(t)
            print("%s failed" % t)
        print("_____________________________________\n")

    print("%d out of %d tests failed" % (len(failedTests), numberOfTests))

    print("\nfailed Tests:\n")
    for t in failedTests:
        print("\t" + t)

    return successfulTests == numberOfTests
