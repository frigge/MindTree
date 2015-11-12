#include "boost/python.hpp"
#include "data/nodes/data_node.h"
#include "data/cache_main.h"
#include "iostream"
#include "fstream"
#include "QFileInfo"
#include "QDateTime"
#include "textio.h"

using namespace MindTree;

PROPERTY_TYPE_INFO(TextWatcherPtr, "TEXTWATCHER");

TextWatcher::TextWatcher(DinSocket *socket, std::string filename)
    : _filename(filename), 
    _watching(false), 
     _socket(socket), 
    _mtime(-1)
{
    QFileInfo finfo(filename.c_str());
    if (finfo.exists())
        _mtime = QFileInfo(filename.c_str()).lastModified().toMSecsSinceEpoch();
}

TextWatcher::~TextWatcher()
{
    stopWatching();
    _thread.detach();
    std::cout << "watcher killed" << std::endl;
}

std::string TextWatcher::getFileName()
{
    return _filename;
}

void TextWatcher::startWatching()
{
    if(_mtime == -1 || isWatching())
        return;

    _thread = std::thread([this]{this->watch();});
}

void TextWatcher::stopWatching()
{
    _watching = false;
}

bool TextWatcher::isWatching()
{
    return _watching;
}

void TextWatcher::watch()
{
    std::cout << "start watching" << std::endl;
    _watching = true;
    while(isWatching()) {
        if(_mtime == -1)
            continue;

        int mt = QFileInfo(_filename.c_str()).lastModified().toMSecsSinceEpoch();
        if(mt > _mtime) {
            std::cout << "file has changed" << std::endl;
            MT_CUSTOM_SIGNAL_EMITTER("socketChanged", _socket);
            _mtime = mt;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    std::cout << "stop watching" << std::endl;
}

BOOST_PYTHON_MODULE(textio) {
    auto readText = [] (DataCache *cache) {
        std::string filename = cache->getData(0).getData<std::string>();
        bool autowatch = cache->getData(1).getData<bool>();

        std::ifstream stream(filename); 

        std::string content;
        std::string line;
        while (stream) {
            std::getline(stream, line);
            content += line;
            content += "\n";
        }

        bool has_watcher = cache->getNode()->hasProperty("_textWatcher");
        std::string watched_filename;
        if(has_watcher)
            watched_filename = cache->getNode()->getProperty("_textWatcher")
                .getData<TextWatcherPtr>()->getFileName();

        if(autowatch) {
            if(!has_watcher || ( has_watcher && watched_filename != filename)) {
                std::cout << "yop, need to watch for the file" << std::endl;
                auto filenamesocket = cache->getNode()->getInSockets().at(0);
                auto watcher = std::make_shared<TextWatcher>(filenamesocket, filename);
                watcher->startWatching();
                DNode *node = const_cast<DNode*>(cache->getNode());
                node->setProperty("_textWatcher", watcher);
            }
        }
        else if(cache->getNode()->hasProperty("_textWatcher")){
            DNode *node = const_cast<DNode*>(cache->getNode());
            node->rmProperty("_textWatcher");
        }

        cache->pushData(content);
    };

    DataCache::addProcessor(new CacheProcessor("STRING", "TEXTREAD", readText));
}
