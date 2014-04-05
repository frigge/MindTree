#ifndef WINDOWLIST_X4UACIPQ

#define WINDOWLIST_X4UACIPQ

#include "QObject"
#include "QList"
#include "map"
#include "vector"
#include "unordered_map"

class QDockWidget;
class QMainWindow;

namespace MindTree
{
class WindowFactory;
class ViewerDockBase;
    
class WindowList : public QObject
{
    Q_OBJECT
public:
    void addFactory(WindowFactory* value);
    QList<WindowFactory*> getFactories();

    void setMainWindow(QMainWindow *mw);

    void showDock(QDockWidget *widget);
    bool isRegistered(QString name);

    QString showWindow(QString name);
    QString showSplitWindow(QString name, QString other, Qt::Orientation orientation, float ratio);
    QString showTabbedWindow(QString name, QString other);

    MindTree::ViewerDockBase* createWindow(QString name);

    static WindowList* instance();

signals:
    void windowFactoryAdded(MindTree::WindowFactory*);

private:
    //singleton
    WindowList();

    static WindowList *windowList;
    QMainWindow *window;
    QList<WindowFactory*> windowFactories;
};

class ViewerFactory;
class DoutSocket;
class ViewerList : public QObject
{
    Q_OBJECT
public:
    void addViewer(ViewerFactory* value);
    std::map<std::string, std::vector<ViewerFactory*>> getFactories();

    bool isRegistered(std::string name, std::string type);
    QString showViewer(DoutSocket *socket, unsigned int index);
    void showDock(QDockWidget *widget);

    static ViewerList* instance();
    void setMainWindow(QMainWindow *mw);

private:
    //Singleton
    ViewerList();

    static ViewerList* viewerList;
    QMainWindow *window;
    std::unordered_map<std::string, ViewerDockBase*> openViewers;

    std::map<std::string, std::vector<ViewerFactory*>> windowFactories;
};
} /* MindTree */

#endif /* end of include guard: WINDOWLIST_X4UACIPQ */
