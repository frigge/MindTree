#ifndef TEXTIO_HEADER
#define TEXTIO_HEADER

#include "string"
#include "thread"
#include "mutex"

namespace MindTree {
class DinSocket;
}

class TextWatcher
{
public:
    TextWatcher(MindTree::DinSocket *socket, std::string filename);
    virtual ~TextWatcher();

    void startWatching();
    std::string getFileName();

private:
    void watch();
    bool isWatching();
    void stopWatching();

    std::thread _thread;
    std::mutex _watchMutex;
    std::string _filename;
    bool _watching;
    MindTree::DinSocket *_socket;
    int _mtime;
};
#endif
