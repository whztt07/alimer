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

#include "../Graphics/PixelFormat.h"

namespace Alimer
{
	class Graphics;

	/// Texture types.
	enum class TextureType : uint32_t
	{
		Type1D = 0,
		Type2D,
		Type3D,
		TypeCube,
	};

	/// Defines a Texture class.
	class Texture
	{
	protected:
		/// Constructor.
		Texture(Graphics* graphics);

	public:
		/// Destructor.
		virtual ~Texture();

		inline TextureType GetTextureType() const { return _textureType; }
		inline PixelFormat GetFormat() const { return _format; }
		inline uint32_t GetWidth() const { return _width; }
		inline uint32_t GetHeight() const { return _height; }
	protected:
		Graphics* _graphics;
		TextureType _textureType;
		PixelFormat _format;
		uint32_t _width;
		uint32_t _height;
	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Texture);
	};
}