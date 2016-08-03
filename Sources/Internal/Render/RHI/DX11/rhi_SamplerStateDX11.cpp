#include "../Common/rhi_Private.h"
#include "../Common/rhi_Pool.h"
#include "Debug/DVAssert.h"
#include "rhi_DX11.h"
#include "_dx11.h"

namespace rhi
{
//==============================================================================

struct
SamplerStateDX11_t
{
    uint32 fragmentSamplerCount;
    uint32 vertexSamplerCount;
    ID3D11SamplerState* fragmentSampler[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    ID3D11SamplerState* vertexSampler[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];

    SamplerStateDX11_t()
        : fragmentSamplerCount(0)
        , vertexSamplerCount(0)
    {
    }
};

typedef ResourcePool<SamplerStateDX11_t, RESOURCE_SAMPLER_STATE, SamplerState::Descriptor, false> SamplerStateDX11Pool;
RHI_IMPL_POOL(SamplerStateDX11_t, RESOURCE_SAMPLER_STATE, SamplerState::Descriptor, false);

//------------------------------------------------------------------------------

static D3D11_FILTER
_TextureFilterDX11(TextureFilter min_filter, TextureFilter mag_filter, TextureMipFilter mip_filter, DAVA::uint32 anisotropy)
{
    D3D11_FILTER f = D3D11_FILTER_MIN_MAG_MIP_POINT;

    switch (mip_filter)
    {
    case TEXMIPFILTER_NONE:
    case TEXMIPFILTER_NEAREST:
    {
        if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_NEAREST)
            f = D3D11_FILTER_MIN_MAG_MIP_POINT;
        else if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_LINEAR)
            f = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_NEAREST)
            f = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_LINEAR)
            f = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    }
    break;

    case TEXMIPFILTER_LINEAR:
    {
        if (anisotropy > 1)
            f = D3D11_FILTER_ANISOTROPIC;
        else if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_NEAREST)
            f = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        else if (min_filter == TEXFILTER_NEAREST && mag_filter == TEXFILTER_LINEAR)
            f = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_NEAREST)
            f = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        else if (min_filter == TEXFILTER_LINEAR && mag_filter == TEXFILTER_LINEAR)
            f = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
    break;
    }

    return f;
}

//------------------------------------------------------------------------------

D3D11_TEXTURE_ADDRESS_MODE
_TextureAddrModeDX11(TextureAddrMode mode)
{
    D3D11_TEXTURE_ADDRESS_MODE m = D3D11_TEXTURE_ADDRESS_WRAP;

    switch (mode)
    {
    case TEXADDR_WRAP:
        m = D3D11_TEXTURE_ADDRESS_WRAP;
        break;
    case TEXADDR_CLAMP:
        m = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;
    case TEXADDR_MIRROR:
        m = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;
    }

    return m;
}

//------------------------------------------------------------------------------

static void
dx11_SamplerState_Delete(Handle hstate)
{
    SamplerStateDX11_t* state = SamplerStateDX11Pool::Get(hstate);

    if (state)
    {
        for (unsigned s = 0; s != state->fragmentSamplerCount; ++s)
        {
            if (state->fragmentSampler[s])
            {
                state->fragmentSampler[s]->Release();
            }
        }

        for (unsigned s = 0; s != state->vertexSamplerCount; ++s)
        {
            if (state->vertexSampler[s])
            {
                state->vertexSampler[s]->Release();
            }
        }

        SamplerStateDX11Pool::Free(hstate);
    }
}

//------------------------------------------------------------------------------

static Handle
dx11_SamplerState_Create(const SamplerState::Descriptor& desc)
{
    Handle handle = SamplerStateDX11Pool::Alloc();
    SamplerStateDX11_t* state = SamplerStateDX11Pool::Get(handle);
    bool success = true;

    memset(state->fragmentSampler, 0, sizeof(state->fragmentSampler));
    memset(state->vertexSampler, 0, sizeof(state->vertexSampler));

    state->fragmentSamplerCount = desc.fragmentSamplerCount;
    for (unsigned s = 0; s != desc.fragmentSamplerCount; ++s)
    {
        D3D11_SAMPLER_DESC s_desc = {};

        s_desc.MaxAnisotropy = desc.fragmentSampler[s].anisotropyLevel;
        DVASSERT(s_desc.MaxAnisotropy >= 1);
        DVASSERT(s_desc.MaxAnisotropy <= rhi::DeviceCaps().maxAnisotropy);

        s_desc.Filter = _TextureFilterDX11(TextureFilter(desc.fragmentSampler[s].minFilter),
                                           TextureFilter(desc.fragmentSampler[s].magFilter),
                                           TextureMipFilter(desc.fragmentSampler[s].mipFilter),
                                           s_desc.MaxAnisotropy);
        s_desc.AddressU = _TextureAddrModeDX11(TextureAddrMode(desc.fragmentSampler[s].addrU));
        s_desc.AddressV = _TextureAddrModeDX11(TextureAddrMode(desc.fragmentSampler[s].addrV));
        s_desc.AddressW = _TextureAddrModeDX11(TextureAddrMode(desc.fragmentSampler[s].addrW));
        s_desc.MinLOD = -D3D11_FLOAT32_MAX;
        s_desc.MaxLOD = D3D11_FLOAT32_MAX;

        DVASSERT(s_desc.MaxAnisotropy >= 1);

        HRESULT hr = _D3D11_Device->CreateSamplerState(&s_desc, state->fragmentSampler + s);
        CHECK_HR(hr)

        if (FAILED(hr))
        {
            state->fragmentSampler[s] = nullptr;
            success = false;
        }
    }

    state->vertexSamplerCount = desc.vertexSamplerCount;
    for (unsigned s = 0; s != desc.vertexSamplerCount; ++s)
    {
        D3D11_SAMPLER_DESC s_desc = {};

        s_desc.MaxAnisotropy = desc.vertexSampler[s].anisotropyLevel;
        DVASSERT(s_desc.MaxAnisotropy >= 1);
        DVASSERT(s_desc.MaxAnisotropy <= rhi::DeviceCaps().maxAnisotropy);

        s_desc.Filter = _TextureFilterDX11(TextureFilter(desc.vertexSampler[s].minFilter),
                                           TextureFilter(desc.vertexSampler[s].magFilter),
                                           TextureMipFilter(desc.vertexSampler[s].mipFilter),
                                           s_desc.MaxAnisotropy);
        s_desc.AddressU = _TextureAddrModeDX11(TextureAddrMode(desc.vertexSampler[s].addrU));
        s_desc.AddressV = _TextureAddrModeDX11(TextureAddrMode(desc.vertexSampler[s].addrV));
        s_desc.AddressW = _TextureAddrModeDX11(TextureAddrMode(desc.vertexSampler[s].addrW));
        s_desc.MinLOD = -D3D11_FLOAT32_MAX;
        s_desc.MaxLOD = D3D11_FLOAT32_MAX;

        DVASSERT(s_desc.MaxAnisotropy >= 1);

        HRESULT hr = _D3D11_Device->CreateSamplerState(&s_desc, state->vertexSampler + s);
        CHECK_HR(hr)

        if (FAILED(hr))
        {
            state->vertexSampler[s] = nullptr;
            success = false;
        }
    }

    if (!success)
    {
        dx11_SamplerState_Delete(handle);
        handle = InvalidHandle;
    }

    return handle;
}

//==============================================================================

namespace SamplerStateDX11
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_SamplerState_Create = &dx11_SamplerState_Create;
    dispatch->impl_SamplerState_Delete = &dx11_SamplerState_Delete;
}

void SetToRHI(Handle hstate, ID3D11DeviceContext* context)
{
    SamplerStateDX11_t* state = SamplerStateDX11Pool::Get(hstate);

    context->PSSetSamplers(0, state->fragmentSamplerCount, state->fragmentSampler);

    if (state->vertexSamplerCount)
    {
        context->VSSetSamplers(0, state->vertexSamplerCount, state->vertexSampler);
    }
}
}

//==============================================================================
} // namespace rhi
