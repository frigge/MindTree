import sys, traceback

class TestCase:
    def __init__(self):
        self.success = True
        self.exceptions = []

    def exit(self):
        if len(self.exceptions) > 0:
            print("Test failed, following exceptions were thrown:")
            for exc in exceptions:
                traceback.print_exception(*exc)

        return self.success

    def _check(self, fn, *args):
        success = False
        try:
            success = fn(*args)
        except:
            self.exceptions.append(sys.exc_info())
        return success

    def equal(self, a, b, msg="test"):
        succ = self._check(self._equal, a, b)
        if succ:
            self.success = self.success and True
        else:
            print("{}: comparison of {} and {} failed".format(msg, a, b))
            self.success = False
        return succ

    def contains(self, a, b, msg="test"):
        succ = self._check(self._contains, a, b)
        if succ:
            self.success = self.success and True
        else:
            print("{}: {} not found in {}".format(msg, a, b))
            self.success = False
        return succ

    def floatEqual(self, a, b, tol, msg="test"):
        succ = self._check(self.floatEqual, a, b, tol)
        if succ:
            self.success = self.success and True
        else:
            print("{}: comparison of {} and {} failed".format(msg, a, b))
            self.success = False
        return succ


    def _equal(self, a, b):
        return a == b

    def _contains(self, a, b):
        return a in b

    def _floatEqual(self, a, b, tol):
        return abs(a - b) < tol

def equal(a, b):
    if a == b:
        return True
    else:
        print("comparison of {} and {} failed".format(a, b))
        return False

def contains(a, b):
    if a in b:
        return True
    else:
        print("{} not found in {}".format(a, b))
        return False

def floatEqual(a, b, tol):
    if abs(a - b) < tol:
        return True
    else:
        print("comparison of {} and {} failed")
        return False
