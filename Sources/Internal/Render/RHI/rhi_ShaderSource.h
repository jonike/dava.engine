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


#ifndef __RHI_SHADERSOURCE_H__
#define __RHI_SHADERSOURCE_H__

    #include "rhi_Type.h"
    #include "Base/BaseTypes.h"
    using DAVA::uint32;
    #include "Base/FastName.h"
    using DAVA::FastName;


namespace rhi
{

struct 
ShaderProp 
{
    enum Type   { TYPE_FLOAT1, TYPE_FLOAT2, TYPE_FLOAT3, TYPE_FLOAT4, TYPE_FLOAT4X4 };
    enum Scope  { SCOPE_UNIQUE, SCOPE_SHARED };

    FastName    uid;
    Type        type;
    uint32      arraySize;
    Scope       scope;
    FastName    tag;
    uint32      bufferindex;
    uint32      bufferReg;
    uint32      bufferRegCount;
    float       defaultValue[16];
};

typedef std::vector<ShaderProp> ShaderPropList;

struct
ShaderSampler
{
    TextureType type;
    FastName    uid;
};

typedef std::vector<ShaderSampler> ShaderSamplerList;


class
ShaderSource
{
public:
                            ShaderSource();
                            ~ShaderSource();

    bool                    Construct( ProgType progType, const char* srcText, const std::vector<std::string>& defines );
    bool                    Construct( ProgType progType, const char* srcText );

    const char*             SourceCode() const;
    const ShaderPropList&   Properties() const;
    const ShaderSamplerList&FragmentSamplers() const;
    const ShaderSamplerList&VertexSamplers() const;
    const VertexLayout&     ShaderVertexLayout() const;
    uint32                  ConstBufferCount() const;
    uint32                  ConstBufferSize( uint32 bufIndex ) const;
    ShaderProp::Scope       ConstBufferScope( uint32 bufIndex ) const;
    BlendState              Blending();

    void                    Dump() const;


private:

    void                    _Reset();
    void                    _AppendLine( const char* line, uint32 lineLen );

    struct
    buf_t
    {
        ShaderProp::Scope   scope;
        FastName            tag;
        uint32              regCount;
        std::vector<int>    avlRegIndex;
    };


    ProgType                    type;
    std::string                 code;
    uint32                      codeLineCount;
    VertexLayout                vdecl;
    std::vector<ShaderProp>     prop;
    std::vector<buf_t>          buf;
    std::vector<ShaderSampler>  fragmentSampler;
    std::vector<ShaderSampler>  vertexSampler;
    BlendState                  blending;
};



} // namespace rhi
#endif // __RHI_SHADERSOURCE_H__

