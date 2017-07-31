#ifndef MT_QTCONTEXT_H
#define MT_QTCONTEXT_H

#include <memory>

class QOpenGLContext;
class QSurfaceFormat;
class QSurface;

namespace MindTree {
namespace GL {

struct QtContext
{
public:
    QtContext(QOpenGLContext *context);

    void makeCurrent();
    void doneCurrent();
    void swapBuffers();

    static QSurfaceFormat format();
    QOpenGLContext *_context;
};

class ContextBinder
{
public:
    ContextBinder(QtContext context);

private:
    QtContext _context;
};

}
}

#endif
