#include "signal.h"

using namespace MindTree;

std::vector<std::string> Signal::emitterIDs;
int Signal::CallbackHandler::sigCounter = 0;

Signal::CallbackHandler::CallbackHandler()
    : detached(false), destructor([]() {})
{
}

Signal::CallbackHandler::CallbackHandler(std::function<void()> destructor) 
    : detached(false), destructor(destructor)
{
}

Signal::CallbackHandler::CallbackHandler(const CallbackHandler &other) 
:    detached(other.detached),
     destructor(other.destructor)
{
    other.detached = true;
}

Signal::CallbackHandler::~CallbackHandler()
{
    if(!detached && destructor)destructor();
}

Signal::CallbackHandler& Signal::CallbackHandler::operator=(const CallbackHandler& other)
{
    if(destructor) destructor();

    destructor = other.destructor;
    detached = other.detached;
    other.detached = true;

    return *this;
}

void Signal::CallbackHandler::detach() 
{ 
    detached = true; 
}

void Signal::CallbackHandler::destruct()    
{
    destructor();
}

Signal::LiveTimeTracker::LiveTimeTracker(void* boundObject)
:   registered(false),
    boundObject(boundObject),
    destructor([]{})
{
}

Signal::LiveTimeTracker::~LiveTimeTracker()
{
    destructor();
}

