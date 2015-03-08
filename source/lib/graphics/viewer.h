#ifndef VIEWER_JMU26VVQ

#define VIEWER_JMU26VVQ

#include "data/cache_main.h"
#include "boost/python.hpp"
#include "thread"
#include "deque"
#include "mutex"
#include "atomic"

namespace BPy=boost::python;
class QWidget;
namespace MindTree
{
namespace Signal {
    class LiveTimeTracker;
}

    
class DoutSocket;
class DinSocket;

class Viewer;

class WorkerThread
{
public:

struct UpdateInfo {
    UpdateInfo(Viewer* viewer) : _viewer(viewer), _node(nullptr), _update(false) {}
    Viewer* _viewer;
    std::atomic<DNode*> _node;
    std::atomic<bool> _update;
};

    static bool needToUpdate();
    static void notifyUpdate(std::weak_ptr<UpdateInfo> info);
    static void removeViewer(Viewer *viewer);

    static void start();
    static void stop();

private:
    bool running();

    static std::thread _updateThread;
    static std::mutex _updateMutex;

    static std::vector<std::weak_ptr<UpdateInfo>> _updateQueue;
    static std::atomic<bool> _running;
};

class Viewer
{
public:
    Viewer(DoutSocket *socket);
    virtual ~Viewer();
    virtual void update()=0;
    DoutSocket* getStart();
    void setStart(DoutSocket* value);
    QWidget* getWidget();
    void setWidget(QWidget* value);

protected:
    QWidget *widget;
    MindTree::DataCache cache;

private:
    void update_viewer(DinSocket*);
    void update_viewer(DNode *node);

    DoutSocket *start;
    Signal::LiveTimeTracker *_signalLiveTime;
    std::vector<Signal::CallbackHandler> cbhandlers;
    std::shared_ptr<WorkerThread::UpdateInfo> _updateInfo;

    friend class WorkerThread;
};

class DoutSocketPyWrapper;
} /* MindTree */

#endif /* end of include guard: VIEWER_JMU26VVQ */
