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

#ifndef FLOAT_CACHE_44P6X3RJ

#define FLOAT_CACHE_44P6X3RJ

#include "data/cache_main.h"

namespace MindTree
{
    
class FloatCache : public DataCache
{
public:
    FloatCache(const MindTree::DoutSocket *socket=0);
    ~FloatCache();
//    virtual FloatCache* getDerived();
//    void clear();
//
//protected:
//    void floatValue();
//    void intValue();
//    void getLoopedCache();
//    void math(eMathOp op);
//    void modulo();
//    void container();
//    void stepup();
};
regCacheType(FloatCache, double);

} /* MindTree */
#endif /* end of include guard: FLOAT_CACHE_44P6X3RJ */
