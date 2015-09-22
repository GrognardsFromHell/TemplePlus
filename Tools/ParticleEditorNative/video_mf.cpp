#include <string>

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <Codecapi.h>
#include <Strmif.h>

#include <exception.h>
#include <atlcomcli.h>

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

#include "video.h"

// Format constants
const UINT32 VIDEO_BIT_RATE = 8000000;

class MfVideoEncoder : public VideoEncoder {
public:
	explicit MfVideoEncoder(const std::wstring& filename);
	~MfVideoEncoder();

	void Init(int width, int height, int fps) override;
	void Finish() override;
	void WriteFrame(uint8_t* data, int stride) override;
private:
	int mWidth = 0;
	int mHeight = 0;
	int mFps = 0;
	uint64_t mCurrentTime = 0;
	uint64_t mFrameTime = 0;
	uint32_t mStreamIndex = 0;
	std::wstring mFilename;
	CComPtr<IMFSinkWriter> mSinkWriter;
};

MfVideoEncoder::MfVideoEncoder(const std::wstring& filename) : mFilename(filename) {

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	auto hr = MFStartup(MF_VERSION);
	if (!SUCCEEDED(hr)) {
		throw TempleException("Unable to initialize Media Foundation: {}", hr);
	}

}

MfVideoEncoder::~MfVideoEncoder() {

	MFShutdown();

	CoUninitialize();

}

void MfVideoEncoder::Init(int width, int height, int fps) {
	mWidth = width;
	mHeight = height;
	mFps = fps;
	mFrameTime = 10 * 1000 * 1000 / fps;

	CComPtr<IMFMediaType> pMediaTypeOut;
	CComPtr<IMFMediaType> pMediaTypeIn;

	HRESULT hr = MFCreateSinkWriterFromURL(mFilename.c_str(), NULL, NULL, &mSinkWriter);

	// Set the output media type.
	if (SUCCEEDED(hr)) {
		hr = MFCreateMediaType(&pMediaTypeOut);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, 8000000);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	if (SUCCEEDED(hr)) {
		hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, width, height);
	}
	if (SUCCEEDED(hr)) {
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, fps, 1);
	}
	if (SUCCEEDED(hr)) {
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	pMediaTypeOut->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, 1);

	if (SUCCEEDED(hr)) {
		hr = mSinkWriter->AddStream(pMediaTypeOut, (DWORD*)&mStreamIndex);
	}

	// Set the input media type.
	if (SUCCEEDED(hr)) {
		hr = MFCreateMediaType(&pMediaTypeIn);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeIn->SetUINT32(MF_MT_MPEG2_PROFILE, 77);
	}
	if (SUCCEEDED(hr)) {
		hr = pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	if (SUCCEEDED(hr)) {
		hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, width, height);
	}
	if (SUCCEEDED(hr)) {
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, fps, 1);
	}
	if (SUCCEEDED(hr)) {
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	if (SUCCEEDED(hr)) {
		hr = mSinkWriter->SetInputMediaType(mStreamIndex, pMediaTypeIn, nullptr);
	}
	
	// Tell the sink writer to start accepting data.
	if (SUCCEEDED(hr)) {
		hr = mSinkWriter->BeginWriting();
	}
	
	CComPtr<ICodecAPI> encoder;
	hr = mSinkWriter->GetServiceForStream(0, GUID_NULL, IID_PPV_ARGS(&encoder));
	if (SUCCEEDED(hr)) {
		CComVariant quality((UINT32)eAVEncCommonRateControlMode_CBR, VT_UI4);
		hr = encoder->SetValue(&CODECAPI_AVEncCommonRateControlMode, &quality);
	}	
	if (SUCCEEDED(hr)) {
		CComVariant quality((UINT32)80000000, VT_UI4);
		hr = encoder->SetValue(&CODECAPI_AVEncCommonMeanBitRate, &quality);
	}
	
	// Return the pointer to the caller.
	if (!SUCCEEDED(hr)) {
		mSinkWriter.Release();
		throw TempleException("Unable to begin writing to the video stream");
	}
}

void MfVideoEncoder::WriteFrame(uint8_t* data, int stride) {
	CComPtr<IMFSample> pSample;
	CComPtr<IMFMediaBuffer> pBuffer;

	const LONG cbWidth = 4 * mWidth;
	const DWORD cbBuffer = cbWidth * mHeight;

	BYTE *pData = NULL;

	// Create a new memory buffer.
	HRESULT hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer);

	// Lock the buffer and copy the video frame to the buffer.
	if (SUCCEEDED(hr))
	{
		hr = pBuffer->Lock(&pData, NULL, NULL);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFCopyImage(
			pData,                      // Destination buffer.
			cbWidth,                    // Destination stride.
			(BYTE*)data,    // First row in source image.
			stride,                    // Source stride.
			cbWidth,                    // Image width in bytes.
			mHeight                // Image height in pixels.
			);
	}
	if (pBuffer)
	{
		pBuffer->Unlock();
	}

	// Set the data length of the buffer.
	if (SUCCEEDED(hr))
	{
		hr = pBuffer->SetCurrentLength(cbBuffer);
	}

	// Create a media sample and add the buffer to the sample.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateSample(&pSample);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSample->AddBuffer(pBuffer);
	}

	// Set the time stamp and the duration.
	if (SUCCEEDED(hr))
	{
		hr = pSample->SetSampleTime(mCurrentTime);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSample->SetSampleDuration(mFrameTime);
	}

	// Send the sample to the Sink Writer.
	if (SUCCEEDED(hr))
	{
		hr = mSinkWriter->WriteSample(mStreamIndex, pSample);
	}

	if (!SUCCEEDED(hr)) {
		throw TempleException("Unable to write video frame: {}", hr);
	}

	mCurrentTime += mFrameTime;

}

void MfVideoEncoder::Finish() {
	if (mSinkWriter) {
		auto hr = mSinkWriter->Finalize();
		if (!SUCCEEDED(hr)) {
			throw TempleException("Unable to finalize video encoding: {}", hr);
		}
		mSinkWriter.Release();
	}
}

std::unique_ptr<VideoEncoder> VideoEncoder::Create(const std::wstring& filename) {
	return std::make_unique<MfVideoEncoder>(filename);
}
