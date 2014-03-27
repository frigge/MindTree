#ifndef PYWINDOWLIST_XQ23SSJ1

#define PYWINDOWLIST_XQ23SSJ1

#include "boost/python.hpp"
#include "data/windowfactory.h"
#include "graphics/viewer.h"

namespace BPy = boost::python;
namespace MindTree
{
class DoutSocketPyWrapper;
class DoutSocket;
class ViewerDockBase;
namespace Python
{
    
class PythonWindowFactory : public WindowFactory
{
public:
    PythonWindowFactory(QString name, BPy::object cls);
    virtual ~PythonWindowFactory();
    MindTree::ViewerDockBase* createWindow();
    QString showWindow();

private:
    BPy::object cls;
};

class PythonViewerFactory : public ViewerFactory
{
public:
    PythonViewerFactory(QString name, QString type, BPy::object cls);
    virtual ~PythonViewerFactory();
    ViewerDockBase* createViewer(DoutSocket *socket);

private:
    BPy::object cls;
};

void wrapViewerFunctions();
void regWindow(QString name, BPy::object windowClass);
void regViewer(QString name, QString type, BPy::object viewerClass);
BPy::dict getViewers();
BPy::list getWindows();
QString showViewer(DoutSocketPyWrapper *pyout, unsigned int index);
QString showWindow(QString name);
QString showSplitWindow(QString name, QString other, QString orientation, float ratio);
QString showTabbedWindow(QString name, QString other);

} /* Python */

class PyViewerBase : public Viewer, public BPy::wrapper<Viewer>
{
public:
    PyViewerBase(DoutSocketPyWrapper *start);
    virtual ~PyViewerBase();
    static void wrap();
    void update(DinSocket *socket);
    void setWidget(BPy::object widget);
    DoutSocketPyWrapper* getSocket();
    DataCache getCache() const;

private:
    BPy::object widget;
};

} /* MindTree */
#endif /* end of include guard: PYWINDOWLIST_XQ23SSJ1 */
