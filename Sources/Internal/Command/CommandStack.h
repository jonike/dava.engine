#pragma once

#include "Command/Command.h"
#include "Command/CommandBatch.h"
#include "Functional/Signal.h"

namespace DAVA
{
class CommandStack
{
public:
    virtual ~CommandStack();

    void Exec(std::unique_ptr<Command>&& command);

    void BeginBatch(const String& name, uint32 commandsCount = 1);
    void EndBatch();

    bool IsClean() const;
    void SetClean();

    void Undo();
    void Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    DAVA::String GetUndoText() const;
    DAVA::String GetRedoText() const;

    Signal<bool> cleanChanged;
    Signal<bool> canUndoChanged;
    Signal<bool> canRedoChanged;
    Signal<const DAVA::String&> undoTextChanged;
    Signal<const DAVA::String&> redoTextChanged;
    Signal<const DAVA::Command*, bool /*redo*/> commandExecuted;

protected:
    virtual CommandBatch* CreateCommmandBatch(const String& name, uint32 commandsCount) const;

    virtual void ExecInternal(std::unique_ptr<Command>&& command, bool isSingleCommand);

    void UpdateCleanState();
    void SetCurrentIndex(int32 currentIndex);

    static const int32 EMPTY_INDEX = -1;

    int32 cleanIndex = EMPTY_INDEX;
    int32 currentIndex = EMPTY_INDEX;

    Vector<std::unique_ptr<Command>> commands;

    std::unique_ptr<CommandBatch> commandBatch;
    uint32 requestedBatchCount = 0;

    //members to remember stack state and do not emit extra signals
    void EmitCleanChanged(bool isClean);
    void EmitCanUndoChanged(bool canUndo);
    void EmitCanRedoChanged(bool canRedo);

    void EmitUndoTextChanged(const DAVA::String& undoText);
    void EmitRedoTextChanged(const DAVA::String& redoText);

    bool isClean = true;
    bool canUndo = false;
    bool canRedo = false;
    DAVA::String undoText;
    DAVA::String redoText;
};
}
