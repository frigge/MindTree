#ifndef PYTHON_VQNZMITG

#define PYTHON_VQNZMITG

#include "data/python/wrapper.h"

class ObjectData;
class Object;
class Group;
class ObjectDataPyWrapper : public MindTree::PyWrapper
{
public:
    ObjectDataPyWrapper(ObjectData *data);
    virtual ~ObjectDataPyWrapper();

    static void wrap();
};

class ObjectPyWrapper : public MindTree::PyWrapper
{
public:
    ObjectPyWrapper(Object *obj);
    virtual ~ObjectPyWrapper();

    static void wrap();
};

class GroupPyWrapper : public MindTree::PyWrapper
{
public:
    GroupPyWrapper(Group *grp);
    virtual ~GroupPyWrapper();

    static void wrap();
};

#endif /* end of include guard: PYTHON_VQNZMITG */
