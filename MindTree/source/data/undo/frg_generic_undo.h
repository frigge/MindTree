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

#ifndef FRG_GENERIC_UNDO_H
#define FRG_GENERIC_UNDO_H

#include "stack"

class FRGUndoRedoObjectBase
{
public:
    virtual ~FRGUndoRedoObjectBase() {}
    virtual void undo() = 0;
    virtual void redo() = 0;
};

class FRGStack
{
public:
    void addItem(FRGUndoRedoObjectBase* item);
    FRGUndoRedoObjectBase* popItem();
    void clear();
    
private:
    std::stack<FRGUndoRedoObjectBase*> items;
};

class FRGUndoBase
{
public:
    void undo();
    void redo();
    void registerUndoRedoObject(FRGUndoRedoObjectBase *item);
    
private:
    FRGStack undoStack, redoStack;
};
    
#endif //FRG_GENERIC_UNDO_H
