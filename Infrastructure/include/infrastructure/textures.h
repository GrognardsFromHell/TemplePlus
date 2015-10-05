#pragma once

#include <memory>
#include <string>
#include "../platform/d3d.h"

namespace gfx {

	struct ContentRect {
		int x;
		int y;
		int width;
		int height;
	};

	struct Size {
		int width;
		int height;
	};

	/*
		Represents a game texture in either an unloaded or 
		loaded state.
	*/
	class Texture {
	public:
		virtual ~Texture() {
		}
		
		virtual const std::string &GetName() const = 0;

		virtual const ContentRect &GetContentRect() const = 0;

		virtual const Size &GetSize() const = 0;
		
		virtual bool IsValid() const {
			return true;
		}

		virtual IDirect3DTexture9 *GetDeviceTexture() = 0;

	};

	using TextureRef = std::shared_ptr<Texture>;

	/*
		Manages textures.
	*/
	class TextureManager {
	public:
		virtual ~TextureManager() {
		}

		virtual TextureRef Resolve(const std::string& filename) = 0;
	};

	extern std::unique_ptr<TextureManager> textureManager;

}
