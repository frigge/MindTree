#include "source/data/python/init.h"
#include "source/data/gl/init.h"
#include "data/cache_main.h"
#include "data/python/pyutils.h"
#include "data/project.h"
#include "data/properties.h"
#include "data/reloadable.h"

#include "init.h"

namespace {
    bool nogui = false;
    std::string loadFile = "";
}

void MindTree::initApp(int argc, char *argv[])
{
    MindTree::DataCache::init();
    MindTree::Project::create();
    MindTree::Python::init(argc, argv);
    MindTree::Python::loadIntern();
    MindTree::Python::loadPlugins();
    MindTree::HotProcessorManager::start();

    MindTree::parseArguments(argc, argv);
}

void MindTree::initGui()
{
    MindTree::Python::loadSettings();

    if(loadFile != "")
        Project::load(loadFile);
}

void MindTree::finalizeApp()
{
    MindTree::Python::finalize();
}

void MindTree::parseArguments(int argc, char* argv[])
{
    std::vector<std::string> arguments;
    for(int i=0; i<argc; i++){
        arguments.push_back(argv[i]);
    }
    if(argc > 1) {
        bool testmode=false;
        for(auto it = arguments.begin(); it != arguments.end(); it++){
            if (*it == "--test" || *it == "-t"){
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout<<"running unit tests" << std::endl;
                testmode = true;
                break;
            }

            if (*it == "--open" || *it == "-o"){
                if((it+1) == end(arguments)) {
                    std::cout << "you have to specify a filename" << std::endl;
                    nogui = true;
                    return;
                }

                std::cout << std::endl;
                std::cout << std::endl;
                loadFile = *(it + 1);
                std::cout<<"open project: " << loadFile << std::endl;
                return;
            }

        }
        if(testmode) runTests(std::vector<std::string>(arguments.begin() + 2, arguments.end()));
        nogui = true;
    }
}

void MindTree::runTests(std::vector<std::string> testlist)
{
    MindTree::Python::GILLocker locker;
    std::vector<std::string> failedTests;
    std::cout << std::endl;
    for(auto test : testlist) {
        bool success = false;
        MindTree::Project::create();
        if(test != "all") {
            std::cout << std::endl;
            std::cout<<"_____________________________________\n";
            std::cout<<"running test: "<<test<<"\n"<<std::endl;
        }
        try{
            auto testmodule = BPy::import("tests");
            if(testmodule.attr(test.c_str())()) {
                if(test != "all") std::cout<<test<< " passed" << std::endl;
                success = true;
            }
        } catch(BPy::error_already_set&){
            PyErr_Print();
        }
        if(!success &&  test != "all") {
            failedTests.push_back(test);
            std::cout<<test<< " failed" << std::endl;
            std::cout<<"_____________________________________\n";
            std::cout << std::endl;
        }

    }
    if(testlist[0] != "all") std::cout<<"finished"<<std::endl;
    if(testlist.size() > 1)
        std::cout<< failedTests.size() << " out of " << testlist.size() << " tests failed" << std::endl;

    if(!failedTests.empty()) {
        std::cout << "failed Tests:\n";
        for(auto test : failedTests)
            std::cout << "\t" << test << "\n";
    }
    std::cout << std::endl;
}

bool MindTree::noGui()
{
    return nogui;
}
