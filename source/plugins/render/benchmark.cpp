#include "iomanip"
#include "algorithm"
#include "benchmark.h"

using namespace MindTree;

BenchmarkHandler::BenchmarkHandler(std::weak_ptr<Benchmark> benchmark) :
    _benchmark(benchmark)
{
    if(_benchmark.expired()) return;
    _benchmark.lock()->start();
}

BenchmarkHandler::~BenchmarkHandler()
{
    if(_benchmark.expired()) return;
    _benchmark.lock()->end();
}

Benchmark::Benchmark(std::string name) :
    _name(name), _time(0), _num_calls(0), _parent(nullptr), _running(false), _children_called(0)
{
}

std::string Benchmark::getName() const
{
    return _name;
}

void Benchmark::addBenchmark(std::weak_ptr<Benchmark> benchmark)
{
    if(benchmark.expired())
        return;

    benchmark.lock()->_parent = this;
    _benchmarks.push_back(benchmark);
}

double Benchmark::getTime() const
{
    double t = _time.count() / 1'000.0;
    return t / _num_calls;
}

void Benchmark::start()
{
    if(_parent && !_parent->_running) {
        _parent->start();
    }
    _running = true;
    _start = std::chrono::steady_clock::now();
}

void Benchmark::setCallback(std::function<void(Benchmark *)> cb)
{
    _callback = cb;
}

void Benchmark::reset()
{
    _time = std::chrono::microseconds(0);
    _num_calls = 0;
    _running = false;
    _children_called = 0;

    for(auto bench : _benchmarks) {
        if(!bench.expired())
            bench.lock()->reset();
    }

    auto e = std::end(_benchmarks);
    _benchmarks.erase(std::remove_if(begin(_benchmarks),
                                     e,
                                     [] (auto bench) {
                                         return bench.expired();
                                     }), e);
}

int Benchmark::getNumCalls() const
{
    return _num_calls;
}

void Benchmark::end()
{
    if(!_running)
        return;

    ++_num_calls;
    _end = std::chrono::steady_clock::now();
    _children_called = 0;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(_end - _start);
    _time += duration;
    _running = false;

    if(_parent) {
        _parent->_children_called += 1;
        if(_parent->_children_called == _parent->_benchmarks.size())
            _parent->end();
    }
    if(_callback) _callback(this);
}

std::ostream& MindTree::operator<<(std::ostream &stream, const MindTree::Benchmark &benchmark)
{
    if(benchmark._num_calls == 0)
        return stream;

    auto calls = std::to_string(benchmark._num_calls);
    auto name = benchmark._name + "(" + calls + "):";
    stream << std::setw(40) << std::left << name << std::right << benchmark.getTime() << "ms" << std::endl;

    if(benchmark._benchmarks.size() > 1) {
        for(auto bench : benchmark._benchmarks) {
            if(!bench.expired())
                stream << *(bench.lock());
        }
    }

    return stream;
}
