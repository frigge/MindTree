#ifndef SIGNAL_WQM6LFMC

#define SIGNAL_WQM6LFMC


#include "algorithm"
#include "functional"
#include "iostream"
#include "list"
#include "map"
#include "mutex"
#include "memory"
#include "string"
#include "vector"

#include "boost/python.hpp"

#include "data/python/wrapper.h"
#include "data/properties.h"
#include "data/python/pyutils.h"
#include "data/debuglog.h"

#define MT_SIGNAL_EMITTER(...) MindTree::Signal::callHandler(__PRETTY_FUNCTION__, ##__VA_ARGS__)

#define MT_BOUND_SIGNAL_EMITTER(...) MindTree::Signal::callBoundHandler(__PRETTY_FUNCTION__, ##VA_ARGS__)

#define MT_CUSTOM_BOUND_SIGNAL_EMITTER(...) MindTree::Signal::callBoundHandler(__VA_ARGS__)

#define MT_CUSTOM_SIGNAL_EMITTER(...) MindTree::Signal::callHandler(__VA_ARGS__)

namespace BPy = boost::python;

namespace MindTree
{

namespace Signal
{

extern std::vector<std::string> emitterIDs;

class CallbackHandler
{
public:
    CallbackHandler();

    CallbackHandler(const CallbackHandler &other) noexcept;
    virtual ~CallbackHandler() noexcept;
    CallbackHandler& operator=(const CallbackHandler& other) noexcept;
    operator bool();
    void detach() noexcept;
    void destruct() noexcept;

private:
    std::atomic<bool> detached;
    std::function<void()> destructor;
    std::function<void(CallbackHandler*)> detacher;
    std::function<void(CallbackHandler*)> copyNotifier;
    static std::atomic<int> sigCounter;
    template<typename... Args> friend class Callback;
    template<typename... Args> friend class Signal;
};

typedef std::vector<CallbackHandler> CallbackVector;

class LiveTimeTracker
{
public:
    LiveTimeTracker(void*);
    ~LiveTimeTracker();

    void* boundObject;

private:
    bool registered;
    template<typename ...Args> friend
    void callBoundHandler(LiveTimeTracker* tracker, std::string sig, Args... args);

    std::function<void()> destructor;
};

template<typename... Args>
class Callback
{
public:
    Callback(std::function<void(Args...)> fn) : fn(fn), sigID(CallbackHandler::sigCounter++) { }

    Callback(const Callback &other) : fn(other.fn), sigID(other.sigID) {}
    bool operator==(const Callback& other) {return other.sigID == sigID;}
    bool operator!=(const Callback& other) {return other.sigID != sigID;}
    void operator()(Args... args) noexcept {fn(args...);}

private:
    std::function<void(Args...)> fn;
    int sigID;
};

template<>
class Callback<>
{
public:
    Callback(std::function<void()> fn) : fn(fn), sigID(CallbackHandler::sigCounter++) { }

    Callback(const Callback &other) : fn(other.fn), sigID(other.sigID) {}
    bool operator==(const Callback& other) {return other.sigID == sigID;}
    bool operator!=(const Callback& other) {return other.sigID != sigID;}
    void operator()() noexcept {fn();}

private:
    std::function<void()> fn;
    int sigID;
};

template<>
class Callback<BPy::object>
{
public:
    Callback(BPy::object fn) : fn(fn), sigID(CallbackHandler::sigCounter++) {}
    Callback(const Callback &other) : fn(other.fn), sigID(other.sigID) {}
    bool operator==(const Callback& other) {return other.sigID == sigID;}
    bool operator!=(const Callback& other) {return other.sigID != sigID;}
    template<typename... Args>
    void operator()(Args... args) {
        Python::GILLocker locker;
        try {
            fn(PyConverter<Args>::pywrap(args)...);
        } catch(BPy::error_already_set const &){
            PyErr_Print();
        }
    }

private:
    BPy::object fn;
    int sigID;
};

template<typename ... Args>
class Signal
{
public:
    Signal() {}
    ~Signal()
    {
        for (auto *cbh : handlers) {
            cbh->detach();
        }
    }

    CallbackHandler connect(std::function<void(Args ...)> fn) {
        Callback<Args...> sigObj(fn);
        callbacks.push_back(sigObj); 
        auto cb = CallbackHandler();

        cb.destructor = [this, sigObj] {
            this->disconnect(sigObj);
        };

        cb.detacher = [this] (CallbackHandler* handler) {
            auto it = std::find(begin(this->handlers), end(this->handlers), handler);
            if (it != end(this->handlers)) {
                this->handlers.erase(it);
                handler->detached = true;
            }
        };

        cb.copyNotifier = [this] (CallbackHandler* handler) {
            //std::cout << "handler: " << handler << " registered" << std::endl;
            for (auto *h : this->handlers)
                if (h == handler) {
                    //std::cout << "handler already registered o.O" << std::endl;
                    return;
                }
            this->handlers.push_back(handler);
        };

        handlers.push_back(&cb);

        return cb;
    }

    void disconnect(Callback<Args...> cb)
    {
        auto it = std::find(callbacks.begin(), callbacks.end(), cb);
        if (it != end(callbacks))
            callbacks.erase(it);
    }

    void operator()(Args ...args) {
        for(auto fn : callbacks) fn(args...);
    }

private:
    friend class CallbackHandler;
    std::vector<CallbackHandler*> handlers;
    std::vector<Callback<Args...>> callbacks;
};

template<>
class Signal<>
{
public:
    Signal() {}
    ~Signal()
    {
        for (auto *cbh : handlers) {
            cbh->detach();
        }
    }

    CallbackHandler connect(std::function<void()> fn) {
        Callback<> sigObj(fn);
        callbacks.push_back(sigObj); 
        auto cb = CallbackHandler();

        cb.destructor = [this, sigObj] {
            this->disconnect(sigObj);
        };

        cb.detacher = [this] (CallbackHandler *handler) {
            auto it = std::find(begin(this->handlers), 
                                end(this->handlers), 
                                handler);
            if (it != end(this->handlers))
                this->handlers.erase(it);
            else
                std::cout << "huh? (<>)" << std::endl;
            handler->detached = true;
        };

        cb.copyNotifier = [this] (CallbackHandler* handler) {
            this->handlers.push_back(handler);
        };

        handlers.push_back(&cb);

        return cb;
    }

    void disconnect(Callback<> cb)
    {
        auto it = std::find(callbacks.begin(), callbacks.end(), cb);
        if (it != end(callbacks))
            callbacks.erase(it);
    }

    void operator()() {
        for(auto fn : callbacks) fn();
    }

private:
    friend class CallbackHandler;
    std::vector<Callback<>> callbacks;
    std::vector<CallbackHandler*> handlers;
};

template<>
class Signal<BPy::object>
{
public:
    Signal() {}
    ~Signal()
    {
        for (auto *cbh : handlers) {
            cbh->detach();
        }
    }

    CallbackHandler connect(BPy::object fn) {
        Callback<BPy::object> sigObj(fn);
        callbacks.push_back(sigObj); 
        auto cb = CallbackHandler();

        cb.destructor = [this, sigObj] {
            this->disconnect(sigObj);
        };

        cb.detacher = [this] (CallbackHandler *handler) {
            auto it = std::find(begin(this->handlers), end(this->handlers), handler);
            if (it != end(this->handlers))
                this->handlers.erase(it);
            else
                std::cout << "huh?? (<BPy::object>)" << std::endl;
            handler->detached = true;
        };

        cb.copyNotifier = [this] (CallbackHandler* handler) {
            this->handlers.push_back(handler);
        };

        handlers.push_back(&cb);
        return cb;
    }

    void disconnect(Callback<BPy::object> cb)
    {
        auto it = std::find(callbacks.begin(), callbacks.end(), cb);
        if (it != end(callbacks))
            callbacks.erase(it);
    }


    template<typename... Args>
    void operator()(Args... args) {
        for(auto fn : callbacks) fn(args...);
    }

private:
    friend class CallbackHandler;
    std::vector<Callback<BPy::object>> callbacks;
    std::vector<CallbackHandler*> handlers;
};

template<typename ...Args>
class SignalCollector
{
    typedef Signal<Args...> Signal_t;
    typedef std::shared_ptr<Signal_t> SignalPtr_t;
public:
    SignalCollector() {}

    SignalCollector(const SignalCollector &&other) {sigs = std::move(other.sigs);}
    SignalCollector& operator=(const SignalCollector &&other) {sigs = std::move(other.sigs);}

    SignalCollector(const SignalCollector&) = delete;
    SignalCollector& operator=(const SignalCollector&) = delete;

    CallbackHandler connect(std::string sigEmitter, std::function<void(Args...)> fun) {
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);

        {
            std::lock_guard<std::mutex> lock(_sigsLock);
            if(!sigs.count(sigEmitter)) 
                sigs[sigEmitter] = std::make_shared<Signal_t>();
            auto handler = sigs[sigEmitter]->connect(fun); 
            return handler;
        }
    }

    void operator()(std::string sigEmitter, Args... args)
    {
        SignalPtr_t signal;
        {
            std::lock_guard<std::mutex> lock(_sigsLock);
            if(sigs.count(sigEmitter))
                signal = sigs[sigEmitter];
        }
        if(signal) (*signal)(args...);

        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::mutex _sigsLock;
    std::map<std::string, SignalPtr_t> sigs;
};

template<>
class SignalCollector<>
{
    typedef Signal<> Signal_t;
    typedef std::shared_ptr<Signal_t> SignalPtr_t;
public:
    SignalCollector() {}

    SignalCollector(const SignalCollector&) = delete;
    SignalCollector& operator=(const SignalCollector&) = delete;

    SignalCollector(const SignalCollector &&other) {sigs = std::move(other.sigs);}
    SignalCollector& operator=(const SignalCollector &&other) {sigs = std::move(other.sigs);}

    CallbackHandler connect(std::string sigEmitter, std::function<void()> fun) {
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);

        {
            std::lock_guard<std::mutex> lock(_sigsLock);
            if(!sigs.count(sigEmitter)) 
                sigs[sigEmitter] = std::make_shared<Signal_t>();
            auto handler = sigs[sigEmitter]->connect(fun); 
            return handler;
        }
    }

    void operator()(std::string sigEmitter)
    {
        SignalPtr_t signal;
        {
            std::lock_guard<std::mutex> lock(_sigsLock);
            if(sigs.count(sigEmitter))
                signal = sigs[sigEmitter];
        }
        if(signal) (*signal)();

        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::mutex _sigsLock;
    std::unordered_map<std::string, SignalPtr_t> sigs;
};

template<>
class SignalCollector<BPy::object>
{
    typedef Signal<BPy::object> Signal_t;
    typedef std::shared_ptr<Signal_t> SignalPtr_t;
public:
    SignalCollector() {}

    SignalCollector(const SignalCollector&) = delete;
    SignalCollector& operator=(const SignalCollector&) = delete;

    SignalCollector(const SignalCollector &&other) {sigs = std::move(other.sigs);}
    SignalCollector& operator=(const SignalCollector &&other) {sigs = std::move(other.sigs);}

    CallbackHandler connect(std::string sigEmitter, BPy::object fun) 
    {
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);

        {
            std::lock_guard<std::mutex> lock(_sigsLock);
            if(sigs.find(sigEmitter) == sigs.end()) {
                sigs.insert({sigEmitter, std::make_shared<Signal_t>()});
            }

            auto handler = sigs[sigEmitter]->connect(fun); 
            return handler;
        }
    }

    template<typename...Args>
    void operator()(std::string sigEmitter, Args... args)
    {
        SignalPtr_t signal;
        {
            std::lock_guard<std::mutex> lock(_sigsLock);
            if(sigs.find(sigEmitter) != sigs.end())
                signal = sigs[sigEmitter];
        }
        if(signal) (*signal)(args...);

        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::mutex _sigsLock;
    std::unordered_map<std::string, SignalPtr_t> sigs;
};

namespace detail {
template<typename...Args>
struct SignalHandler {
    static SignalCollector<Args...> handler;
};

template<typename...Args> 
SignalCollector<Args...> SignalHandler<Args...>::handler;

template<typename ...Args>
struct BoundSignalHandler {
    static std::map<void*, SignalCollector<Args...>> handlers;
};

template<typename...Args> 
std::map<void*, SignalCollector<Args...>> BoundSignalHandler<Args...>::handlers;
} //ns detail

template<typename ...Args>
void callHandler(std::string sig, Args... args) noexcept
{
    detail::SignalHandler<Args...>::handler(sig, args ...);
    
    //The Python Signal only registers one tuple as an argument so the
    //signature is different.
    detail::SignalHandler<BPy::object>::handler(sig, args ...);
}

template<typename ...Args>
void callBoundHandler(LiveTimeTracker* tracker, std::string sig, Args... args)
{
#ifdef MT_DEBUG_SIGNALS
    dbout("calling signal: " << sig << " on bound object: " << tracker->boundObject);
#endif
    if(!tracker->registered) {
        tracker->destructor = [tracker] {
            detail::BoundSignalHandler<Args...>::handlers.erase(tracker->boundObject);
        };
        tracker->registered = true;
    }

    auto end = detail::BoundSignalHandler<Args...>::handlers.end();
    if (detail::BoundSignalHandler<Args...>::handlers.find(tracker->boundObject) != end)
        detail::BoundSignalHandler<Args...>::handlers[tracker->boundObject](sig, args ...);
    else {
        detail::BoundSignalHandler<Args...>::handlers[tracker->boundObject] = 
            std::move(SignalCollector<Args...>());
    }
    
    //The Python Signal only registers one tuple as an argument so the
    //signature is different.
    auto e = detail::BoundSignalHandler<BPy::object>::handlers.end();
    if (detail::BoundSignalHandler<BPy::object>::handlers.find(tracker->boundObject) != e) {
        auto &handler = detail::BoundSignalHandler<BPy::object>::handlers.at(tracker->boundObject);
        handler(sig, args...);
    }
    else {
        detail::BoundSignalHandler<BPy::object>::handlers[tracker->boundObject] =
            std::move(SignalCollector<BPy::object>());
    }
}

template<typename ...Args>
SignalCollector<Args...>& getHandler()
{
    return detail::SignalHandler<Args...>::handler;
}

template<typename ...Args>
SignalCollector<Args...>& getBoundHandler(void* boundObject)
{
#ifdef MT_DEBUG_SIGNALS
    dbout("receiving bound signal handler for: " << boundObject);
#endif
    auto begin = detail::BoundSignalHandler<Args...>::handlers.begin();
    auto end = detail::BoundSignalHandler<Args...>::handlers.end();

    if(detail::BoundSignalHandler<Args...>::handlers.find(boundObject) == end)
        detail::BoundSignalHandler<Args...>::handlers[boundObject] =
            SignalCollector<Args...>();

    SignalCollector<Args...> &handler = detail::BoundSignalHandler<Args...>::handlers.at(boundObject);
    return handler;
}

namespace detail {
template<typename ...Args>
struct MergedSignals
{
    static std::unordered_map<std::string, std::string> merged;
};
template<typename ...Args>
std::unordered_map<std::string, std::string> MergedSignals<Args...>::merged;
}

template<typename ...Args>
void mergeSignals(std::string oldSignal, std::string newSignal)
{
    //merge only once!!!
    auto &merged = detail::MergedSignals<Args...>::merged;
    if(merged.find(oldSignal) != merged.end())
        return;

    getHandler<Args...>().connect(oldSignal, [newSignal](Args... args){
                MT_CUSTOM_SIGNAL_EMITTER(newSignal);
            }).detach();

    detail::MergedSignals<Args...>::merged[oldSignal] = newSignal;
}

} /* Signal */
    
} //MindTree
#endif
