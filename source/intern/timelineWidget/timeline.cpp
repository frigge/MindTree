#include "boost/python.hpp"
#include "chrono"
#include "data/python/pyutils.h"
#include "data/nodes/node_db.h"
#include "data/cache_main.h"
#include "data/debuglog.h"
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

std::atomic<int> Timeline::_frame{1};
std::atomic<int> Timeline::_start{1};
std::atomic<int> Timeline::_end{100};
std::atomic<int> Timeline::_fps{25};
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
    MT_CUSTOM_SIGNAL_EMITTER("frameChanged", _frame.load());
}

void Timeline::incFrame()
{
    _frame++;
    if (_frame > _end) _frame = _start.load();
    MT_CUSTOM_SIGNAL_EMITTER("frameChanged", _frame.load());
}

void Timeline::decFrame()
{
    _frame--;
    if (_frame < _start) _frame = _end.load();
    MT_CUSTOM_SIGNAL_EMITTER("frameChanged", _frame.load());
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
    if (!_timer.isRunning()) {
        _timer.start();
    }
    else {
        _timer.stop();
    }
}

void Timeline::stop()
{
    _timer.stop();
    setFrame(_start);
}

bool Timeline::isPlaying()
{
    return _timer.isRunning();
}

void Python::Timeline::setFrame(int frame)
{
    MindTree::Python::GILReleaser releaser;
    ::Timeline::setFrame(frame);
}

void Python::Timeline::setStart(int start)
{
    MindTree::Python::GILReleaser releaser;
    ::Timeline::setStart(start);
}

void Python::Timeline::setEnd(int end)
{
    MindTree::Python::GILReleaser releaser;
    ::Timeline::setEnd(end);
}

void Python::Timeline::setFps(int fps)
{
    MindTree::Python::GILReleaser releaser;
    ::Timeline::setFps(fps);
}

int Python::Timeline::start()
{
    MindTree::Python::GILReleaser releaser;
    return ::Timeline::start();
}

int Python::Timeline::end()
{
    MindTree::Python::GILReleaser releaser;
    return ::Timeline::end();
}

int Python::Timeline::frame()
{
    MindTree::Python::GILReleaser releaser;
    return ::Timeline::frame();
}

int Python::Timeline::fps()
{
    MindTree::Python::GILReleaser releaser;
    return ::Timeline::fps();
}

void Python::Timeline::playpause()
{
    MindTree::Python::GILReleaser releaser;
    ::Timeline::playpause();
}

void Python::Timeline::stop()
{
    MindTree::Python::GILReleaser releaser;
    ::Timeline::stop();
}

bool Python::Timeline::isPlaying()
{
    MindTree::Python::GILReleaser releaser;
    return ::Timeline::isPlaying();
}

void Python::Timeline::registerAPI()
{
    BPy::def("frame", Python::Timeline::frame);
    BPy::def("setFrame", Python::Timeline::setFrame);
    BPy::def("start", Python::Timeline::start);
    BPy::def("setStart", Python::Timeline::setStart);
    BPy::def("end", Python::Timeline::end);
    BPy::def("setEnd", Python::Timeline::setEnd);
    BPy::def("fps", Python::Timeline::fps);
    BPy::def("setFps", Python::Timeline::setFps);
    BPy::def("isPlaying", Python::Timeline::isPlaying);
    BPy::def("playpause", Python::Timeline::playpause);
    BPy::def("stop", Python::Timeline::stop);
}

TimelineNode::TimelineNode(bool raw)
    : MindTree::DNode("Timeline")
{
    if(!raw)
        new MindTree::DoutSocket("Frame", "INTEGER", this);

    setType("TIMELINE");
    auto cbhandler = MindTree::Signal::SignalHandler<int>
        ::handler.connect("frameChanged",
                          [this](int) {
                              MT_CUSTOM_SIGNAL_EMITTER("nodeChanged",
                                                       static_cast<DNode*>(this));
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
    auto timelineNodeDecorator =
        std::make_unique<MindTree::BuildInDecorator>("TIMELINE",
                                     "Values.Frame",
                            [](bool raw){
                                    return std::make_shared<TimelineNode>(raw);
                            });

    MindTree::NodeDataBase::registerNodeType(std::move(timelineNodeDecorator));

    auto frameProc = [](MindTree::DataCache* cache) {
        cache->pushData(Timeline::frame());
    };

    MindTree::DataCache::addProcessor(new MindTree::CacheProcessor("INTEGER",
                                                                   "TIMELINE",
                                                                   frameProc));

    Timeline::init();
    Python::Timeline::registerAPI();
}
