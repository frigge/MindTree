#include "iomanip"
#include "iostream"
#include "algorithm"
#include "benchmark.h"

using namespace MindTree;

BenchmarkHandler::BenchmarkHandler(std::weak_ptr<Benchmark> benchmark) :
    _benchmark(benchmark)
{
    if(!_benchmark.expired()) _benchmark.lock()->start();
}

BenchmarkHandler::~BenchmarkHandler()
{
    if(!_benchmark.expired()) _benchmark.lock()->end();
}

Benchmark::Benchmark(std::string name) :
    _name(name), _time(0), _num_calls(0), _parent(nullptr), _running(false), _children_called(0)
{
}

const Benchmark *Benchmark::parent() const
{
	return _parent;
}

std::vector<std::shared_ptr<Benchmark>> Benchmark::benchmarks() const
{
	return _benchmarks;
}

void Benchmark::callback()
{
	_callback(this);
}

std::string Benchmark::getName() const
{
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);
    return _name;
}

void Benchmark::addBenchmark(std::shared_ptr<Benchmark> benchmark)
{
	if (!benchmark) return;
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);

    benchmark->_parent = this;
    _benchmarks.push_back(benchmark);
}

double Benchmark::getTime() const
{
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);
    double t = _time.count() / 1000.0;
    return t / _num_calls;
}

void Benchmark::start()
{
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);
    if(_parent && !_parent->_running) {
        _parent->start();
    }
    _running = true;
    _start = std::chrono::steady_clock::now();
}

void Benchmark::setCallback(std::function<void(Benchmark *)> cb)
{
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);
    _callback = cb;
}

void Benchmark::reset()
{
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);

    _time = std::chrono::microseconds(0);
    _num_calls = 0;
    _running = false;
    _children_called = 0;

    for(auto bench : _benchmarks) {
		bench->reset();
	}
}

int Benchmark::getNumCalls() const
{
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);
    return _num_calls;
}

void Benchmark::end()
{
    std::lock_guard<std::recursive_mutex> lock(_benchmarkLock);
    if(!_running)
        return;

    ++_num_calls;
    _end = std::chrono::steady_clock::now();
    _children_called = 0;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(_end - _start);
    _time += duration;
    _running = false;

    //if(_parent) {
    //    std::lock_guard<std::recursive_mutex> lock(_parent->_benchmarkLock);
    //    _parent->_children_called += 1;
    //    if(_parent->_children_called == _parent->_benchmarks.size())
    //        _parent->end();
    //}
    if(_callback) _callback(this);
}

std::ostream& MindTree::operator<<(std::ostream &stream, const MindTree::Benchmark &benchmark)
{
    std::lock_guard<std::recursive_mutex> lock(benchmark._benchmarkLock);
    if(benchmark._num_calls == 0)
        return stream;

    auto calls = std::to_string(benchmark._num_calls);
    auto name = benchmark._name + "(" + calls + "):";
    stream << std::setw(40) << std::left << name << std::right << std::fixed << std::setprecision(2) << benchmark.getTime() << std::endl;

    if(benchmark._benchmarks.size() > 1) {
        for(auto bench : benchmark._benchmarks) {
			stream << *(bench);
		}
	}

    return stream;
}
