#include "signal.h"

using namespace MindTree;

std::vector<std::string> Signal::emitterIDs;
std::atomic<int> Signal::CallbackHandler::sigCounter{0};

Signal::CallbackHandler::CallbackHandler()
    : detached(false)
{
}

Signal::CallbackHandler::CallbackHandler(const CallbackHandler &other) noexcept 
:    detached(other.detached.load()),
     destructor(other.destructor),
     detacher(other.detacher),
     copyNotifier(other.copyNotifier)
{
    if (detacher != nullptr) detacher(const_cast<CallbackHandler*>(&other));
    if (copyNotifier != nullptr) copyNotifier(this);
}

Signal::CallbackHandler::~CallbackHandler() noexcept
{
    if(!detached && destructor != nullptr) destructor();
    if(!detached && detacher != nullptr) detacher(this);
}

Signal::CallbackHandler& Signal::CallbackHandler::operator=(const CallbackHandler& other) noexcept
{
    if(destructor != nullptr) destructor();

    destructor = other.destructor;
    detached = other.detached.load();
    copyNotifier = other.copyNotifier;
    detacher = other.detacher;
    if(detacher != nullptr) detacher(const_cast<CallbackHandler*>(&other));

    if(copyNotifier != nullptr) copyNotifier(this);

    return *this;
}

Signal::CallbackHandler::operator bool()
{
    return destructor != nullptr;
}

void Signal::CallbackHandler::detach() noexcept
{ 
    if(detacher != nullptr) detacher(this);
}

void Signal::CallbackHandler::destruct() noexcept
{
    if(destructor != nullptr) destructor();
}

Signal::LiveTimeTracker::LiveTimeTracker(void* boundObject)
:   boundObject(boundObject),
    registered(false),
    destructor([]{})
{
}

Signal::LiveTimeTracker::~LiveTimeTracker()
{
    destructor();
}

