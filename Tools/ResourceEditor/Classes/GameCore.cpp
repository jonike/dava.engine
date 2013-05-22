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


#include "GameCore.h"
#include "AppScreens.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "TextureCompression/PVRConverter.h"

#include "CommandLine/CommandLineManager.h"

#include "TextureBrowser/TextureConvertor.h"
#include "DockParticleEditor/ParticlesEditorController.h"

using namespace DAVA;


GameCore::GameCore()
{ }

GameCore::~GameCore()
{ }

void GameCore::OnAppStarted()
{
	Logger::Instance()->SetLogFilename("ResEditor.txt");
	RenderManager::Instance()->SetFPS(30);

    LocalizationSystem::Instance()->SetCurrentLocale(EditorSettings::Instance()->GetLanguage());
	LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");

    
#ifdef __DAVAENGINE_BEAST__
	new BeastProxyImpl();
#else 
    new BeastProxy();
#endif //__DAVAENGINE_BEAST__
	
#if defined (__DAVAENGINE_MACOS__)
	PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL"));
#elif defined (__DAVAENGINE_WIN32__)
    PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL.exe"));
#endif

	new ParticlesEditorController();

    if(!CommandLineManager::Instance()->IsCommandLineModeEnabled())
    {
        Texture::SetDefaultFileFormat((ImageFileFormat)EditorSettings::Instance()->GetTextureViewFileFormat());
    }
	
	// Yuri Coder, 2013/01/23. The call below is needed for Win32 linker to notify it we are using
	// ParticleEffectNode and thus REGISTER_CLASS(ParticleEffectNode) must not be removed during optimization.
	ParticleEffectNode* effectNode = new ParticleEffectNode();
	delete effectNode;
}

void GameCore::OnAppFinished()
{
    SceneValidator::Instance()->Release();

	BeastProxy::Instance()->Release();
}

void GameCore::OnSuspend()
{
	//prevent going to suspend
    //ApplicationCore::OnSuspend();
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
	//ApplicationCore::OnBackground();
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}


