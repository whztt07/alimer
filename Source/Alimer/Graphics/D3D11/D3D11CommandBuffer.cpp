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

#include "D3D11CommandBuffer.h"
#include "D3D11Graphics.h"
#include "D3D11Texture.h"
#include "D3D11RenderPass.h"
#include "D3D11Shader.h"
#include "D3D11PipelineState.h"
#include "D3D11GpuBuffer.h"
#include "../D3D/D3DConvert.h"
#include "../../Util/HashMap.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

static_assert(sizeof(D3D11_VIEWPORT) == sizeof(Alimer::Viewport), "D3D11_VIEWPORT mismatch");
static_assert(offsetof(Alimer::Viewport, x) == offsetof(D3D11_VIEWPORT, TopLeftX), "D3D11_VIEWPORT TopLeftX offset mismatch");
static_assert(offsetof(Alimer::Viewport, y) == offsetof(D3D11_VIEWPORT, TopLeftY), "D3D11_VIEWPORT TopLeftY offset mismatch");
static_assert(offsetof(Alimer::Viewport, width) == offsetof(D3D11_VIEWPORT, Width), "D3D11_VIEWPORT Width offset mismatch");
static_assert(offsetof(Alimer::Viewport, height) == offsetof(D3D11_VIEWPORT, Height), "D3D11_VIEWPORT Height offset mismatch");
static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(D3D11_VIEWPORT, MinDepth), "D3D11_VIEWPORT MinDepth offset mismatch");
static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(D3D11_VIEWPORT, MaxDepth), "D3D11_VIEWPORT MaxDepth offset mismatch");

namespace Alimer
{
    static ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};

    D3D11CommandBuffer::D3D11CommandBuffer(D3D11Graphics* graphics, bool immediate)
        : CommandBuffer()
        , _graphics(graphics)
        , _immediate(immediate)
    {
    }

    D3D11CommandBuffer::~D3D11CommandBuffer() = default;

    void D3D11CommandBuffer::Execute(D3D11CommandContext* context)
    {
        while (Command* command = Front())
        {
            switch (command->type)
            {
            case Command::Type::BeginRenderPass:
            {
                const BeginRenderPassCommand* beginRenderPassCommand = static_cast<const BeginRenderPassCommand*>(command);
                context->BeginRenderPassCore(
                    beginRenderPassCommand->renderPass,
                    beginRenderPassCommand->renderArea,
                    beginRenderPassCommand->clearColors.data(),
                    static_cast<uint32_t>(beginRenderPassCommand->clearColors.size()),
                    beginRenderPassCommand->clearDepth,
                    beginRenderPassCommand->clearStencil
                );
            }
            break;

            case Command::Type::EndRenderPass:
            {
                context->EndRenderPassCore();
            }
            break;

            case Command::Type::SetViewport:
            {
                const SetViewportCommand* typedCmd = static_cast<const SetViewportCommand*>(command);
                context->SetViewport(typedCmd->viewport);
            }
            break;

            default:
                ALIMER_LOGCRITICAL("Invalid CommandBuffer command");
                break;
            }

            Pop();
        }
    }

    // D3D11CommandContext
    D3D11CommandContext::D3D11CommandContext(D3D11Graphics* graphics, ID3D11DeviceContext1* context)
        : D3D11CommandBuffer(graphics, true)
        , _context(context)
    {
        Reset();
    }

    D3D11CommandContext::~D3D11CommandContext()
    {

    }

    void D3D11CommandContext::Reset()
    {
        _currentRenderPass = nullptr;
        _currentColorAttachmentsBound = 0;
        _currentTopology = PrimitiveTopology::Count;
        _currentShader = nullptr;
        _currentRasterizerState = nullptr;
        _currentDepthStencilState = nullptr;
        _currentBlendState = nullptr;

        _currentInputLayout.first = 0;
        _currentInputLayout.second = 0;

        //_dirtyVbos = ~0u;
        memset(_vbo.buffers, 0, sizeof(_vbo.buffers));
        memset(_vbo.d3dBuffers, 0, sizeof(_vbo.d3dBuffers));
        _inputLayoutDirty = false;
    }

    void D3D11CommandContext::BeginRenderPassCore(RenderPass* renderPass, const Rectangle& renderArea, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        if (!renderPass)
        {
            renderPass = _graphics->GetBackbufferRenderPass();
        }

        _currentRenderPass = static_cast<D3D11RenderPass*>(renderPass);
        _currentRenderPass->Bind(_context);
        _currentColorAttachmentsBound = renderPass->GetColorAttachmentsCount();

        for (uint32_t i = 0; i < _currentColorAttachmentsBound; ++i)
        {
            const RenderPassAttachment& colorAttachment = renderPass->GetColorAttachment(i);

            switch (colorAttachment.loadAction)
            {
            case LoadAction::Clear:
                _context->ClearRenderTargetView(_currentRenderPass->GetRTV(i), &clearColors[i].r);
                break;

            default:
                break;
            }
        }

        // Set viewport and scissor from render area.
        Rectangle setRenderArea = renderArea;
        if (renderArea.IsEmpty())
        {
            setRenderArea = Rectangle((int32_t)renderPass->GetWidth(), (int32_t)renderPass->GetHeight());
        }

        D3D11_VIEWPORT d3dViewport = {
            static_cast<float>(setRenderArea.x), static_cast<float>(setRenderArea.y),
            static_cast<float>(setRenderArea.width), static_cast<float>(setRenderArea.height),
            0.0f, 1.0f
        };
        _context->RSSetViewports(1, &d3dViewport);
        SetScissor(setRenderArea);
    }

    void D3D11CommandContext::EndRenderPassCore()
    {
        _context->OMSetRenderTargets(_currentColorAttachmentsBound, nullViews, nullptr);
        _currentRenderPass = nullptr;
    }

    void D3D11CommandContext::SetViewport(const Viewport& viewport)
    {
        _context->RSSetViewports(1, (D3D11_VIEWPORT*)&viewport);
    }

    void D3D11CommandContext::SetViewports(uint32_t numViewports, const Viewport* viewports)
    {
        _context->RSSetViewports(numViewports, (D3D11_VIEWPORT*)viewports);
    }

    void D3D11CommandContext::SetScissor(const Rectangle& scissor)
    {
        D3D11_RECT scissorD3D;
        scissorD3D.left = scissor.x;
        scissorD3D.top = scissor.y;
        scissorD3D.right = scissor.x + scissor.width;
        scissorD3D.bottom = scissor.y + scissor.height;
        _context->RSSetScissorRects(1, &scissorD3D);
    }

    void D3D11CommandContext::SetScissors(uint32_t numScissors, const Rectangle* scissors)
    {
        numScissors = std::min(numScissors, static_cast<uint32_t>(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
        D3D11_RECT scissorsD3D[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (uint32_t i = 0; i < numScissors; ++i)
        {
            const Rectangle& src = scissors[i];
            D3D11_RECT& dst = scissorsD3D[i];

            dst.left = src.x;
            dst.top = src.y;
            dst.right = src.x + src.width;
            dst.bottom = src.y + src.height;
        }

        _context->RSSetScissorRects(numScissors, scissorsD3D);
    }

    void D3D11CommandContext::SetShaderCore(Shader* shader)
    {
        _currentShader = static_cast<D3D11Shader*>(shader);
    }

    void D3D11CommandContext::SetVertexBufferCore(uint32_t binding, VertexBuffer* buffer, uint64_t offset, uint64_t stride, VertexInputRate inputRate)
    {
        if (_vbo.buffers[binding] != buffer
            || _vbo.offsets[binding] != offset)
        {
            _vbo.d3dBuffers[binding] = static_cast<D3D11GpuBuffer*>(buffer->GetHandle())->GetD3DBuffer();

            _dirtyVbos |= 1u << binding;
            _inputLayoutDirty = true;
        }

        if (_vbo.strides[binding] != stride
            || _vbo.inputRates[binding] != inputRate)
        {
            //set_dirty(COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT);
        }

        _vbo.buffers[binding] = buffer;
        _vbo.offsets[binding] = offset;
        _vbo.strides[binding] = stride;
        _vbo.inputRates[binding] = inputRate;
    }


    void D3D11CommandContext::SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType)
    {
        ID3D11Buffer* d3dBuffer = static_cast<D3D11GpuBuffer*>(buffer->GetHandle())->GetD3DBuffer();
        DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R16_UINT;
        if (indexType == IndexType::UInt32)
        {
            dxgiFormat = DXGI_FORMAT_R32_UINT;
        }

        _context->IASetIndexBuffer(d3dBuffer, dxgiFormat, offset);
    }

    void D3D11CommandContext::SetUniformBufferCore(uint32_t set, uint32_t binding, GpuBuffer* buffer, uint64_t offset, uint64_t range)
    {
        ID3D11Buffer* d3dBuffer = static_cast<D3D11GpuBuffer*>(buffer->GetHandle())->GetD3DBuffer();
        _context->VSSetConstantBuffers(binding, 1, &d3dBuffer);
    }

    void D3D11CommandContext::SetTextureCore(uint32_t binding, Texture* texture, ShaderStageFlags stage)
    {
        auto d3d11Texture = static_cast<D3D11Texture*>(texture);
        ID3D11ShaderResourceView* shaderResourceView =  d3d11Texture->GetShaderResourceView();
        ID3D11SamplerState* samplerState = d3d11Texture->GetSamplerState();

        if (stage & ShaderStage::Compute)
        {
            _context->CSSetShaderResources(binding, 1, &shaderResourceView);
            _context->CSSetSamplers(binding, 1, &samplerState);
        }

        if (stage & ShaderStage::Vertex)
        {
            _context->VSSetShaderResources(binding, 1, &shaderResourceView);
            _context->VSSetSamplers(binding, 1, &samplerState);
        }

        if (stage & ShaderStage::Fragment)
        {
            _context->PSSetShaderResources(binding, 1, &shaderResourceView);
            _context->PSSetSamplers(binding, 1, &samplerState);
        }
    }

    bool D3D11CommandContext::PrepareDraw(PrimitiveTopology topology)
    {
        uint32_t updateVboMask = _dirtyVbos/* & _currentPipeline->GetBindingMask()*/;
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
        {
            UpdateVbos(binding, count);
        });
        _dirtyVbos &= ~updateVboMask;

        if (_currentTopology != topology)
        {
            _context->IASetPrimitiveTopology(d3d::Convert(topology));
            _currentTopology = topology;
        }

        _currentShader->Bind(_context);

        /*auto rasterizerState = _currentPipeline->GetD3DRasterizerState();
        if (_currentRasterizerState != rasterizerState)
        {
            _currentRasterizerState = rasterizerState;
            _context->RSSetState(rasterizerState);
        }

        auto dsState = _currentPipeline->GetD3DDepthStencilState();
        if (_currentDepthStencilState != dsState)
        {
            _currentDepthStencilState = dsState;
            _context->OMSetDepthStencilState(dsState, 0);
        }

        auto blendState = _currentPipeline->GetD3DBlendState();
        if (_currentBlendState != blendState)
        {
            _currentBlendState = blendState;
            _context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
        }*/

        return true;
    }

    void D3D11CommandContext::UpdateVbos(uint32_t binding, uint32_t count)
    {
#ifdef ALIMER_DEV
        for (uint32_t i = binding; i < binding + count; i++)
        {
            ALIMER_ASSERT(_vbo.buffers[i] != nullptr);
        }
#endif
        _context->IASetVertexBuffers(
            binding,
            count,
            _vbo.d3dBuffers + binding,
            _vbo.strides + binding,
            _vbo.offsets + binding);

        // InputLayout
        if (_inputLayoutDirty)
        {
            _inputLayoutDirty = false;

            Hasher inputLayoutHasher;
            
            for (uint32_t i = binding; i < binding + count; i++)
            {
                inputLayoutHasher.u32(binding);
                inputLayoutHasher.u64(_vbo.buffers[i]->GetVertexFormat().GetHash());
            }

            InputLayoutDesc newInputLayout;
            newInputLayout.first = inputLayoutHasher.get();
            newInputLayout.second = _currentShader->GetVertexShaderHash();

            if (_currentInputLayout != newInputLayout)
            {
                // Check if layout already exists
                auto inputLayout = _graphics->GetInputLayout(newInputLayout);
                if (inputLayout != nullptr)
                {
                    _context->IASetInputLayout(inputLayout);
                    _currentInputLayout = newInputLayout;
                }
                else
                {
                    // Not found, create new
                    D3D11_INPUT_ELEMENT_DESC d3dElementDescs[MaxVertexAttributes];
                    memset(d3dElementDescs, 0, sizeof(d3dElementDescs));

                    UINT attributeCount = 0;
                    for (uint32_t i = binding; i < binding + count; i++)
                    {
                        const VertexFormat& vertexFormat = _vbo.buffers[i]->GetVertexFormat();
                        const std::vector<VertexElement>& elements = vertexFormat.GetElements();

                        uint32_t attributeIndex = 0;
                        for (const VertexElement& element : vertexFormat.GetElements())
                        {
                            D3D11_INPUT_ELEMENT_DESC* d3d11ElementDesc = &d3dElementDescs[attributeCount++];

                            d3d11ElementDesc->SemanticName = "TEXCOORD";
                            d3d11ElementDesc->SemanticIndex = attributeIndex++;
                            //d3d11ElementDesc->SemanticName = element.semanticName;
                            //d3d11ElementDesc->SemanticIndex = element.semanticIndex;
                            d3d11ElementDesc->Format = d3d::Convert(element.format);
                            d3d11ElementDesc->InputSlot = i;
                            d3d11ElementDesc->AlignedByteOffset = element.offset;
                            if (_vbo.inputRates[i] == VertexInputRate::Instance)
                            {
                                d3d11ElementDesc->InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                                d3d11ElementDesc->InstanceDataStepRate = 1;
                            }
                        }
                    }

                    auto vsByteCode = _currentShader->AcquireVertexShaderBytecode();
                    ID3D11InputLayout* d3dInputLayout = nullptr;

                    if (FAILED(_graphics->GetD3DDevice()->CreateInputLayout(
                        d3dElementDescs,
                        attributeCount,
                        vsByteCode.data(),
                        vsByteCode.size(), &d3dInputLayout)))
                    {
                        ALIMER_LOGERROR("Failed to create input layout");
                    }
                    else
                    {
                        _graphics->StoreInputLayout(newInputLayout, d3dInputLayout);
                        _context->IASetInputLayout(d3dInputLayout);
                        _currentInputLayout = newInputLayout;
                    }
                }
            }
        }
    }

    void D3D11CommandContext::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!PrepareDraw(topology))
            return;

        if (instanceCount <= 1)
        {
            _context->Draw(vertexCount, vertexStart);
        }
        else
        {
            _context->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
        }
    }

    void D3D11CommandContext::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        if (!PrepareDraw(topology))
            return;

        if (instanceCount <= 1)
        {
            _context->DrawIndexed(indexCount, startIndex, 0);
        }
        else
        {
            _context->DrawIndexedInstanced(indexCount, instanceCount, startIndex, 0, 0);
        }
    }

    
}

