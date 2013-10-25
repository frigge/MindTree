#ifndef VIEWER_JMU26VVQ

#define VIEWER_JMU26VVQ

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
    void update_viewer(DinSocket*);
    virtual void update(DinSocket*);
    DoutSocket* getStart();
    void setStart(DoutSocket* value);
    QWidget* getWidget();
    void setWidget(QWidget* value);

protected:
    QWidget *widget;

private:
    DoutSocket *start;
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
