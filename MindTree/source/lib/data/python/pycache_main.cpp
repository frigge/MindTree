/*
    FRG Shader Editor, a Node-based Renderman Shading Language Editor
    Copyright (C) 2011  Sascha Fricke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "data/properties.h"
#include "pycache_main.h"

MindTree::PyCacheProcessor::PyCacheProcessor(BPy::object obj)
    : processor(obj)
{
}

MindTree::PyCacheProcessor::~PyCacheProcessor()
{
}

void MindTree::PyCacheProcessor::operator()(MindTree::DataCache* cache)    
{
    processor(BPy::ptr(cache)); 
}

MindTree::DNodePyWrapper* MindTree::wrap_DataCache_getNode(MindTree::DataCache *cache)    
{
    return new MindTree::DNodePyWrapper(const_cast<DNode*>(cache->getNode())); 
}

BPy::object MindTree::wrap_DataCache_getData(MindTree::DataCache *self, int index)
{
    return self->getData(index).toPython();
}

BPy::object MindTree::wrap_DataCache_getOutput(MindTree::DataCache *self, DoutSocketPyWrapper *outsocket)
{
    if (!outsocket->alive())  return BPy::object();
    return self->getOutput(outsocket->getWrapped<DoutSocket>()).toPython();
}

BPy::object MindTree::wrap_DataCache_getOutput(MindTree::DataCache *self)
{
    return self->getOutput().toPython();
}

void MindTree::wrap_DataCache_setData(MindTree::DataCache *self, BPy::object data)
{
    self->pushData(MindTree::Property::createPropertyFromPython(data));
}

MindTree::DoutSocketPyWrapper* MindTree::wrap_DataCache_getStart(DataCache *cache)    
{
    return new MindTree::DoutSocketPyWrapper(const_cast<DoutSocket*>(cache->getStart()));
}

void MindTree::wrap_DataCache_addProcessor(BPy::object fn, std::string ntype, std::string stype)
{
    MindTree::DataCache::addProcessor(  MindTree::SocketType(stype),
                                        MindTree::NodeType(ntype),
                                        new MindTree::PyCacheProcessor(fn));
}

std::string MindTree::wrap_DataCache_getType(DataCache *self)
{
    return self->getType().toStr();
}

//void MindTree::wrap_DataCache_cache(MindTree::DataCache *self, MindTree::DinSocketPyWrapper* socket)
//{
//    if(!socket->alive()) return;
//    self->cache(socket->getWrapped<DinSocket>());
//}
//
BPy::dict MindTree::wrap_DataCache_getProcessors()
{
    const auto &processors = DataCache::getProcessors();
    BPy::dict dict;
    for(ulong i=0; i<processors.size(); i++){
        ulong size = processors[i].size();
        BPy::list l;
        for(ulong j=0; j<size; j++) {
            auto node_str = NodeType::byID(j).toStr();
            l.append(NodeType::byID(j).toStr());
        }
        dict[SocketType::byID(i).toStr()] = l;
    }
    return dict;
}

void MindTree::wrap_DataCache_invalidate(DNodePyWrapper *node)
{
    if(!node->alive()) return;
    MindTree::DataCache::invalidate(node->getWrapped<DNode>());
}

void MindTree::wrap_DataCache()    
{
    BPy::class_<MindTree::DataCache>("_DataCache", BPy::no_init)
        //.def("cache", &wrap_DataCache_cache)
        .def("addProcessor", &wrap_DataCache_addProcessor)
        .def("invalidate", &wrap_DataCache_invalidate)
        .staticmethod("addProcessor")
        .staticmethod("invalidate")
        .add_property("node", BPy::make_function(&wrap_DataCache_getNode,
                                BPy::return_value_policy<BPy::manage_new_object>()))
        .def("getData", &wrap_DataCache_getData)
        .def("getOutput", static_cast<BPy::object(*)(DataCache*)>(&wrap_DataCache_getOutput))
        .def("getOutput", static_cast<BPy::object(*)(DataCache*, DoutSocketPyWrapper*)>(&wrap_DataCache_getOutput))
        .def("setData", &wrap_DataCache_setData)
        .add_property("type", &wrap_DataCache_getType)
        .add_static_property("processors", &wrap_DataCache_getProcessors)
        .add_property("start", BPy::make_function(&wrap_DataCache_getStart,
                                BPy::return_value_policy<BPy::manage_new_object>()));

    BPy::class_<MindTree::PyWrapCache, BPy::bases<MindTree::DataCache>>
        ("DataCache", BPy::init<DoutSocketPyWrapper*>());
}
