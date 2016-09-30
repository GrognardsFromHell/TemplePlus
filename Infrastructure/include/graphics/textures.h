
#pragma once

#include <unordered_map>

struct ID3D11ShaderResourceView;

namespace gfx {

	class RenderingDevice;

	struct ContentRect {
		int x;
		int y;
		int width;
		int height;
	};

	struct Size {
		int width;
		int height;

		bool operator==(const Size &o) const {
			return width == o.width && height == o.height;
		}
	};

	// One of the predefined texture types
	enum class TextureType {
		Invalid,
		Dynamic,
		File,
		RenderTarget
	};

	/*
	Represents a game texture in either an unloaded or
	loaded state.
	*/
	class Texture {
	public:

		virtual ~Texture() {
		}

		virtual int GetId() const = 0;

		virtual const std::string& GetName() const = 0;

		virtual const ContentRect& GetContentRect() const = 0;

		virtual const Size& GetSize() const = 0;

		virtual bool IsValid() const {
			return true;
		}

		// Unloads the device texture (does't prevent it from being loaded again later)
		virtual void FreeDeviceTexture() = 0;

		virtual ID3D11ShaderResourceView* GetResourceView() = 0;

		virtual TextureType GetType() const = 0;

		static std::shared_ptr<class Texture> GetInvalidTexture();
		
	};

	using TextureRef = std::shared_ptr<Texture>;

	class Textures {
	friend class Texture;
	public:

		Textures(RenderingDevice &device, size_t memoryBudget);

		/*
			Call this after every frame to free texture memory by
			freeing the least recently used textures.
		*/
		void FreeUnusedTextures();

		// Frees all GPU texture memory i.e. after a device reset
		void FreeAllTextures();

		gfx::TextureRef Resolve(const std::string& filename, bool withMipMaps);
		gfx::TextureRef Override(const std::string& filename, bool withMipMaps);
		
		gfx::TextureRef ResolveUncached(const std::string& filename, bool withMipMaps);

		gfx::TextureRef GetById(int textureId);

		int GetLoaded();
		int GetRegistered();
		size_t GetUsageEstimate();
		size_t GetMemoryBudget();

	private:
		std::shared_ptr<class TextureLoader> mLoader;
		int mNextFreeId = 1;
		std::unordered_map<int, gfx::TextureRef> mTexturesById;
		std::unordered_map<std::string, gfx::TextureRef> mTexturesByName;
	};

}
