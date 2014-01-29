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

#ifndef CACHE_DATA_77LKPR3V

#define CACHE_DATA_77LKPR3V

#include "data/cache_main.h"
#include "../Object/object.h"

namespace MindTree
{

class PolygonCache : public DataCache
{
public:
    PolygonCache(const MindTree::DoutSocket *socket);
    //PolygonCache(const MindTree::DAInSocket *asocket);
    ~PolygonCache();
    virtual PolygonCache* getDerived();
    //PolygonList* getData();
    void clear();

protected:
    void composePolygon();
    void composeArray();
    void container();
    void stepup();
    void setArray();
    void getLoopedCache();
    void forloop();
};
regCacheType(PolygonCache, Polygon);

} /* MindTree */

#endif /* end of include guard: CACHE_DATA_77LKPR3V */
