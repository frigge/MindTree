#include <QThread>
#include "../render/glwrapper.h"
#include "qtcontext.h"
#include "../render/rendertree.h"
#include <QOpenGLContext>

#include "renderthread.h"

using namespace MindTree;
using namespace MindTree::GL;

void RenderThread::addTree(RenderTree* tree, QOpenGLContext *context)
{
    std::unique_lock<std::mutex> lock(_renderingLock, std::defer_lock);
    auto it = std::find(begin(_renderQueue), end(_renderQueue), tree);
    if(it == end(_renderQueue))
        _renderQueue.push_back(tree);

    if(!isRendering()) {
		moveToThread(&_renderThread);
		context->moveToThread(&_renderThread);
		connect(&_renderThread, &QThread::started, this, &RenderThread::render);
		_renderThread.start();
	}
    _update = true;
}

void RenderThread::removeTree(RenderTree *tree)
{
    std::unique_lock<std::mutex> lock(_renderingLock, std::defer_lock);
    auto it = std::find(begin(_renderQueue), end(_renderQueue), tree);
    if(it != end(_renderQueue))
        _renderQueue.erase(it);
    if(_renderQueue.empty()) stop();
}

bool RenderThread::isRendering()
{
    return _rendering;
}

void RenderThread::updateOnce()
{
    _update = true;
    _renderNotifier.notify_all();
    _update = false;
}

void RenderThread::update()
{
    //noop if already updating
    if(_update) return;

    _update = true;
    _renderNotifier.notify_all();
}

void RenderThread::pause()
{
    _update = false;
}

void RenderThread::render()
{
    if(isRendering()) stop();
	std::unique_lock<std::mutex> lock(_renderingLock, std::defer_lock);
    _rendering = true;
	QtContext ctx(ctx_);
	ContextBinder binder(ctx);
	while(isRendering()) {
		if(!_update) _renderNotifier.wait(lock);
		for(auto *tree : _renderQueue) {
			tree->draw();
			ctx.swapBuffers();
		}
	}
}

void RenderThread::stop()
{
    std::cout << "stop rendering" << std::endl;
    _rendering = false;
    _update = false;
    _renderNotifier.notify_all();
	_renderThread.quit();
	_renderThread.wait();
}
