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
