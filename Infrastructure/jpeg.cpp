#include <turbojpeg.h>

#include "infrastructure/images.h"
#include "infrastructure/exception.h"

namespace gfx {

	namespace {

		TempleException CreateTjError(const char* msg) {
			return TempleException("{}: {}", msg, tjGetErrorStr());
		}

		class JpegEncoder {
		public:
			JpegEncoder() : mHandle(tjInitTransform()) {
				if (!mHandle) {
					throw CreateTjError("Unable to initialize JPEG encoder.");
				}
			}

			~JpegEncoder() {
				tjDestroy(mHandle);
			}

			operator tjhandle() {
				return mHandle;
			}

		private:
			tjhandle mHandle;
		};

	}

	static TJPF ConvertPixelFormat(JpegPixelFormat format) {
		switch (format) {
		case JpegPixelFormat::RGB:
			return TJPF_RGB;
		case JpegPixelFormat::BGR:
			return TJPF_BGR;
		case JpegPixelFormat::RGBX:
			return TJPF_RGBX;
		case JpegPixelFormat::BGRX:
			return TJPF_BGRX;
		case JpegPixelFormat::XBGR:
			return TJPF_XBGR;
		case JpegPixelFormat::XRGB:
			return TJPF_XRGB;
		default:
			throw TempleException("Unknown pixel format.");
		}
	}

	std::vector<uint8_t> EncodeJpeg(const uint8_t* imageData,
	                                JpegPixelFormat imageDataFormat,
	                                int width,
	                                int height,
	                                int quality,
	                                int pitch = 0) {

		JpegEncoder encoder;
		auto pf = ConvertPixelFormat(imageDataFormat);
		constexpr auto subsampling = TJSAMP_444;
		auto bufSize = tjBufSize(width, height, subsampling);

		std::vector<uint8_t> resultData(bufSize, 0);
		auto outputBuffer = &resultData[0];

		auto result = tjCompress2(encoder,
		                          const_cast<uint8_t*>(imageData),
		                          width,
		                          pitch,
		                          height,
		                          pf,
		                          &outputBuffer,
		                          &bufSize,
		                          subsampling,
		                          quality,
		                          TJFLAG_NOREALLOC);

		if (result != 0) {
			throw CreateTjError("Unable to compress JPEG");
		}

		// Trim the output buffer to the actual compressed size
		resultData.resize(bufSize);

		return resultData;

	}

}
