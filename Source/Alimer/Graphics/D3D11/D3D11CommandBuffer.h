//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Graphics/CommandBuffer.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class D3D11RenderPass;
    class D3D11Shader;
    class D3D11Graphics;
    class D3D11CommandContext;

    /// D3D11 CommandBuffer implementation.
    class D3D11CommandBuffer : public CommandBuffer
    {
    public:
        /// Constructor.
        D3D11CommandBuffer(D3D11Graphics* graphics, bool immediate);

        /// Destructor.
        ~D3D11CommandBuffer() override;

        void Execute(D3D11CommandContext* context);

        bool IsImmediate() const { return _immediate; }

    protected:
        D3D11Graphics* _graphics;
        bool _immediate;
    };

    class D3D11CommandContext final : public D3D11CommandBuffer
    {
    public:
        /// Constructor.
        D3D11CommandContext(D3D11Graphics* graphics, ID3D11DeviceContext1* context);

        /// Destructor.
        ~D3D11CommandContext() override;

        void Reset();

        void BeginRenderPassCore(RenderPass* renderPass, const Rectangle& renderArea, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil) override;
        void EndRenderPassCore() override;
        void SetViewport(const Viewport& viewport) override;
        void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Rectangle& scissor) override;
        void SetScissors(uint32_t numScissors, const Rectangle* scissors) override;

        void SetShaderCore(Shader* shader) override;
        void SetVertexBufferCore(uint32_t binding, VertexBuffer* buffer, uint64_t offset, uint64_t stride, VertexInputRate inputRate) override;
        void SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType) override;
        void SetUniformBufferCore(uint32_t set, uint32_t binding, GpuBuffer* buffer, uint64_t offset, uint64_t range) override;
        void SetTextureCore(uint32_t binding, Texture* texture, ShaderStageFlags stage) override;

        void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) override;
        void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) override;

    private:
        bool PrepareDraw(PrimitiveTopology topology);
        void UpdateVbos(uint32_t binding, uint32_t count);

    private:
        ID3D11DeviceContext1 * _context;

        D3D11RenderPass* _currentRenderPass;
        uint32_t _currentColorAttachmentsBound;
        PrimitiveTopology _currentTopology;
        D3D11Shader* _currentShader;
        ID3D11RasterizerState1* _currentRasterizerState;
        ID3D11DepthStencilState* _currentDepthStencilState;
        ID3D11BlendState1* _currentBlendState;

        struct VertexBindingState
        {
            VertexBuffer* buffers[MaxVertexBufferBindings];
            ID3D11Buffer* d3dBuffers[MaxVertexBufferBindings];
            uint32_t offsets[MaxVertexBufferBindings];
            uint32_t strides[MaxVertexBufferBindings];
            VertexInputRate inputRates[MaxVertexBufferBindings];
        };

        VertexBindingState _vbo = {};
        bool _inputLayoutDirty;
        uint32_t _dirtyVbos = 0;

        /// Current input layout: vertex buffers' element mask and vertex shader's element mask combined.
        InputLayoutDesc _currentInputLayout;
    };
}
