#include "infrastructure/exception.h"
#include "infrastructure/images.h"
#include "infrastructure/binaryreader.h"
#include "infrastructure/vfs.h"

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

	static std::string BuildTgaFilenamePattern(const std::string &imgFilename) {
		// Build a pattern for the actual tga filenames
		std::string filenamePattern(imgFilename);
		auto dotPos = filenamePattern.rfind('.');
		if (dotPos != std::string::npos) {
			filenamePattern.erase(dotPos);
		}
		filenamePattern.append("_{}_{}.tga");
		return filenamePattern;
	}

	DecodedImage DecodeCombinedImage(const std::string &filename, const span<uint8_t> imgData) {

		BinaryReader reader(imgData);

		DecodedImage result;
		result.info.format = ImageFileFormat::IMG;
		result.info.hasAlpha = true; // Depends on the TGAs, actually
		result.info.width = reader.Read<uint16_t>();
		result.info.height = reader.Read<uint16_t>();

		auto filenamePattern = BuildTgaFilenamePattern(filename);

		result.data = std::make_unique<uint8_t[]>(result.info.width * result.info.height * 4);
		auto stride = result.info.width * 4;

		int yTile = 0;
		for (int yCur = 0; yCur < result.info.height; yCur += 256, yTile++) {
			int xTile = 0;
			for (int xCur = 0; xCur < result.info.width; xCur += 256, xTile++) {

				auto filename = fmt::format(filenamePattern, xTile, yTile);
				auto tileData(DecodeImage(vfs->ReadAsBinary(filename)));
				
				// Must fit into remaining image
				Expects(tileData.info.width <= result.info.width - xCur);
				Expects(tileData.info.height <= result.info.height - yCur);

				// Have to copy row by row
				auto srcStride = tileData.info.width * 4;
				for (auto row = 0; row < tileData.info.height; row++) {
					// Convert from Y-up to Y-down axis
					auto targetRow = result.info.height - yCur - tileData.info.height + row;
					auto dest = &result.data[targetRow * stride + xCur * 4];
					auto src = &tileData.data[row * srcStride];
					memcpy(dest, src, srcStride);
				}
			}
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

	HCURSOR LoadImageToCursor(const span<uint8_t> data, uint32_t hotspotX, uint32_t hotspotY) {

		auto img(DecodeImage(data));
		auto w = img.info.width;
		auto h = img.info.height;

		if (!img.info.hasAlpha) {
			throw TempleException("Cursor has no alpha channel");
		}

		BITMAPV5HEADER bi;
		ZeroMemory(&bi, sizeof(BITMAPV5HEADER));
		bi.bV5Size = sizeof(BITMAPV5HEADER);
		bi.bV5Width = w;
		bi.bV5Height = - h;
		bi.bV5Planes = 1;
		bi.bV5BitCount = 32;
		bi.bV5Compression = BI_BITFIELDS;
		bi.bV5RedMask = 0x00FF0000;
		bi.bV5GreenMask = 0x0000FF00;
		bi.bV5BlueMask = 0x000000FF;
		bi.bV5AlphaMask = 0xFF000000;
		HDC hdc;
		hdc = GetDC(NULL);
		
		void *lpBits = nullptr;
		auto hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS,
			(void **)&lpBits, NULL, (DWORD)0);

		memcpy(lpBits, img.data.get(), w * h * 4);

		ReleaseDC(NULL, hdc);

		auto hMonoBitmap = CreateBitmap(w, h, 1, 1, NULL);

		ICONINFO ii;
		ii.fIcon = FALSE;
		ii.xHotspot = hotspotX;
		ii.yHotspot = hotspotY;
		ii.hbmMask = hMonoBitmap;
		ii.hbmColor = hBitmap;

		auto hAlphaCursor = CreateIconIndirect(&ii);

		DeleteObject(hBitmap);
		DeleteObject(hMonoBitmap);

		return hAlphaCursor;
	}

}
