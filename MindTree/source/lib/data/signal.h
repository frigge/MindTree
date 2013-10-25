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

#define MT_SIGNAL_EMITTER(...) MindTree::Signal::callHandler(__PRETTY_FUNCTION__, ##__VA_ARGS__)

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
    template<typename... Args> friend class SignalObject;
};

template<typename... Args>
class SignalObject
{
public:
    SignalObject(std::function<void(Args...)> fn) : fn(fn), sigID(CallbackHandler::sigCounter++) { }

    SignalObject(const SignalObject &other) : fn(other.fn), sigID(other.sigID) {}
    bool operator==(const SignalObject& other) {return other.sigID == sigID;}
    bool operator!=(const SignalObject& other) {return other.sigID != sigID;}
    void operator()(Args... args) {fn(args...);}

private:
    std::function<void(Args...)> fn;
    int sigID;
};

template<>
class SignalObject<>
{
public:
    SignalObject(std::function<void()> fn) : fn(fn), sigID(CallbackHandler::sigCounter++) { }

    SignalObject(const SignalObject &other) : fn(other.fn), sigID(other.sigID) {}
    bool operator==(const SignalObject& other) {return other.sigID == sigID;}
    bool operator!=(const SignalObject& other) {return other.sigID != sigID;}
    void operator()() {fn();}

private:
    std::function<void()> fn;
    int sigID;
};

template<>
class SignalObject<BPy::object>
{
public:
    SignalObject(BPy::object fn) : fn(fn), sigID(CallbackHandler::sigCounter++) {}
    SignalObject(const SignalObject &other) : fn(other.fn), sigID(other.sigID) {}
    bool operator==(const SignalObject& other) {return other.sigID == sigID;}
    bool operator!=(const SignalObject& other) {return other.sigID != sigID;}
    template<typename... Args>
    void operator()(Args... args) {
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
class Sig
{
public:
    CallbackHandler add(std::function<void(Args ...)> fn) {
        SignalObject<Args...> sigObj(fn);
        callbacks.push_back(sigObj); 
        return CallbackHandler([this, sigObj] {
                    this->callbacks.erase(std::find(this->callbacks.begin(), this->callbacks.end(), sigObj));
                });
    }

    void operator()(Args ...args) {
        for(auto fn : callbacks) fn(args...);
    }

private:
    std::vector<SignalObject<Args...>> callbacks;
};

template<>
class Sig<>
{
public:
    CallbackHandler add(std::function<void()> fn) {
        SignalObject<> sigObj(fn);
        callbacks.push_back(sigObj); 
        return CallbackHandler([this, sigObj]
                {
                    this->callbacks.erase(std::find(this->callbacks.begin(), this->callbacks.end(), sigObj));
                });
    }

    void operator()() {
        for(auto fn : callbacks) fn();
    }

private:
    std::vector<SignalObject<>> callbacks;
};

template<>
class Sig<BPy::object>
{
public:
    CallbackHandler add(BPy::object fn) {
        SignalObject<BPy::object> sigObj(fn);
        callbacks.push_back(sigObj); 
        return CallbackHandler([this, sigObj]
                {
                    this->callbacks.erase(std::find(this->callbacks.begin(), this->callbacks.end(), sigObj));
                });
    }

    template<typename... Args>
    void operator()(Args... args) {
        for(auto fn : callbacks) fn(args...);
    }

private:
    std::vector<SignalObject<BPy::object>> callbacks;
};

template<typename ...Args>
class SignalCollector
{
public:
    CallbackHandler add(std::string sigEmitter, std::function<void(Args...)> fun) {
        if(!sigs.count(sigEmitter)) sigs[sigEmitter] = new Sig<Args...>();
        auto handler = sigs[sigEmitter]->add(fun); 
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
        return handler;
    }

    void operator()(std::string sigEmitter, Args... args)
    {
        if(sigs.count(sigEmitter))(*sigs[sigEmitter])(args...);
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::map<std::string, Sig<Args...>*> sigs;
};

template<>
class SignalCollector<>
{
public:
    CallbackHandler add(std::string sigEmitter, std::function<void()> fun) {
        if(!sigs.count(sigEmitter)) sigs[sigEmitter] = new Sig<>();
        auto handler = sigs[sigEmitter]->add(fun); 
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
    std::map<std::string, Sig<>*> sigs;
};

template<>
class SignalCollector<BPy::object>
{
public:
    CallbackHandler add(std::string sigEmitter, BPy::object fun) {
        if(!sigs.count(sigEmitter)) sigs[sigEmitter] = new Sig<BPy::object>();
        auto handler = sigs[sigEmitter]->add(fun); 
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
        return handler;
    }

    template<typename...Args>
    void operator()(std::string sigEmitter, Args... args)
    {
        if(sigs.count(sigEmitter))(*sigs[sigEmitter])(args...);
        if(std::find(emitterIDs.begin(), emitterIDs.end(), sigEmitter) == emitterIDs.end())
            emitterIDs.push_back(sigEmitter);
    }

private:
    std::map<std::string, Sig<BPy::object>*> sigs;
};

template<typename...Args>
struct SignalHandler {
    static SignalCollector<Args...> handler;
};

template<typename...Args> 
SignalCollector<Args...> SignalHandler<Args...>::handler;

template<typename ...Args>
void callHandler(std::string sig, Args... args)
{
    SignalHandler<Args...>::handler(sig, args ...);
    
    //The Python Signal only registers one tuple as an argument so the
    //signature is different.
    SignalHandler<BPy::object>::handler(sig, args ...);
}

template<typename ...Args>
SignalCollector<Args...>& getHandler()
{
    return SignalHandler<Args...>::handler;
}

template<typename ...Args>
void mergeSignals(std::string oldSignal, std::string newSignal)
{
    getHandler<Args...>().add(oldSignal, [newSignal](Args... args){
                MT_CUSTOM_SIGNAL_EMITTER(newSignal);
            }).detach();
}

} /* Signal */
    
} //MindTree
#endif
