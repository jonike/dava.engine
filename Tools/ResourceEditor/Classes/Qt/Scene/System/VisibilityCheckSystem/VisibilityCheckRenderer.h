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

#ifndef __VISIBILITYCHECKRENDERER_H__
#define __VISIBILITYCHECKRENDERER_H__

#include "Base/Noncopyable.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"

struct VisibilityCheckRendererDelegate
{
    virtual ~VisibilityCheckRendererDelegate() = default;
    virtual bool ShouldDrawRenderObject(DAVA::RenderObject*) = 0;
};

class VisibilityCheckRenderer : public DAVA::Noncopyable
{
public:
    struct VisbilityPoint
    {
        DAVA::Vector3 point;
        DAVA::Vector3 normal;
        DAVA::Color color;
        DAVA::float32 upAngleCosine;
        DAVA::float32 downAngleCosine;
        DAVA::float32 maxDistance;
        VisbilityPoint(const DAVA::Vector3& p, const DAVA::Vector3& n, const DAVA::Color& clr,
                       DAVA::float32 upAngle, DAVA::float32 downAngle, DAVA::float32 md)
            : point(p)
            , normal(n)
            , color(clr)
            , upAngleCosine(upAngle)
            , downAngleCosine(downAngle)
            , maxDistance(md)
        {
        }
    };

    static const DAVA::float32 cameraNearClipPlane;
    static const DAVA::PixelFormat TEXTURE_FORMAT = DAVA::PixelFormat::FORMAT_RG32F;

public:
    VisibilityCheckRenderer();
    ~VisibilityCheckRenderer();

    void SetDelegate(VisibilityCheckRendererDelegate*);

    void PreRenderScene(DAVA::RenderSystem* renderSystem, DAVA::Camera* camera);

    void RenderToCubemapFromPoint(DAVA::RenderSystem* renderSystem, const DAVA::Vector3& point, DAVA::Texture* cubemapTarget);

    void RenderVisibilityToTexture(DAVA::RenderSystem* renderSystem, DAVA::Camera* batchesCamera,
                                   DAVA::Camera* drawCamera, DAVA::Texture* cubemap, const VisbilityPoint& vp);

    void RenderCurrentOverlayTexture(DAVA::RenderSystem* renderSystem, DAVA::Camera* camera);
    void RenderProgress(float, const DAVA::Color&);

    void InvalidateMaterials();

    void FixFrame();
    void ReleaseFrame();

    void CreateOrUpdateRenderTarget(const DAVA::Size2i&);

    bool FrameFixed() const;

private:
    void SetupCameraToRenderFromPointToFaceIndex(const DAVA::Vector3& point, DAVA::uint32 faceIndex);
    void RenderWithCurrentSettings(DAVA::RenderSystem* renderSystem);
    bool ShouldRenderObject(DAVA::RenderObject*);
    bool ShouldRenderBatch(DAVA::RenderBatch*);

    void CollectRenderBatches(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera, DAVA::Vector<DAVA::RenderBatch*>& batches);

    void UpdateVisibilityMaterialProperties(DAVA::Texture* cubemapTexture, const VisbilityPoint& vp);

    void RenderToDistanceMapFromCamera(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera);
    void RenderWithReprojection(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera);

    void FixFrame(DAVA::RenderSystem* renderSystem, DAVA::Camera* fromCamera);

private:
    VisibilityCheckRendererDelegate* renderDelegate = nullptr;
    DAVA::ScopedPtr<DAVA::Camera> cubemapCamera;
    DAVA::ScopedPtr<DAVA::NMaterial> distanceMaterial;
    DAVA::ScopedPtr<DAVA::NMaterial> visibilityMaterial;
    DAVA::ScopedPtr<DAVA::NMaterial> prerenderMaterial;
    DAVA::ScopedPtr<DAVA::NMaterial> reprojectionMaterial;
    rhi::HDepthStencilState visibilityDepthStencilState;
    rhi::HDepthStencilState reprojectionDepthStencilState;
    rhi::RenderPassConfig renderTargetConfig;
    rhi::RenderPassConfig visibilityConfig;
    rhi::RenderPassConfig prerenderConfig;
    rhi::RenderPassConfig reprojectionConfig;
    rhi::RenderPassConfig distanceMapConfig;
    DAVA::Matrix4 fixedFrameMatrix;
    DAVA::Texture* renderTarget = nullptr;
    DAVA::Texture* distanceRenderTarget = nullptr;
    DAVA::Texture* fixedFrame = nullptr;
    DAVA::Texture* reprojectionTexture = nullptr;
    DAVA::Vector3 fixedFrameCameraPosition;
    float frameCompleteness = 0.0f;
    bool frameFixed = false;
    bool shouldFixFrame = false;
};

inline bool VisibilityCheckRenderer::FrameFixed() const
{
    return frameFixed;
}

#endif