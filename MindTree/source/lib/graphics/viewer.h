#ifndef VIEWER_JMU26VVQ

#define VIEWER_JMU26VVQ

#include "data/cache_main.h"
#include "boost/python.hpp"
#include "thread"
#include "mutex"

namespace BPy=boost::python;
class QWidget;
namespace MindTree
{
namespace Signal {
    class LiveTimeTracker;
}

    
class DoutSocket;
class DinSocket;
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
    bool needToUpdate();
    void notifyUpdate();
    bool running();
    void update_viewer(DinSocket*);
    void update_viewer(DNode *node);

    DoutSocket *start;
    Signal::LiveTimeTracker *_signalLiveTime;
    std::vector<Signal::CallbackHandler> cbhandlers;
    bool _needToUpdate;
    bool _running;
    std::thread _updateThread;
    std::mutex _updateMutex;
    std::mutex _runningMutex;
};

class DoutSocketPyWrapper;
} /* MindTree */

//namespace boost{
//template<typename T>
//T* get_pointer(std::shared_ptr<T> &ptr){
//    return ptr.get();
//}
//
//template<typename T>
//T* get_pointer(std::shared_ptr<T> const &ptr){
//    return ptr.get();
//}
//}

#endif /* end of include guard: VIEWER_JMU26VVQ */
