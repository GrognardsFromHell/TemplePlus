#include "infrastructure/images.h"
#include "infrastructure/exception.h"

namespace gfx {

	enum class TgaColorMapType : uint8_t {
		TrueColor = 0,
		Indexed = 1
	};

	enum class TgaDataType : uint8_t {
		NoData = 0,
		UncompressedColorMapped = 1,
		UncompressedRgb = 2,
		UncompressedMono = 3,
		RleColorMapped = 9,
		RleRgb = 10,
		CompressedMono = 11,
		CompressedColorMapped = 32,
		CompressedColorMapped2 = 33
	};

#pragma pack(push, 1)
	struct TgaHeader {
		uint8_t imageIdLength;
		TgaColorMapType colorMapType;
		TgaDataType dataType;
		uint16_t colorMapOffset;
		uint16_t colorMapOrigin;
		uint8_t colorMapDepth;
		int16_t xOrigin;
		int16_t yOrigin;
		uint16_t width;
		uint16_t height;
		uint8_t bpp;
		uint8_t imageDescriptor;
	};
#pragma pack(pop)

	bool DetectTga(span<uint8_t> data, ImageFileInfo& info) {

		if (data.size() < sizeof(TgaHeader)) {
			return false;
		}

		auto header = reinterpret_cast<const TgaHeader*>(data.data());

		if (header->colorMapType != TgaColorMapType::TrueColor) {
			return false; // We don't supported index TGA
		}

		if (header->dataType != TgaDataType::UncompressedRgb) {
			return false; // We only support uncompressed RGB
		}

		if (header->bpp != 24 && header->bpp != 32) {
			return false; // We only support 24 or 32 bit TGAs
		}

		info.width = header->width;
		info.height = header->height;
		info.hasAlpha = (header->bpp == 32);
		info.format = ImageFileFormat::TGA;
		return true;

	}

	std::unique_ptr<uint8_t[]> DecodeTga(span<uint8_t> data) {

		if (data.size() < sizeof(TgaHeader)) {
			throw TempleException("Not enough data for TGA header");
		}

		auto header = reinterpret_cast<const TgaHeader*>(data.data());

		if (header->colorMapType != TgaColorMapType::TrueColor) {
			throw TempleException("Only true color TGA images are supported.");
		}

		if (header->dataType != TgaDataType::UncompressedRgb) {
			throw TempleException("Only uncompressed RGB TGA images are supported.");
		}

		if (header->bpp != 24 && header->bpp != 32) {
			throw TempleException("Only uncompressed RGB 24-bpp or 32-bpp TGA images are supported.");
		}

		auto result(std::make_unique<uint8_t[]>(header->width * header->height * 4));
		auto dest = result.get();

		// Points to the start of the TGA image data
		auto srcStart = data.data() + sizeof(TgaHeader) + header->imageIdLength;
		auto srcSize = data.size() - sizeof(TgaHeader) - header->imageIdLength;		

		if (header->bpp == 24) {
			auto srcPitch = header->width * 3;
			Expects((int) srcSize >= header->height * srcPitch);			
			for (int y = 0; y < header->height; ++y) {
				auto src = srcStart + (header->height - y - 1) * srcPitch;
				for (int x = 0; x < header->width; ++x) {
					*dest++ = *src++;
					*dest++ = *src++;
					*dest++ = *src++;
					*dest++ = 0xFF; // Fixed alpha
				}
			}
		} else {
			auto srcPitch = header->width * 4;
			Expects((int) srcSize >= header->height * srcPitch);
			for (int y = 0; y < header->height; ++y) {
				auto src = srcStart + (header->height - y - 1) * srcPitch;
				for (int x = 0; x < header->width; ++x) {
					*dest++ = *src++;
					*dest++ = *src++;
					*dest++ = *src++;
					*dest++ = *src++;
				}
			}
		}

		return result;
	}

}
