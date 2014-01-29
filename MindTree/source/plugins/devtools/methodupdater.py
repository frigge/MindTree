import gc, inspect, MT

def updateModule(module, path):
    reload(module)
    print("loading objects from: " + path)
    for cls in module.__dict__.values():
        if inspect.isclass(cls) and not cls.__name__.startswith("__"):
            __update_objects(cls, path)

def __update_objects(cls, path):
    print("update class: "+ cls.__name__)
    for obj in eval(path):
        if obj.__class__.__name__ == cls.__name__:
            obj.__class__ = cls
           # __updateObject(obj, cls)

def __updateObject(obj, cls):
    #l = list(set(dir(cls)) - set(dir(obj)))
    print("updating object ...")
    l = dict(inspect.getmembers(cls, inspect.ismethod)).keys()
    print("list of methods to update:", l)
    for m in inspect.getmembers(cls, inspect.ismethod):
        setattr(obj.__class__, m[0], m[1])

