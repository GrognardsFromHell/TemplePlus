#pragma once

#include <vector>

#include <gsl/gsl.h>
#include <atlcomcli.h>

using gsl::array_view;

struct IDirect3DDevice9;
struct IDirect3DSurface9;

namespace gfx {

	/*
		Defines the file formats supported by the imaging functions.
	*/
	enum class ImageFileFormat {
		BMP,
		JPEG,
		TGA,
		FNTART,
		Unknown
	};

	struct ImageFileInfo {
		ImageFileFormat format = ImageFileFormat::Unknown;
		int width = 0;
		int height = 0;
		bool hasAlpha = false;
	};

	/*
		Tries to detect the image format of the given data by
		inspecting the header only.
	*/
	ImageFileInfo DetectImageFormat(array_view<uint8_t> data);

	bool DetectTga(array_view<uint8_t> data, ImageFileInfo &info);

	std::unique_ptr<uint8_t[]> DecodeTga(array_view<uint8_t> data);

	/*
		Specifies the pixel format of the uncompressed data
		when encoding or decoding a JPEG image.
	*/
	enum class JpegPixelFormat {
		RGB,
		BGR,
		RGBX,
		BGRX,
		XBGR,
		XRGB
	};

	/*
		Encodes a JPEG image in memory and returns the compressed data.
		pitch is the size in pixels of a line of the uncompressed data.
		quality is an integer between 1 and 100.
	*/
	std::vector<uint8_t> EncodeJpeg(const uint8_t* imageData,
	                                JpegPixelFormat imageDataFormat,
	                                int width,
	                                int height,
	                                int quality,
	                                int pitch);

	struct DecodedImage {
		std::unique_ptr<uint8_t[]> data;
		ImageFileInfo info;
	};

	DecodedImage DecodeFontArt(const array_view<uint8_t> data);

	DecodedImage DecodeImage(const array_view<uint8_t> data);

	CComPtr<IDirect3DSurface9> LoadImageToSurface(IDirect3DDevice9* device,
	                                              const array_view<uint8_t> data,
	                                              ImageFileInfo& info);

}
