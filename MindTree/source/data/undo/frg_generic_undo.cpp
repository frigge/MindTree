/*
    Base for a generic Undo System
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

#include "frg_generic_undo.h"

void FRGStack::addItem(FRGUndoRedoObjectBase* item)    
{
    items.push(item);
}

void FRGStack::clear()    
{
    while(!items.empty())
    {
        delete items.top();
        items.pop();
    }
}

FRGUndoRedoObjectBase* FRGStack::popItem()    
{
    if(items.empty())return nullptr;
    FRGUndoRedoObjectBase *item = items.top();
    items.pop();
    return item;
}

void FRGUndoBase::registerUndoRedoObject(FRGUndoRedoObjectBase *item)    
{
    undoStack.addItem(item);
    redoStack.clear();
}

void FRGUndoBase::undo()    
{
    FRGUndoRedoObjectBase *item = undoStack.popItem();
    if(!item)return;
    item->undo();
    redoStack.addItem(item);
}

void FRGUndoBase::redo()    
{
    FRGUndoRedoObjectBase *item = redoStack.popItem();
    if(!item)return;
    item->redo();
    undoStack.addItem(item);
}
