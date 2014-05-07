#ifndef VIEWER_JMU26VVQ

#define VIEWER_JMU26VVQ

#include "data/cache_main.h"
#include "boost/python.hpp"

namespace BPy=boost::python;
class QWidget;
namespace MindTree
{
    
class DoutSocket;
class DinSocket;
class Viewer
{
public:
    Viewer(DoutSocket *socket);
    virtual ~Viewer();
    virtual void update(DinSocket*)=0;
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
    std::vector<Signal::CallbackHandler> cbhandlers;
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
