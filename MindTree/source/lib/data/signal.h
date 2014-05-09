#ifndef SIGNAL_WQM6LFMC

#define SIGNAL_WQM6LFMC


#include "functional"
#include "list"
#include "string"
#include "map"
#include "vector"
#include "iostream"
#include "algorithm"
#include "boost/python.hpp"
#include "data/python/wrapper.h"
#include "data/properties.h"
#include "data/python/pyutils.h"

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
    CallbackHandler(std::function<void()> destructor);
    CallbackHandler(const CallbackHandler &other);
    virtual ~CallbackHandler();
    void detach();
    void destruct();

private:
    mutable bool detached;
    std::function<void()>destructor;
    static int sigCounter;
    template<typename... Args> friend class Callback;
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
    void operator()(Args... args) {fn(args...);}

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
    void operator()() {fn();}

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
            fn(typename PyConverter<Args>::t(args)...);
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
    CallbackHandler connect(std::function<void(Args ...)> fn) {
        Callback<Args...> sigObj(fn);
        callbacks.push_back(sigObj); 
        return CallbackHandler([this, sigObj] {
                    this->disconnect(sigObj);
                });
    }

    void disconnect(Callback<Args...> cb)
    {
        callbacks.erase(std::find(callbacks.begin(), callbacks.end(), cb));
    }

    void operator()(Args ...args) {
        for(auto fn : callbacks) fn(args...);
    }

private:
    std::vector<Callback<Args...>> callbacks;
};

template<>
class Signal<>
{
public:
    CallbackHandler connect(std::function<void()> fn) {
        Callback<> sigObj(fn);
        callbacks.push_back(sigObj); 
        return CallbackHandler([this, sigObj]
                {
                    this->disconnect(sigObj);
                });
    }

    void disconnect(Callback<> cb)
    {
        callbacks.erase(std::find(callbacks.begin(), callbacks.end(), cb));
    }

    void operator()() {
        for(auto fn : callbacks) fn();
    }

private:
    std::vector<Callback<>> callbacks;
};

template<>
class Signal<BPy::object>
{
public:
    CallbackHandler connect(BPy::object fn) {
        Callback<BPy::object> sigObj(fn);
        callbacks.push_back(sigObj); 
        return CallbackHandler([this, sigObj]
                {
                    this->disconnect(sigObj);
                });
    }

    void disconnect(Callback<BPy::object> cb)
    {
        callbacks.erase(std::find(callbacks.begin(), callbacks.end(), cb));
    }


    template<typename... Args>
    void operator()(Args... args) {
        for(auto fn : callbacks) fn(args...);
    }

private:
    std::vector<Callback<BPy::object>> callbacks;
};

template<typename ...Args>
class SignalCollector
{
public:
    CallbackHandler connect(std::string sigEmitter, std::function<void(Args...)> fun) {
        if(!sigs.count(sigEmitter)) sigs[sigEmitter] = new Signal<Args...>();
        auto handler = sigs[sigEmitter]->connect(fun); 
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
        return handler;
    }

    void operator()(std::string sigEmitter, Args... args)
    {
        if(sigs.count(sigEmitter))
            (*sigs[sigEmitter])(args...);

        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::map<std::string, Signal<Args...>*> sigs;
};

template<>
class SignalCollector<>
{
public:
    CallbackHandler connect(std::string sigEmitter, std::function<void()> fun) {
        if(!sigs.count(sigEmitter)) sigs[sigEmitter] = new Signal<>();
        auto handler = sigs[sigEmitter]->connect(fun); 
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
        return handler;
    }

    void operator()(std::string sigEmitter)
    {
        if(sigs.count(sigEmitter))(*sigs[sigEmitter])();
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::map<std::string, Signal<>*> sigs;
};

template<>
class SignalCollector<BPy::object>
{
public:
    CallbackHandler connect(std::string sigEmitter, BPy::object fun) 
    {
        if(sigs.find(sigEmitter) == sigs.end()) 
            sigs.insert({sigEmitter, new Signal<BPy::object>()});

        auto handler = sigs[sigEmitter]->connect(fun); 

        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
        return handler;
    }

    template<typename...Args>
    void operator()(std::string sigEmitter, Args... args)
    {
        if(sigs.find(sigEmitter) != sigs.end())
            (*sigs[sigEmitter])(args...);

        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::map<std::string, Signal<BPy::object>*> sigs;
};

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

template<typename ...Args>
void callHandler(std::string sig, Args... args)
{
    SignalHandler<Args...>::handler(sig, args ...);
    
    //The Python Signal only registers one tuple as an argument so the
    //signature is different.
    SignalHandler<BPy::object>::handler(sig, args ...);
}

template<typename ...Args>
void callBoundHandler(LiveTimeTracker* tracker, std::string sig, Args... args)
{
    if(!tracker->registered) {
        tracker->destructor = [tracker] {
            BoundSignalHandler<Args...>::handlers.erase(tracker->boundObject);
        };
        tracker->registered = true;
    }

    auto end = BoundSignalHandler<Args...>::handlers.end();
    if (BoundSignalHandler<Args...>::handlers.find(tracker->boundObject) != end)
        BoundSignalHandler<Args...>::handlers[tracker->boundObject](sig, args ...);
    else
        BoundSignalHandler<Args...>::handlers.insert({tracker->boundObject, SignalCollector<Args...>()});
    
    //The Python Signal only registers one tuple as an argument so the
    //signature is different.
    auto e = BoundSignalHandler<BPy::object>::handlers.end();
    if (BoundSignalHandler<BPy::object>::handlers.find(tracker->boundObject) != e) {
        auto &handler = BoundSignalHandler<BPy::object>::handlers.at(tracker->boundObject);
        handler(sig, args...);
    }
    else
        BoundSignalHandler<BPy::object>::handlers.insert({tracker->boundObject, SignalCollector<BPy::object>()});
}

template<typename ...Args>
SignalCollector<Args...>& getHandler()
{
    return SignalHandler<Args...>::handler;
}

template<typename ...Args>
SignalCollector<Args...>& getBoundHandler(void* boundObject)
{
    auto begin = BoundSignalHandler<Args...>::handlers.begin();
    auto end = BoundSignalHandler<Args...>::handlers.end();

    if(BoundSignalHandler<Args...>::handlers.find(boundObject) == end)
        BoundSignalHandler<Args...>::handlers.insert({boundObject, SignalCollector<Args...>()});

    SignalCollector<Args...> &handler = BoundSignalHandler<Args...>::handlers.at(boundObject);
    return handler;
}

template<typename ...Args>
void mergeSignals(std::string oldSignal, std::string newSignal)
{
    getHandler<Args...>().connect(oldSignal, [newSignal](Args... args){
                MT_CUSTOM_SIGNAL_EMITTER(newSignal);
            }).detach();
}

} /* Signal */
    
} //MindTree
#endif
