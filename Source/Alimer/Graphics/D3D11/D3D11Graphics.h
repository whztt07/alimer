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

#include "../../Core/Flags.h"
#include "../../Graphics/Graphics.h"
#include "D3D11Prerequisites.h"
#include <array>
#include <mutex>

namespace Alimer
{
	class D3D11CommandBuffer;
	class D3D11Texture;
    class D3D11SwapChain;

	/// D3D11 Low-level 3D graphics API class.
	class D3D11Graphics final : public Graphics
	{
	public:
		/// Is backend supported?
		static bool IsSupported();

		/// Constructor.
		D3D11Graphics(bool validation);

		/// Destructor.
		~D3D11Graphics() override;

        void WaitIdle() override;
        CommandBuffer* GetDefaultCommandBuffer() const override;

        SharedPtr<RenderPass> CreateRenderPass(const RenderPassDescription& description) override;
        SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferDescription& description, const void* initialData) override;

		SharedPtr<Shader> CreateComputeShader(const ShaderStageDescription& desc) override;
		SharedPtr<Shader> CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment) override;
        SharedPtr<PipelineState> CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) override;

        void HandleDeviceLost();

        // Getters
		inline IDXGIFactory2* GetDXGIFactory() const { return _dxgiFactory.Get(); }
		inline D3D_FEATURE_LEVEL GetFeatureLevel() const { return _d3dFeatureLevel; }
		inline ID3D11Device1* GetD3DDevice() const { return _d3dDevice.Get(); }
        inline ID3D11DeviceContext1* GetImmediateContext() const { return _d3dContext.Get(); }

	private:
        void Finalize() override;
        bool BackendInitialize() override;
        SharedPtr<RenderPass> BeginFrameCore() override;
        void EndFrameCore() override;
		bool InitializeCaps();
        

        Microsoft::WRL::ComPtr<IDXGIFactory2>               _dxgiFactory;
        D3D_FEATURE_LEVEL                                   _d3dFeatureLevel;
        Microsoft::WRL::ComPtr<ID3D11Device1>               _d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1>        _d3dContext;
        Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>   _d3dAnnotation;

        D3D11SwapChain* _swapChain = nullptr;
        D3D11CommandBuffer* _immediateCommandBuffer = nullptr;
	};
}
