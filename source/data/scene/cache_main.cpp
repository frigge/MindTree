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

#include "cache_main.h"

QHash<AbstractDataCache*, DoutSocket*>CacheControl::caches;

bool CacheControl::isCached(DoutSocket *socket)    
{
    return caches.values().contains(socket);
}

void CacheControl::addCache(DoutSocket *socket, AbstractDataCache *cache)    
{
    caches.insert(cache, socket); 
}

void CacheControl::removeCache(AbstractDataCache *cache)    
{
    caches.remove(cache);
}

AbstractDataCache::AbstractDataCache(DoutSocket *socket)
    : start(socket)
{
    CacheControl::addCache(socket, this);
}

void AbstractDataCache::cacheSocket(DoutSocket *socket)    
{
    cachedSockets.add(socket);
}

AbstractDataCache* AbstractDataCache::getDerived()    
{
    return this; 
}

DoutSocket* AbstractDataCache::getStart()    
{
    return start;
}

void AbstractDataCache::cacheInputs()    
{
    switch(start->getNode()->getNodeType())
    {
    case CONTAINER:
        break;
    case FUNCTION:
        break;
    case CONDITIONCONTAINER:
        break;
    case FOR:
        break;
    case WHILE:
        break;
    case COLORNODE:
        break;
    case STRINGNODE:
        break;
    case FLOATNODE:
        break;
    case INTNODE:
        intValue();
        break;
    case VECTORNODE:
        vectorValue();
        break;
    case INSOCKETS:
        break;
    case GETARRAY:
        break;
    case SETARRAY:
        break;
    case COMPOSEARRAY:
        composeArray();
        break;
    case OBJECTNODE:
        composeObject();
        break;
    case POLYGONNODE:
        composePolygon();
        break;
    case ADD:
    case SUBTRACT:
    case MULTIPLY:
    case DIVIDE:
    case DOTPRODUCT:
    case GREATERTHAN:
    case SMALLERTHAN:
    case EQUAL:
    case AND:
    case OR:
    case NOT:
    case VARNAME:
        break;
    default:
        break;
            
    }
}

void AbstractDataCache::composeArray()    
{
}

void AbstractDataCache::composePolygon()    
{
    
}

void AbstractDataCache::composeObject()    
{
}

void AbstractDataCache::vectorValue()    
{
}

void AbstractDataCache::intValue()    
{
}
