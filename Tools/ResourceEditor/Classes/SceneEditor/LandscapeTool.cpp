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

#include "LandscapeTool.h"
#include "../Qt/Main/QtUtils.h"

LandscapeTool::LandscapeTool(int32 _ID, eToolType _type, const FilePath & _imageName)
{
    image = NULL;
    
    toolID = _ID;
    type = _type;
    
    imageName = _imageName;
    image = CreateTopLevelImage(imageName);
    
    RenderManager::Instance()->LockNonMain();
    
    float32 sideSize = (float32)image->width;
    sprite = Sprite::CreateAsRenderTarget(sideSize, sideSize, FORMAT_RGBA8888);
    
    Texture *srcTex = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
                                              image->GetWidth(), image->GetHeight(), false);
    
    Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, (float32)image->GetWidth(), (float32)image->GetHeight());
    
    RenderManager::Instance()->SetRenderTarget(sprite);
    
    RenderManager::Instance()->SetColor(Color::Black());
    RenderHelper::Instance()->FillRect(Rect(0, 0, sideSize, sideSize));
    
    RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    RenderManager::Instance()->SetColor(Color::White());
    
    srcSprite->SetScaleSize(sideSize, sideSize);
    srcSprite->SetPosition(Vector2(0, 0));
    srcSprite->Draw();
    RenderManager::Instance()->RestoreRenderTarget();
    
    
    SafeRelease(srcSprite);
    SafeRelease(srcTex);
    
    RenderManager::Instance()->UnlockNonMain();
    
    maxSize = 0.f;
    size = 0.f;
    maxStrength = 0.f;
    strength = 0.f;
    height = 0.0f;
    averageStrength = 0.f;

    
    relativeDrawing = true;
    averageDrawing = false;
    absoluteDropperDrawing = false;
    
    copyHeightmap = false;
    copyTilemask = false;
}

LandscapeTool::~LandscapeTool()
{
    SafeRelease(image);
    SafeRelease(sprite);
}

float32 LandscapeTool::SizeColorMin()
{
    return 0.2f;
}

float32 LandscapeTool::SizeColorMax()
{
    return 2.0f;
}

float32 LandscapeTool::StrengthColorMin()
{
    return 0.0f;
}

float32 LandscapeTool::StrengthColorMax()
{
    return 0.50f;
}

float32 LandscapeTool::StrengthHeightMax()
{
    return 30;
}

float32 LandscapeTool::SizeHeightMax()
{
    return 60.0f;
}

float32 LandscapeTool::DefaultStrengthHeight()
{
    return 15.f;
}

float32 LandscapeTool::DefaultSizeHeight()
{
    return SizeHeightMax() / 2.0f;
}
