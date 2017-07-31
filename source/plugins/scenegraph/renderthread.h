#ifndef MT_RENDERTHREAD_H
#define MT_RENDERTHREAD_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <QOpenGLContext>
#include <QObject>
#include <QThread>

namespace MindTree
{
namespace GL
{
class RenderTree;

class RenderThread : public QObject
{
	Q_OBJECT
public:
    void addTree(RenderTree *tree, QOpenGLContext *context);
    void removeTree(RenderTree *tree);
    void update();
    void updateOnce();
    void pause();

	Q_SLOT void render();

private:
    void stop();
    bool isRendering();

    std::atomic_bool _rendering{false};
    std::atomic_bool _update{false};
    std::condition_variable _renderNotifier;
    std::mutex _renderingLock;
    QThread _renderThread;
	QOpenGLContext *ctx_;
    std::vector<RenderTree*> _renderQueue;
};

}
}

#endif // MT_RENDERTHREAD_H
