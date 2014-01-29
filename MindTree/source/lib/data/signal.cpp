#include "signal.h"

std::vector<std::string> MindTree::Signal::emitterIDs;
int MindTree::Signal::CallbackHandler::sigCounter = 0;

MindTree::Signal::CallbackHandler::CallbackHandler(std::function<void()> destructor) 
    : detached(false), destructor(destructor)
{
}

MindTree::Signal::CallbackHandler::CallbackHandler(const CallbackHandler &other) 
    : destructor(other.destructor), detached(other.detached)
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

