/*
 * base for a generic undo system.
 *Author: Sascha Fricke 
 * 
 */

#ifndef NULL
#define NULL 0x0
#endif

#ifndef FRG_GENERIC_UNDO_H
#define FRG_GENERIC_UNDO_H

#include "stack"

class FRGUndoRedoObjectBase
{
public:
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
