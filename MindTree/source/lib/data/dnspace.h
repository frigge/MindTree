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

#ifndef DNSPACE_H
#define DNSPACE_H

#include "data/nodes/data_node.h"
//#include "source/graphics/nodelink.h"
#include "source/data/undo/frg_generic_undo.h"
#include "source/data/data_info_box.h"
#include "data/python/pyexposable.h"

namespace MindTree
{

namespace IO {
    void write(std::ostream &stream, const DNSpace *space);
}

class ContainerSpace;
class DNSpace : public FRGUndoBase, public PyExposable
{
public:
    class Iterator
    {
    public:
        Iterator(const DNSpace *space, int pos);
        bool operator!=(const Iterator &other) const;
        const Iterator& operator++();
        DNode* operator*()const;
    
    private:
        int pos;
        const DNSpace *space;
    };

    DNSpace();
    DNSpace(const DNSpace &space);
    virtual ~DNSpace();
    void addNode(DNode *node);
    void removeNode(DNode *node);
    void unregisterNode(DNode *node);
    std::string getName() const;
	void setName(std::string value);
    bool operator==(DNSpace &space);
    bool operator!=(DNSpace &space);

    uint getNodeCnt();
    NodeList getNodes()const;

    bool isContainerSpace() const;
    void setContainerSpace(bool value);
    ContainerSpace* toContainer();

    void addInfoBox(DInfoBox *box);
    void removeInfoBox(DInfoBox *box);
    std::vector<DInfoBox*> getInfoBoxes();

private:
    friend void MindTree::IO::write(std::ostream &, const DNSpace*);
    std::vector<DInfoBox*> infos;
    std::string name;
    NodeList nodes;
    bool isCSpace;
};


class ContainerSpace : public DNSpace
{
public:
    ContainerSpace();
    ContainerSpace(const ContainerSpace &space);
    ~ContainerSpace();

    ContainerNode *getContainer();
    void setContainer(ContainerNode *node);
    DNSpace* getParent();

private:
    DNSpace *parentSpace;
    ContainerNode *node;
};

} /* MindTree */

MindTree::DNSpace::Iterator begin(MindTree::DNSpace *space);
MindTree::DNSpace::Iterator end(MindTree::DNSpace *space);

#endif // DNSPACE_H
