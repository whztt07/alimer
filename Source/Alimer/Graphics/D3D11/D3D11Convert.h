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

#include "D3D11Prerequisites.h"
#include "../Types.h"
#include "../PixelFormat.h"
#include "../GpuBuffer.h"

namespace Alimer
{
    namespace d3d11
    {
        static inline D3D11_USAGE Convert(ResourceUsage usage)
        {
            switch (usage)
            {
            case ResourceUsage::Default:
                return D3D11_USAGE_DEFAULT;
            case ResourceUsage::Immutable:
                return D3D11_USAGE_IMMUTABLE;
            case ResourceUsage::Dynamic:
                return D3D11_USAGE_DYNAMIC;
            case ResourceUsage::Staging:
                return D3D11_USAGE_STAGING;
            default:
                ALIMER_UNREACHABLE();
                return D3D11_USAGE_DEFAULT;
            }
        }
    }
}
