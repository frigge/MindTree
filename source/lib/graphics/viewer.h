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
    static bool needToUpdate();
    static void notifyUpdate(Viewer *viewer);
    static void removeViewer(Viewer *viewer);

    static void start();
    static void stop();

private:
    bool running();

    static std::thread _updateThread;
    static std::mutex _updateMutex;

    static std::deque<Viewer*> _updateQueue;
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

    friend class WorkerThread;
};

class DoutSocketPyWrapper;
} /* MindTree */

#endif /* end of include guard: VIEWER_JMU26VVQ */
