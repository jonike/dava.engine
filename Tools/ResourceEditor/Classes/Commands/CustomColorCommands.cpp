/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "CustomColorCommands.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/QtUtils.h"
#include <QFileDialog>
#include "../SceneEditor/EditorBodyControl.h"

CommandSaveTextureCustomColors::CommandSaveTextureCustomColors()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_SAVE_TEXTURE_CUSTOM_COLORS)
{
    
}

void CommandSaveTextureCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(!screen)
		return;

	FilePath selectedPathname = screen->CustomColorsGetCurrentSaveFileName();

	if(selectedPathname.IsEmpty())
	{
		selectedPathname = FilePath(screen->CurrentScenePathname().GetDirectory());
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save texture"), QString(selectedPathname.GetAbsolutePathname().c_str()), QString("PNG image (*.png)"));

	selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
		screen->CustomColorsSaveTexture(selectedPathname);
}

CommandLoadTextureCustomColors::CommandLoadTextureCustomColors()
:	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_LOAD_TEXTURE_CUSTOM_COLORS)
{
}

void CommandLoadTextureCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(!screen)
		return;

	FilePath currentPath = screen->CustomColorsGetCurrentSaveFileName();

	if(currentPath.IsEmpty())
	{
		currentPath = FilePath(screen->CurrentScenePathname().GetDirectory());
	}

	FilePath selectedPathname = GetOpenFileName(String("Load texture"), currentPath, String("PNG image (*.png)"));
	if(!selectedPathname.IsEmpty())
	{
		screen->CustomColorsLoadTexture(selectedPathname);
	}
}


CommandDrawCustomColors::CommandDrawCustomColors(Image* originalImage, Image* newImage)
:	Command(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_DRAW_CUSTOM_COLORS)
{
	commandName = "Custom Color Draw";

	undoImage = SafeRetain(originalImage);
	redoImage = SafeRetain(newImage);
}

CommandDrawCustomColors::~CommandDrawCustomColors()
{
	SafeRelease(undoImage);
	SafeRelease(redoImage);
}

void CommandDrawCustomColors::Execute()
{
	LandscapeEditorCustomColors* editor = GetEditor();
	if (editor == NULL || redoImage == NULL)
	{
		SetState(STATE_INVALID);
		return;
	}

	editor->RestoreState(redoImage);
}

void CommandDrawCustomColors::Cancel()
{
	LandscapeEditorCustomColors* editor = GetEditor();
	if (editor && undoImage)
		editor->RestoreState(undoImage);
}

LandscapeEditorCustomColors* CommandDrawCustomColors::GetEditor()
{
	LandscapeEditorCustomColors* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorCustomColors*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_CUSTOM_COLORS));
	}

	return editor;
}