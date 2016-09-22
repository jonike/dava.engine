#ifndef __RHI_PRIVATE_H__
#define __RHI_PRIVATE_H__

    #include "../rhi_Type.h"

namespace rhi
{
////////////////////////////////////////////////////////////////////////////////
// render-target

////////////////////////////////////////////////////////////////////////////////
// vertex-buffer

namespace VertexBuffer
{
Handle Create(const VertexBuffer::Descriptor& desc);
void Delete(Handle vb);

bool Update(Handle vb, const void* data, uint32 offset = 0, uint32 size = 0);

void* Map(Handle vb, uint32 offset, uint32 size);
void Unmap(Handle vb);

bool NeedRestore(Handle vb);

} // namespace VertexBuffer

////////////////////////////////////////////////////////////////////////////////
// index-buffer

namespace IndexBuffer
{
Handle Create(const IndexBuffer::Descriptor& desc);
void Delete(Handle ib);

bool Update(Handle ib, const void* data, uint32 offset = 0, uint32 size = 0);

void* Map(Handle ib, uint32 offset, uint32 size);
void Unmap(Handle ib);

bool NeedRestore(Handle ib);

} // namespace IndexBuffer

////////////////////////////////////////////////////////////////////////////////
// query-buffer

namespace QueryBuffer
{
Handle Create(uint32 maxObjectCount);
void Reset(Handle buf);
void Delete(Handle buf);

bool BufferIsReady(Handle buf);
bool IsReady(Handle buf, uint32 objectIndex);
int Value(Handle buf, uint32 objectIndex);
}

////////////////////////////////////////////////////////////////////////////////
// perfquery-set

namespace PerfQuery
{
Handle Create();
void Delete(Handle query);
void Reset(Handle query);

bool IsReady(Handle query);
uint64 Value(Handle query);

void SetCurrent(Handle startQuery, Handle endQuery);
}

////////////////////////////////////////////////////////////////////////////////
// texture

namespace Texture
{
Handle Create(const Descriptor& desc);
void Delete(Handle tex);

void* Map(Handle tex, unsigned level = 0, TextureFace face = TEXTURE_FACE_NEGATIVE_X);
void Unmap(Handle tex);

void Update(Handle tex, const void* data, uint32 level, TextureFace face = TEXTURE_FACE_NEGATIVE_X);

bool NeedRestore(Handle tex);
};

////////////////////////////////////////////////////////////////////////////////
// pipeline-state

namespace PipelineState
{
Handle Create(const Descriptor& desc);
void Delete(Handle ps);
Handle CreateVertexConstBuffer(Handle ps, uint32 bufIndex);
Handle CreateFragmentConstBuffer(Handle ps, uint32 bufIndex);

uint32 VertexConstBufferCount(Handle ps);
uint32 VertexConstCount(Handle ps, uint32 bufIndex);
bool GetVertexConstInfo(Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info);

uint32 FragmentConstBufferCount(Handle ps);
uint32 FragmentConstCount(Handle ps, uint32 bufIndex);
bool GetFragmentConstInfo(Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info);

} // namespace PipelineState

namespace ConstBuffer
{
uint32 ConstCount(Handle cb);
bool SetConst(Handle cb, uint32 constIndex, uint32 constCount, const float* data);
bool SetConst(Handle cb, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount);
void Delete(Handle cb);

} // namespace ConstBuffer

namespace DepthStencilState
{
Handle Create(const Descriptor& desc);
void Delete(Handle state);
}

namespace SamplerState
{
Handle Create(const Descriptor& desc);
void Delete(Handle state);
}

namespace RenderPass
{
Handle Allocate(const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf);
void Begin(Handle pass);
void End(Handle pass);
}

namespace SyncObject
{
Handle Create();
void Delete(Handle obj);
bool IsSygnaled(Handle obj);
}

namespace CommandBuffer
{
void Begin(Handle cmdBuf);
void End(Handle cmdBuf, Handle syncObject = InvalidHandle);

void SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdecl = VertexLayout::InvalidUID);
void SetCullMode(Handle cmdBuf, CullMode mode);
void SetScissorRect(Handle cmdBuf, ScissorRect rect);
void SetViewport(Handle cmdBuf, Viewport vp);
void SetFillMode(Handle cmdBuf, FillMode mode);

void SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex = 0);
void SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer);
void SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex);

void SetIndices(Handle cmdBuf, Handle ib);

void SetQueryBuffer(Handle cmdBuf, Handle queryBuf);
void SetQueryIndex(Handle cmdBuf, uint32 index);

void IssueTimestampQuery(Handle cmdBuf, Handle perfQuery);

void SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buf);
void SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex);

void SetDepthStencilState(Handle cmdBuf, Handle depthStencilState);
void SetSamplerState(Handle cmdBuf, const Handle samplerState);

void DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count);
void DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex = 0, uint32 startIndex = 0);

void DrawInstancedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count);
void DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 primCount, uint32 vertexCount, uint32 firstVertex = 0, uint32 startIndex = 0, uint32 baseInstance = 0);

void SetMarker(Handle cmdBuf, const char* text);

} // namespace CommandBuffer

struct RenderDeviceCaps;
namespace MutableDeviceCaps
{
RenderDeviceCaps& Get();
}

void InitPacketListPool(uint32 maxCount);
void InitTextreSetPool(uint32 maxCount);

void PresentImpl(Handle sync);

// debug

extern uint32 stat_DIP;
extern uint32 stat_DP;
extern uint32 stat_DTL;
extern uint32 stat_DTS;
extern uint32 stat_DLL;
extern uint32 stat_SET_PS;
extern uint32 stat_SET_SS;
extern uint32 stat_SET_TEX;
extern uint32 stat_SET_CB;
extern uint32 stat_SET_VB;
extern uint32 stat_SET_IB;

} // namespace rhi



#define RHI_GL__USE_UNIFORMBUFFER_OBJECT 0
#define RHI_GL__USE_STATIC_CONST_BUFFER_OPTIMIZATION 0
#define RHI_GL__DEBUG_CONST_BUFFERS 0


#endif // __RHI_PRIVATE_H__
