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

#include "../Resource/Resource.h"
#include <memory>
#include <atomic>

namespace Alimer
{
	class Stream;

	/// Runtime resource loader class.
	class ResourceLoader
	{
	protected:
		/// Constructor.
		ResourceLoader();

	public:
		/// Destructor.
		virtual ~ResourceLoader() = default;

		/// Load the resource synchronously from a binary stream. Return instance on success.
		Resource* Load(Stream& source);

	protected:
		virtual bool BeginLoad(Stream& source) = 0;
		virtual Resource* EndLoad() = 0;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(ResourceLoader);
	};
}
