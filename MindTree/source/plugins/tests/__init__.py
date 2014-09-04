from .floatcachetest import *
from .stringcachetest import *
from .connectsockets import *
from .properties import *
from .cpp_tests import *
from .timelinetests import *
from .project_tests import *
from .datacachetests import *

import sys, traceback

def all():
    print("testing all tests ...")

    tests = [t for t in dir(__import__(__name__)) if t.startswith("test")]
    numberOfTests = len(tests)
    failedTests = []
    exceptions = []

    for t in tests:
        print("\n_____________________________________")
        print("running test: %s\n" % t)
        success = False
        try:
            success = eval(t).__call__()
        except:
            exceptions.append(sys.exc_info())
        if success:
            print("%s passed" % t)
        else:
            failedTests.append(t)
            print("%s failed" % t)
        print("_____________________________________\n")

    if len(failedTests) > 0:
        print("%d out of %d tests failed" % (len(failedTests), numberOfTests))
        print("\nfailed Tests:\n")
        for t in failedTests:
            print("\t" + t)

        print("Error info:")
        for e in exceptions:
            traceback.print_exception(*e)
    else:
            print("All tests passed")

    return not len(failedTests)
