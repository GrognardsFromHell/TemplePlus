#include <atlcomcli.h>
#include <particles/instances.h>
#include <particles/render.h>

#include <d3dx9tex.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

using namespace particles;

#include "api.h"

class VideoEncoder {
public:

	explicit VideoEncoder(const std::string& outputName);
	~VideoEncoder();

	void Init(int width, int height, int fps);
	void Finish();
	void WriteFrame(uint8_t* data, int stride);

private:
	int mWidth = 0;
	int mHeight = 0;
	std::string mOutputName;
	AVCodec* mCodec = nullptr;
	AVFormatContext* mContext = nullptr;
	AVFrame* mFrame = nullptr;
	AVOutputFormat* mOutputFormat = nullptr;
	AVStream* mStream = nullptr;
	SwsContext* mConvertCtx = nullptr;
	int mFrameNo = 0;

	std::string AvErrorStr(int errorCode) const;
};

VideoEncoder::VideoEncoder(const std::string& outputName) : mOutputName(outputName) {
	av_register_all();
}

VideoEncoder::~VideoEncoder() {

	if (mConvertCtx) {
		sws_freeContext(mConvertCtx);
	}

	if (mFrame) {
		av_frame_free(&mFrame);
	}
	
	if (mStream) {
		avcodec_close(mStream->codec);
	}

	if (mContext) {
		if (mContext->pb) {
			avio_closep(&mContext->pb);
		}

		avformat_free_context(mContext);
	}

}

void VideoEncoder::Init(int width, int height, int fps) {

	mWidth = width;
	mHeight = height;

	// Setup the desired output format
	mOutputFormat = av_guess_format("mp4", nullptr, nullptr);

	mContext = avformat_alloc_context();
	mContext->oformat = mOutputFormat;
	strcpy_s(mContext->filename, sizeof(mContext->filename), mOutputName.c_str());

	mCodec = avcodec_find_encoder(mOutputFormat->video_codec);
	if (!mCodec) {
		throw std::exception("Unable to initialize VP9 codec.");
	}

	mStream = avformat_new_stream(mContext, mCodec);
	if (!mStream) {
		throw std::exception("Unable to initialize stream");
	}
	
	mStream->time_base = AVRational{ 1, fps };
	auto c = mStream->codec;
	
	c->codec_id = mOutputFormat->video_codec;
	c->codec_type = AVMEDIA_TYPE_VIDEO;
	c->bit_rate = 10000000;
	c->width = width;
	c->height = height;
	c->time_base = { 1, fps };
	c->pix_fmt = AV_PIX_FMT_YUV420P;
	c->gop_size = 10; /* emit one intra frame every ten frames */
	c->max_b_frames = 1;
	c->profile = FF_PROFILE_H264_HIGH;
	av_opt_set(c->priv_data, "preset", "slow", 0);
	av_opt_set(c->priv_data, "coder", "1", 0);
	av_opt_set(c->priv_data, "flags", "+loop", 0);
	av_opt_set(c->priv_data, "cmp", "+chroma", 0);
	av_opt_set(c->priv_data, "partitions", "-parti8x8+parti4x4+partp8x8+partp4x4-partb8x8", 0);
	av_opt_set(c->priv_data, "me_method", "hex", 0);
	av_opt_set(c->priv_data, "subq", "5", 0);
	av_opt_set(c->priv_data, "me_range", "16", 0);
	av_opt_set(c->priv_data, "g", "250", 0);
	av_opt_set(c->priv_data, "keyint_min", "25", 0);
	av_opt_set(c->priv_data, "sc_threshold", "40", 0);
	av_opt_set(c->priv_data, "i_qfactor", "0.71", 0);
	av_opt_set(c->priv_data, "b_strategy", "1", 0);
	av_opt_set(c->priv_data, "qcomp", "0.61", 0);
	av_opt_set(c->priv_data, "qmin", "0", 0);
	av_opt_set(c->priv_data, "qmax", "69", 0);
	av_opt_set(c->priv_data, "qdiff", "4", 0);
	av_opt_set(c->priv_data, "directpred", "1", 0);
	av_opt_set(c->priv_data, "flags2", "+fastpskip", 0);
	av_opt_set(c->priv_data, "cqp", "0", 0);
	av_opt_set(c->priv_data, "wpredp", "2", 0);

	if (mOutputFormat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

	// avcodec_opt_set(c->priv_data, "quality", "realtime", AV_OPT_SEARCH_CHILDREN); //can be good, best or realtime
	// av_opt_set(c->priv_data, "passes", "2", AV_OPT_SEARCH_CHILDREN);

	auto ret = avcodec_open2(c, mCodec, nullptr);
	if (ret < 0) {
		auto errorStr = fmt::format("Unable to open codec: {}", AvErrorStr(ret));
		throw std::exception(errorStr.c_str());
	}

	mFrame = av_frame_alloc();
	mFrame->format = c->pix_fmt;
	mFrame->width = width;
	mFrame->height = height;

	ret = av_frame_get_buffer(mFrame, 32);
	if (ret < 0) {
		auto errorStr = fmt::format("Unable to alloc frame buffer: {}", AvErrorStr(ret));
		throw std::exception(errorStr.c_str());
	}

	ret = avio_open(&mContext->pb, mOutputName.c_str(), AVIO_FLAG_WRITE);
	if (ret < 0) {
		auto errorStr = fmt::format("Unable to open output file: {}", AvErrorStr(ret));
		throw std::exception(errorStr.c_str());
	}

	/* Write the stream header, if any. */
	ret = avformat_write_header(mContext, nullptr);
	if (ret < 0) {
		auto errorStr = fmt::format("Unable to write header: {}", AvErrorStr(ret));
		throw std::exception(errorStr.c_str());
	}

	mConvertCtx = sws_getContext(mWidth, mHeight,
	                             AV_PIX_FMT_BGRA,
	                             mWidth, mHeight,
	                             AV_PIX_FMT_YUV420P,
	                             SWS_BICUBIC, NULL, NULL, NULL);

}

void VideoEncoder::Finish() {
	av_write_trailer(mContext);
}

void VideoEncoder::WriteFrame(uint8_t* data, int stride) {
	
	// Convert ARGBA to YUV
	sws_scale(mConvertCtx,
		&data, &stride, 
		0, mHeight, 
		mFrame->data, mFrame->linesize);

	AVPacket pkt = { 0 };
	av_init_packet(&pkt);

	mFrame->pts = mFrameNo++;

	auto c = mStream->codec;
	int gotPacket;
	auto ret = avcodec_encode_video2(c, &pkt, mFrame, &gotPacket);
	if (ret < 0) {
		auto errorStr = fmt::format("Unable to alloc frame buffer: {}", AvErrorStr(ret));
		throw std::exception(errorStr.c_str());
	}

	if (gotPacket) {		
		/* rescale output packet timestamp values from codec to stream timebase */
		av_packet_rescale_ts(&pkt, c->time_base, mStream->time_base);
		pkt.stream_index = mStream->index;
		
		ret = av_interleaved_write_frame(mContext, &pkt);
		if (ret < 0) {
			auto errorStr = fmt::format("Unable to write interleaved frame: {}", AvErrorStr(ret));
			throw std::exception(errorStr.c_str());
		}
	}
}

std::string VideoEncoder::AvErrorStr(int errorCode) const {
	char errorBuf[AV_LOG_MAX_OFFSET] = {0,};
	return av_make_error_string(errorBuf, sizeof(errorBuf), errorCode);
}

static float GetTotalLifetime(const PartSysPtr& sys, bool &permanent) {
	auto result = 0.0f;

	for (const auto& emitter : *sys) {
		auto spec = emitter->GetSpec();
		
		auto lifetime = spec->GetDelay();
		
		if (emitter->GetSpec()->IsPermanent()) {
			auto maxParticlesReachedIn = spec->GetMaxParticles() / (float)spec->GetParticleRate();
			lifetime += maxParticlesReachedIn + spec->GetParticleLifespan();
			permanent = true;
		} else {
			lifetime += spec->GetLifespan() + spec->GetParticleLifespan();
		}
		
		if (lifetime > result) {
			result = lifetime;
		}
	}

	return result;
}

bool ParticleSystem_RenderVideo(IDirect3DDevice9* device, PartSys* orgSys, D3DCOLOR background, const char* outputFile, int fps) {

	// The assumption is that the screen BB of the part sys encompasses the entire system
	// so we use that to render it to a video file
	auto screenBounds = orgSys->GetScreenBounds();
	
	auto w = (int)abs(screenBounds.right - screenBounds.left) + 10;
	auto h = (int)abs(screenBounds.bottom - screenBounds.top) + 10;
	// Needs to be divisible by 2 for h264
	if (w % 2)
		w++;
	if (h % 2)
		h++;

	const auto scale = 1.0f;

	VideoEncoder encoder(outputFile);
	encoder.Init(w, h, fps);

	// Create a clone here to not influence the original system
	auto sys = std::make_shared<PartSys>(orgSys->GetSpec());

	// Save the old render target
	CComPtr<IDirect3DSurface9> orgRenderTarget;
	CComPtr<IDirect3DSurface9> orgDepthSurface;
	device->GetRenderTarget(0, &orgRenderTarget);
	device->GetDepthStencilSurface(&orgDepthSurface);

	// Create a render target
	CComPtr<IDirect3DSurface9> depthSurface;
	CComPtr<IDirect3DSurface9> renderTarget;
	auto result = device->CreateRenderTarget(w, h, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &renderTarget, nullptr);
	if (!SUCCEEDED(result)) {
		return false;
	}

	result = device->CreateDepthStencilSurface(w, h, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &depthSurface, nullptr);
	if (!SUCCEEDED(result)) {
		return false;
	}

	CComPtr<IDirect3DSurface9> sysMemSurface;
	result = device->CreateOffscreenPlainSurface(w, h, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &sysMemSurface, nullptr);
	if (!SUCCEEDED(result)) {
		return false;
	}

	auto timeStepSec = 1.0f / fps;
	auto elapsed = 0.0f;
	bool permanent;
	auto totalTime = GetTotalLifetime(sys, permanent);
	
	if (permanent) {
		sys->Simulate(5.0f);
	}
	

	InitRenderStates(device, (float) w, (float) h, scale);

	particles::ParticleRendererManager renderManager(device);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	device->SetRenderTarget(0, renderTarget);
	device->SetDepthStencilSurface(depthSurface);

	while (elapsed < totalTime) {
		sys->Simulate(timeStepSec);
		elapsed += timeStepSec;

		device->BeginScene();
		result = device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 32, 32, 32), 1.0f, 0);
		if (!SUCCEEDED(result)) {
			return false;
		}

		for (auto& emitter : *sys) {
			auto renderer = renderManager.GetRenderer(emitter->GetSpec()->GetParticleType());
			if (renderer) {
				renderer->Render(emitter.get());
			}
		}

		device->EndScene();
		device->Present(nullptr, nullptr, nullptr, nullptr);

		// Copy from video mem to system mem
		result = device->GetRenderTargetData(renderTarget, sysMemSurface);
		if (!SUCCEEDED(result)) {
			return false;
		}
		
		D3DLOCKED_RECT locked;
		result = sysMemSurface->LockRect(&locked, nullptr, 0);
		if (!SUCCEEDED(result)) {
			return false;
		}

		encoder.WriteFrame((uint8_t*) locked.pBits, locked.Pitch);

		sysMemSurface->UnlockRect();


	}

	encoder.Finish();

	// Restore the org render target
	device->SetRenderTarget(0, orgRenderTarget);
	device->SetDepthStencilSurface(orgDepthSurface);

	return true;

}
