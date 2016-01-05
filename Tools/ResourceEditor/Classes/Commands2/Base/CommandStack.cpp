/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Commands2/Base/CommandStack.h"
#include "Commands2/Base/CommandAction.h"

CommandStack::CommandStack()
{
    stackCommandsNotify = new CommandStackNotify(this);
}

CommandStack::~CommandStack()
{
    Clear();
    SafeRelease(stackCommandsNotify);
}

bool CommandStack::CanUndo() const
{
    return (commandList.size() > 0 && nextCommandIndex > 0);
}

bool CommandStack::CanRedo() const
{
    return (nextCommandIndex < static_cast<DAVA::int32>(commandList.size()));
}

void CommandStack::Clear()
{
    nextCommandIndex = 0;
    cleanCommandIndex = 0;

    for (auto & command : commandList)
    {
        delete command;
    }
    commandList.clear();

    CleanCheck();
}

void CommandStack::Clear(DAVA::int32 commandId)
{
    for (DAVA::uint32 i = 0, count = static_cast<DAVA::uint32>(commandList.size()); i < count; i++)
    {
        Command2* cmd = GetCommandInternal(i);
        if (cmd->GetId() == commandId)
        {
            ClearCommand(i);

            i--; // check command with same index on next step
        }
        else if (cmd->GetId() == CMDID_BATCH)
        {
            CommandBatch* batch = static_cast<CommandBatch *> (cmd);

            batch->RemoveCommands(commandId);
            if (batch->Size() == 0)
            {
                // clear empty batch
                ClearCommand(i);

                i--; // check command with same index on next step
            }
        }
    }
}

void CommandStack::Undo()
{
    if (CanUndo())
    {
        nextCommandIndex--;
        Command2* commandToUndo = GetCommandInternal(nextCommandIndex);

        if (nullptr != commandToUndo)
        {
            commandToUndo->Undo();
            EmitNotify(commandToUndo, false);
        }
    }

    CleanCheck();
}

void CommandStack::Redo()
{
    if (CanRedo())
    {
        Command2* commandToRedo = GetCommandInternal(nextCommandIndex);
        nextCommandIndex++;

        if (nullptr != commandToRedo)
        {
            commandToRedo->Redo();
            EmitNotify(commandToRedo, true);
        }
    }

    CleanCheck();
}

void CommandStack::Exec(Command2* command)
{
    DVASSERT(command != nullptr);

    CommandAction* action = dynamic_cast<CommandAction*>(command);
    if (action != nullptr)
    {
        action->Redo();
        EmitNotify(command, true);
        delete action;
    }
    else
    {   //command
        if (nullptr != curBatchCommand)
        {
            curBatchCommand->AddAndExec(command);
        }
        else
        {
            ExecInternal(command, true);
        }
    }
}

void CommandStack::BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount/* = 1*/)
{
    if (nestedBatchesCounter++ == 0)
    {
        curBatchCommand = new CommandBatch(text, commandsCount);
        curBatchCommand->SetNotify(stackCommandsNotify);
    }
    else
    {
        DVASSERT(curBatchCommand != nullptr);
        DAVA::Logger::Error("Begin batch(%s) is called inside other batch(&s)", text.c_str(), curBatchCommand->GetText().c_str());
    }
}

void CommandStack::EndBatch()
{
    if (nullptr != curBatchCommand)
    {
        --nestedBatchesCounter;
        DVASSERT(nestedBatchesCounter >= 0);

        if (nestedBatchesCounter > 0)
            return;

        if (curBatchCommand->Size() > 0)
        {
            // all command were already executed in batch
            // so just add them to stack without calling redo
            ExecInternal(curBatchCommand, false);
        }
        else
        {
            delete curBatchCommand;
        }

        curBatchCommand = nullptr;
    }
}

bool CommandStack::IsBatchStarted() const
{
    return (curBatchCommand != nullptr);
}

bool CommandStack::IsClean() const
{
    return (cleanCommandIndex == nextCommandIndex);
}

void CommandStack::SetClean(bool clean)
{
    if (clean)
    {
        cleanCommandIndex = nextCommandIndex;
    }
    else
    {
        cleanCommandIndex = -1;
    }

    CleanCheck();
}

DAVA::int32 CommandStack::GetCleanIndex() const
{
    return cleanCommandIndex;
}

DAVA::int32 CommandStack::GetNextIndex() const
{
    return nextCommandIndex;
}

DAVA::int32 CommandStack::GetUndoLimit() const
{
    return commandListLimit;
}

void CommandStack::SetUndoLimit(DAVA::int32 limit)
{
    commandListLimit = limit;
}

DAVA::uint32 CommandStack::GetCount() const
{
    return static_cast<DAVA::uint32>(commandList.size());
}

const Command2* CommandStack::GetCommand(DAVA::int32 index) const
{
    return GetCommandInternal(index);
}

Command2* CommandStack::GetCommandInternal(DAVA::int32 index) const
{
    if (index < static_cast<DAVA::int32>(commandList.size()))
    {
        std::list<Command2*>::const_iterator i = commandList.begin();
        std::advance(i, index);

        if (i != commandList.end())
        {
            return (*i);
        }
    }
    else
    {
        DAVA::Logger::Error("In %s requested wrong index %d from %d", __FUNCTION__, index, commandList.size());
    }

    return nullptr;
}

void CommandStack::ExecInternal(Command2* command, bool runCommand)
{
    ClearRedoCommands();

    commandList.push_back(command);
    nextCommandIndex++;

    if (runCommand)
    {
        command->SetNotify(stackCommandsNotify);
        command->Redo();
    }

    EmitNotify(command, true);
    ClearLimitedCommands();

    CleanCheck();
}

void CommandStack::ClearRedoCommands()
{
    if (CanRedo())
    {
        std::list<Command2*>::iterator i = commandList.begin();
        std::advance(i, nextCommandIndex);
        while (i != commandList.end())
        {
            delete *i;
            i = commandList.erase(i);
        }
    }
}

void CommandStack::ClearLimitedCommands()
{
    while ((commandListLimit > 0) && (static_cast<DAVA::int32>(commandList.size()) > commandListLimit))
    {
        ClearCommand(0);
    }
}

void CommandStack::ClearCommand(DAVA::int32 index)
{
    const Command2* command = GetCommand(index);
    if (nullptr != command)
    {
        commandList.remove((Command2*)command);

        if (nextCommandIndex > 0)
        {
            nextCommandIndex--;
        }

        if (cleanCommandIndex > 0 && index < cleanCommandIndex)
        {
            cleanCommandIndex--;
        }

        delete command;
    }

    CleanCheck();
}

void CommandStack::CleanCheck()
{
    if (lastCheckCleanState != IsClean())
    {
        lastCheckCleanState = IsClean();
        EmitCleanChanged(lastCheckCleanState);
    }
}

void CommandStack::CommandExecuted(const Command2* command, bool redo)
{
    EmitNotify(command, redo);
}

CommandStackNotify::CommandStackNotify(CommandStack* _stack)
    : stack(_stack)
{
}

void CommandStackNotify::Notify(const Command2* command, bool redo)
{
    if (nullptr != stack)
    {
        stack->CommandExecuted(command, redo);
    }
}
