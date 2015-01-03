#ifndef MT_PYUTILS_H
#define MT_PYUTILS_H

#include "Python.h"

namespace MindTree {
namespace Python {
class GILLocker
{
public:
    GILLocker();
    ~GILLocker();
private:
    PyGILState_STATE state;
};

class GILReleaser
{
public:
    GILReleaser();
    ~GILReleaser();

private:
    PyThreadState* state;

};

}
}

#endif
