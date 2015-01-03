#ifndef MINDTREE_TIME_H
#define MINDTREE_TIME_H

#include "thread"
#include "mutex"
#include "data/nodes/data_node.h"
#include "data/signal.h"

class Timer
{
public:
    Timer(int interval);
    ~Timer();

    void start();
    void stop();

    void setInterval(int interval);
    bool isRunning();

    MindTree::Signal::Signal<> timerFinished;

private:
    int _interval;
    bool _running;
    std::thread _thread;
    std::mutex _running_mutex;
};

class Timeline 
{
public:
    static void init();
    static void setFrame(int frame);
    static void setStart(int start);
    static void setEnd(int end);
    static void setFps(int fps);
    static int start();
    static int end();
    static int frame();
    static int fps();
    static void incFrame();
    static void decFrame();

    static void playpause();
    static void stop();
    static bool isPlaying();

private:
    static int _frame, _start, _end, _fps;
    static Timer _timer;
};

class TimelineNode : public MindTree::DNode
{
public:
    TimelineNode(bool raw = false);
    TimelineNode(const TimelineNode &other);
    ~TimelineNode();

private:
    MindTree::Signal::CallbackVector callbacks;
};

namespace Python {
class Timeline 
{
public:
    static void setFrame(int frame);
    static int frame();
    static void setStart(int start);
    static int start();
    static void setEnd(int end);
    static int end();
    static void setFps(int fps);
    static int fps();

    static void playpause();
    static void stop();
    static bool isPlaying();
    static void registerAPI();
};
};
#endif
