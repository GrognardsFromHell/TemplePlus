#include "infrastructure/exception.h"
#include "infrastructure/images.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#define STBI_ONLY_BMP
#include "stb_image.h"

#include <turbojpeg.h>

#include "platform/d3d.h"

namespace gfx {

	class TjDecompressHandle {
	public:
		TjDecompressHandle() : mHandle(tjInitDecompress()) {
			if (!mHandle) {
				throw TempleException("Unable to initialize libjpeg-turbo decompressor: {}",
				                      tjGetErrorStr());
			}
		}

		~TjDecompressHandle() {
			tjDestroy(mHandle);
		}

		operator tjhandle() {
			return mHandle;
		}

	private:
		tjhandle mHandle;
	};

	ImageFileInfo DetectImageFormat(span<uint8_t> data) {
		ImageFileInfo info;

		stbi__context ctx;
		stbi__start_mem(&ctx, &data[0], data.size_bytes());

		int comp;
		if (stbi__bmp_info(&ctx, &info.width, &info.height, &comp)) {
			info.hasAlpha = (comp == 4);
			info.format = ImageFileFormat::BMP;
			return info;
		}

		TjDecompressHandle handle;
		if (tjDecompressHeader(handle, &data[0], data.size_bytes(), &info.width, &info.height) == 0) {
			info.hasAlpha = false;
			info.format = ImageFileFormat::JPEG;
			return info;
		}

		if (DetectTga(data, info)) {
			return info;
		}

		// Not a very good heuristic
		if (data.size() == 256 * 256) {
			info.width = 256;
			info.height = 256;
			info.hasAlpha = true;
			info.format = ImageFileFormat::FNTART;
			return info;
		}

		return info;
	}

	std::unique_ptr<uint8_t[]> DecodeJpeg(const span<uint8_t> data) {
		TjDecompressHandle handle;

		int w, h;
		tjDecompressHeader(handle, &data[0], data.size_bytes(), &w, &h);

		std::unique_ptr<uint8_t[]> result(new uint8_t[w * h * 4]);
		auto status = tjDecompress2(handle, &data[0], data.size_bytes(), &result[0], w, w * 4, h, TJPF_BGRX, 0);
		if (status != 0) {
			throw TempleException("Unable to decompress jpeg image: {}",
			                      tjGetErrorStr());
		}
		return result;
	}

	DecodedImage DecodeFontArt(const span<uint8_t> data) {

		// 256x256 image with 8bit alpha
		Expects(data.size() == 256 * 256);

		DecodedImage result;
		result.info.width = 256;
		result.info.height = 256;
		result.info.format = ImageFileFormat::FNTART;
		result.info.hasAlpha = true;
		result.data = std::make_unique<uint8_t[]>(256 * 256 * 4);

		auto *dest = result.data.get();
		for (auto alpha : data) {			
			*dest++ = 0xFF;
			*dest++ = 0xFF;
			*dest++ = 0xFF;
			*dest++ = alpha;
		}

		return result;

	}

	DecodedImage DecodeImage(const span<uint8_t> data) {

		DecodedImage result;
		result.info = DetectImageFormat(data);

		stbi__context ctx;
		stbi__start_mem(&ctx, &data[0], data.size_bytes());
		int w, h, comp;

		switch (result.info.format) {
		case ImageFileFormat::BMP:
			result.data.reset(stbi__bmp_load(&ctx, &w, &h, &comp, 4));
			break;
		case ImageFileFormat::JPEG:
			result.data = DecodeJpeg(data);
			break;
		case ImageFileFormat::TGA:
			result.data = DecodeTga(data);
			break;
		case ImageFileFormat::FNTART:
			return DecodeFontArt(data);
		default:
		case ImageFileFormat::Unknown:
			throw TempleException("Unrecognized image format.");
		}

		return result;
	}

	CComPtr<IDirect3DSurface9> LoadImageToSurface(IDirect3DDevice9* device, const span<uint8_t> data, ImageFileInfo& info) {
		auto img(DecodeImage(data));
		auto w = img.info.width;
		auto h = img.info.height;
		info = img.info;

		auto format = img.info.hasAlpha ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;

		CComPtr<IDirect3DSurface9> imgSurface;
		if (!SUCCEEDED(device->CreateOffscreenPlainSurface(w, h, format, D3DPOOL_DEFAULT, &imgSurface, nullptr))) {
			throw TempleException("Unable to create offscreen surface for image");
		}

		D3DLOCKED_RECT locked;
		auto result = imgSurface->LockRect(&locked, nullptr, D3DLOCK_DISCARD);
		if (!SUCCEEDED(result)) {
			throw TempleException("Unable to lock result surface");
		}

		auto dest = reinterpret_cast<uint8_t*>(locked.pBits);
		auto src = img.data.get();
		if (locked.Pitch != w * 4) {
			for (auto y = 0; y < h; ++y) {
				memcpy(dest, src, w * 4);
				dest += locked.Pitch;
				src += w * 4;
			}
		} else {
			memcpy(dest, src, w * h * 4);
		}

		imgSurface->UnlockRect();

		return imgSurface;
	}

}
