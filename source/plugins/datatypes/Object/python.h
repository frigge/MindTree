#ifndef PYTHON_VQNZMITG

#define PYTHON_VQNZMITG

#include "data/python/wrapper.h"

class ObjectData;
class GeoObject;
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
    ObjectPyWrapper(GeoObject *obj);
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

namespace MindTree { class DataCache; }
void parentProc(MindTree::DataCache *cache);
void groupProc(MindTree::DataCache *cache);
void transformProc(MindTree::DataCache *cache);

#endif /* end of include guard: PYTHON_VQNZMITG */
