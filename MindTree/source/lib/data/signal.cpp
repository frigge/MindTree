#include "signal.h"

std::vector<std::string> MindTree::Signal::emitterIDs;
int MindTree::Signal::CallbackHandler::sigCounter = 0;

MindTree::Signal::CallbackHandler::CallbackHandler(std::function<void()> destructor) 
    : detached(false), destructor(destructor)
{
}

MindTree::Signal::CallbackHandler::CallbackHandler(const CallbackHandler &other) 
:    detached(other.detached),
     destructor(other.destructor)
{
    other.detached = true;
}

MindTree::Signal::CallbackHandler::~CallbackHandler()
{
    if(!detached)destructor();
}

void MindTree::Signal::CallbackHandler::detach() 
{ 
    detached = true; 
}

void MindTree::Signal::CallbackHandler::destruct()    
{
    destructor();
}

MindTree::Signal::LiveTimeTracker::LiveTimeTracker(void* boundObject)
:   registered(false),
    boundObject(boundObject),
    destructor([]{})
{
}

MindTree::Signal::LiveTimeTracker::~LiveTimeTracker()
{
    destructor();
}

