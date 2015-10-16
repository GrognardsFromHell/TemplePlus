#include "infrastructure/textures.h"

namespace gfx {

	class InvalidTexture : public gfx::Texture {
	public:
		const std::string& GetName() const override {
			static std::string sInvalidName("<invalid>");
			return sInvalidName;
		}

		int GetId() const override {
			return -1;
		}

		const gfx::ContentRect& GetContentRect() const override {
			static gfx::ContentRect rect{ 0,0,1,1 };
			return rect;
		}

		const gfx::Size& GetSize() const override {
			static gfx::Size size{ 1, 1 };
			return size;
		}

		void FreeDeviceTexture() override {
		}

		IDirect3DTexture9* GetDeviceTexture() override {
			return nullptr;
		}

		bool IsValid() const override {
			return false;
		}
	};

	std::shared_ptr<class Texture> Texture::GetInvalidTexture() {
		static std::shared_ptr<Texture> instance(std::make_shared<InvalidTexture>());
		return instance;
	}

	TextureManager *textureManager;

}
