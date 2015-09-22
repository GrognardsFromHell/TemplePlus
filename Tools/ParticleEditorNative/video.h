
#pragma once

#include <memory>
#include <stdint.h>

class VideoEncoder {
public:
	virtual ~VideoEncoder() {}

	static std::unique_ptr<VideoEncoder> Create(const std::wstring &filename);

	virtual void Init(int width, int height, int fps) = 0;
	virtual void Finish() = 0;
	virtual void WriteFrame(uint8_t* data, int stride) = 0;
};

