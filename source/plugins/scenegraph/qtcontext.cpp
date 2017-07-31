#include <QThread>
#include <QSurfaceFormat>
#include <QOpenGLContext>

#include "qtcontext.h"

using namespace MindTree;
using namespace MindTree::GL;

QtContext::QtContext(QOpenGLContext *context) :
    _context(context)
{
}

void QtContext::makeCurrent()
{
    if(!_context->makeCurrent(_context->surface()))
	{
		_context->create();
		_context->makeCurrent(_context->surface());
	}
}

void QtContext::doneCurrent()
{
    _context->doneCurrent();
}

void QtContext::swapBuffers()
{
    _context->swapBuffers(_context->surface());
}

QSurfaceFormat QtContext::format()
{
    QSurfaceFormat format;
    //format.setAlphaBufferSize(true);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    format.setVersion(4, 3);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    return format;
}

ContextBinder::ContextBinder(QtContext context)
    : _context(context)
{
	context.makeCurrent();
}
