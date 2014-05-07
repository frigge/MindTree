#include "boost/python.hpp"
#include "chrono"
#include "data/python/pyutils.h"
#include "data/nodes/node_db.h"
#include "data/cache_main.h"
#include "timeline.h"

Timer::Timer(int interval)
    : _interval(interval), 
    _running(false)
{
}

Timer::~Timer()
{
    stop();
}

void Timer::setInterval(int interval)
{
    _interval = interval;
}

bool Timer::isRunning()
{
    return _running;
}

void Timer::start() 
{
    if (_running) return;
    _running = true;

    auto fn = [this]() {
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(this->_interval));
            this->timerFinished();

            {
                std::lock_guard<std::mutex> guard(this->_running_mutex);
                if (!this->_running) break;
            }
        }
    };

    _thread = std::thread(fn);
}

void Timer::stop()
{

    {
        std::lock_guard<std::mutex> guard(_running_mutex);
        _running = false;
    }

    if(_thread.joinable())
        _thread.join();
}

int Timeline::_frame = 1;
int Timeline::_start = 1;
int Timeline::_end = 100;
int Timeline::_fps = 25;
Timer Timeline::_timer(1000/25);

namespace BPy = boost::python;

void Timeline::init()
{
    setFps(25);
    _timer.timerFinished.connect(Timeline::incFrame).detach();
}

void Timeline::setFrame(int frame)
{
    if (frame < _start || frame > _end || frame == _frame) return;
    _frame = frame;
    MT_CUSTOM_SIGNAL_EMITTER("frameChanged", _frame);
}

void Timeline::incFrame()
{
    _frame++;
    if (_frame > _end) _frame = _start;
    MT_CUSTOM_SIGNAL_EMITTER("frameChanged", _frame);
}

void Timeline::decFrame()
{
    _frame--;
    if (_frame < _start) _frame = _end;
    MT_CUSTOM_SIGNAL_EMITTER("frameChanged", _frame);
}

void Timeline::setStart(int start)
{
    _start = start;
}

void Timeline::setEnd(int end)
{
    _end = end;
}

void Timeline::setFps(int fps)
{
    _fps = fps;
    _timer.setInterval(1000/fps);
}

int Timeline::start()
{
    return _start;
}

int Timeline::end()
{
    return _end;
}

int Timeline::frame()
{
    return _frame;
}

int Timeline::fps()
{
    return _fps;
}

void Timeline::playpause()
{
    MindTree::Python::GILReleaser releaser;
    if (!_timer.isRunning()) {
        _timer.start();
    }
    else {
        _timer.stop();
    }
}

void Timeline::stop()
{
    MindTree::Python::GILReleaser releaser;
    _timer.stop();
    setFrame(_start);
}

bool Timeline::isPlaying()
{
    return _timer.isRunning();
}

TimelineNode::TimelineNode()
    : MindTree::DNode("Timeline")
{
    new MindTree::DoutSocket("Frame", "INTEGER", this);
    setNodeType("TIMELINE");
    auto cbhandler = MindTree::Signal::SignalHandler<int>
        ::handler.connect("frameChanged",
                          [this](int) { 
                              MT_CUSTOM_SIGNAL_EMITTER("nodeChanged", static_cast<DNode*>(this)); 
                          }); 

    callbacks.push_back(cbhandler);
}

TimelineNode::TimelineNode(const TimelineNode &other)
    : DNode(other)
{
}

TimelineNode::~TimelineNode()
{
}


BOOST_PYTHON_MODULE(mttimeline) {
    auto *timelineNodeFactory = 
        new MindTree::BuildInFactory("TIMELINE", 
                                     "Values.Frame", 
                                     []()->MindTree::DNode*{ return new TimelineNode(); });

    MindTree::NodeDataBase::registerNodeType(timelineNodeFactory);

    auto frameProc = [](MindTree::DataCache* cache) {
        std::cout <<  "caching current frame: " << Timeline::frame() << std::endl;
        cache->pushData(Timeline::frame());
    };

    MindTree::DataCache::addProcessor("INTEGER", 
                                      "TIMELINE", 
                                      new MindTree::CacheProcessor(frameProc));

    Timeline::init();

    BPy::def("frame", Timeline::frame);
    BPy::def("setFrame", Timeline::setFrame);
    BPy::def("start", Timeline::start);
    BPy::def("setStart", Timeline::setStart);
    BPy::def("end", Timeline::end);
    BPy::def("setEnd", Timeline::setEnd);
    BPy::def("fps", Timeline::fps);
    BPy::def("setFps", Timeline::setFps);
    BPy::def("isPlaying", Timeline::isPlaying);
    BPy::def("playpause", Timeline::playpause);
    BPy::def("stop", Timeline::stop);
}
