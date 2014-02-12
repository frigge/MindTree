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

#include "data/cache_main.h"
#include "source/plugins/datatypes/Object/object.h"

namespace MindTree
{

class ObjectDataCache : public DataCache
{
public:
    ObjectDataCache(const MindTree::DoutSocket *socket);
    virtual ~ObjectDataCache();
//    void createMesh();
};

class ObjectCache : public DataCache
{
public:
    ObjectCache(const MindTree::DinSocket *socket);
    ObjectCache(const MindTree::DAInSocket *asocket);
    ObjectCache(const MindTree::DoutSocket *socket);
    ~ObjectCache();
//    void clear();
//    ObjectCache* getDerived();

protected:
//    virtual void composeArray();
//    void composeObject();
//    void container();
//    void stepup();
//    void loadObj();
//    void composeGroup();
//    void createCamera();
//    void createLight();
//    //void transform();
};
} /* MindTree */

