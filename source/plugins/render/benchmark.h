#ifndef MT_GL_BENCHMARK_H
#define MT_GL_BENCHMARK_H

#include "chrono"
#include "string"
#include "memory"
#include "vector"
#include "ostream"
#include "functional"

namespace MindTree { class Benchmark; }

namespace MindTree
{
std::ostream& operator<<(std::ostream &stream, const MindTree::Benchmark &benchmark);

class Benchmark
{
public:
    Benchmark(std::string name);
    double getTime() const;
    void addBenchmark(std::weak_ptr<Benchmark> benchmark);
    std::string getName() const;
    void reset();
    void setCallback(std::function<void(Benchmark *)> cb);
    int getNumCalls() const;

private:
    friend class BenchmarkHandler;
    friend std::ostream& operator<<(std::ostream &stream, const Benchmark &benchmark);

    std::function<void(Benchmark*)> _callback;

    void start();
    void end();

    Benchmark *_parent;

    std::string _name;
    std::chrono::nanoseconds _time;
    int _num_calls;
    int _children_called;

    std::vector<std::weak_ptr<Benchmark>> _benchmarks;
    bool _running;
    std::chrono::time_point<std::chrono::steady_clock> _start;
    std::chrono::time_point<std::chrono::steady_clock> _end;
};


class BenchmarkHandler
{
public:
    BenchmarkHandler(std::weak_ptr<Benchmark> benchmark);
    ~BenchmarkHandler();

private:
    std::weak_ptr<Benchmark> _benchmark;
};

}

#endif
