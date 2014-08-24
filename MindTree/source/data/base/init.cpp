#include "source/data/python/init.h"
#include "source/data/gl/init.h"
#include "data/python/pyutils.h"
#include "data/project.h"
#include "data/properties.h"
#include "init.h"

namespace {
    bool nogui = false;
}

void MindTree::initApp(int argc, char *argv[])
{
    MindTree::Project::create();
    MindTree::Python::init(argc, argv);
    MindTree::Python::loadIntern();
    MindTree::Python::loadPlugins();

    MindTree::parseArguments(argc, argv);
}

void MindTree::initGui()
{
    //MindTree::GL::init();
    MindTree::Python::loadSettings();
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
            if (*it == "--test"){
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout<<"running unit tests" << std::endl;
                testmode = true;
                break;
            }
        }
        if(testmode) runTests(std::vector<std::string>(arguments.begin() + 2, arguments.end()));
        nogui = true;
    }
}

void MindTree::runTests(std::vector<std::string> testlist)    
{
    MindTree::Python::GILLocker locker;
    int failed = 0;
    std::cout << std::endl;
    for(auto test : testlist) {
        try{
            std::cout << std::endl;
            std::cout<<"_____________________________________"<<std::endl;
            std::cout<<"running test: "<<test<<std::endl;
            auto testmodule = BPy::import("tests");
            if(testmodule.attr(test.c_str())())
                std::cout<<test<< " passed" << std::endl;
            else {
                std::cout<<test<< " failed" << std::endl;
                failed++;
            }
            std::cout<<"_____________________________________"<<std::endl;
            std::cout << std::endl;
        } catch(BPy::error_already_set&){
            PyErr_Print();
        }
    }
    std::cout<<"finished"<<std::endl;
    if(testlist.size() > 1)
        std::cout<< failed << " out of " << testlist.size() << " tests failed" << std::endl;
}

bool MindTree::noGui()
{
    return nogui;
}
