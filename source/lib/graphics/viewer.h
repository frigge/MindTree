#ifndef VIEWER_JMU26VVQ

#define VIEWER_JMU26VVQ

#include "data/cache_main.h"
#include "boost/python.hpp"
#include "thread"
#include "deque"
#include "mutex"
#include "condition_variable"
#include "memory"
#include "atomic"

namespace BPy=boost::python;
class QWidget;
namespace MindTree
{
namespace Signal {
    class LiveTimeTracker;
}

class Viewer;
struct UpdateInfo;

class WorkerThread
{
public:
    static bool needToUpdate();
    static void notifyUpdate();
    static void removeViewer(Viewer *viewer);
    static void update(UpdateInfo info);

    static void start();
    static void stop();

private:
    bool running();

    static std::thread _updateThread;
    static std::mutex _updateMutex;
    static std::condition_variable _needToUpdateCondition;

    static std::vector<UpdateInfo> _updateQueue;
    static std::atomic<uint32_t> _updateQueueSize;
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
    DNode* getSettings();
    void initBase();

protected:
    virtual void init() = 0;
    QWidget *widget;
    MindTree::DataCache dataCache;
    MindTree::DataCache settingsCache;
    NodePtr _settingsNode;

private:
    void update_viewer(DinSocket*);
    void update_viewer(DNode *node);

    void cacheAndUpdate();

    DoutSocket *start;
    Signal::LiveTimeTracker *_signalLiveTime;
    std::vector<Signal::CallbackHandler> cbhandlers;

    friend class WorkerThread;
};

class DoutSocketPyWrapper;
} /* MindTree */

#endif /* end of include guard: VIEWER_JMU26VVQ */
