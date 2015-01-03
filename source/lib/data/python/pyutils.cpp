#include "pyutils.h"

using namespace MindTree::Python;

GILLocker::GILLocker()
{
    state = PyGILState_Ensure();
}

GILLocker::~GILLocker()
{
    PyGILState_Release(state);
}

GILReleaser::GILReleaser()
{
    state = PyEval_SaveThread();
}

GILReleaser::~GILReleaser()
{
    PyEval_RestoreThread(state);
}
