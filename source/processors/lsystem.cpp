#include "../plugins/datatypes/Object/skeleton.h"
#include "data/reloadable_plugin.h"
#include "data/cache_main.h"

using namespace MindTree;

void run(DataCache *cache)
{
    std::string axiom = cache->getData(0).getData<std::string>();
    auto rawrules = cache->getData(1).getData<std::vector<std::string>>();
    auto joints = cache->getData(2).getData<std::vector<JointPtr>>();
    auto iterations = cache->getData(3).getData<int>();

    std::unordered_map<char, std::string> rules;
    for(auto rule : rawrules) {
        rules[rule[0]] = rule.substr(2, std::string::npos);
    }

    std::string expanded = axiom;
    for (int i = 0; i < iterations; ++i) {
        std::string newexpanded;
        for (char c : expanded) {
            if (rules.find(c) != rules.end()) {
                newexpanded += "(" + rules[c] + ")";
            }
            else {
                newexpanded += c;
            }
        }
        expanded = newexpanded;
    }

    std::unordered_map<char, JointPtr> joint_map;
    for(const auto j : joints)
        joint_map[j->getName()[0]] = j; 

    Joint *parent{nullptr};
    JointPtr j, root;
    uint generation{0};
    for (char c : expanded) {
        if(c == '(') {
            generation++;
        }
        else if(c == ')') {
            generation--;
        }
        else if(c == '[') {
            parent = j.get();
        }
        else if(c == ']') {
            if(parent->getParent()) parent = static_cast<Joint*>(parent->getParent());
        }
        else if (joint_map.find(c) == joint_map.end()) {
            continue;
        }
        else {
            j = std::dynamic_pointer_cast<Joint>(joint_map[c]->clone());
            j->setProperty("lsystem_generation", generation);
            if(!root) {
                root = j;
                parent = root.get();
                continue;
            }

            parent->addChild(j);
        }
    }

    cache->pushData(root);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "TRANSFORMABLE";
    info.node_type = "LSYSTEMNODE";
    info.cache_proc = run;
    return info;
}

void unload()
{
    dbout("hot reloadable plugin unloaded");
}
}
