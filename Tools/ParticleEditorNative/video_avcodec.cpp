#include <codecvt>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <string>
#include <infrastructure/format.h>
#include <infrastructure/stringutil.h>

#include "video.h"

class AvcodecVideoEncoder : public VideoEncoder {
public:

	explicit AvcodecVideoEncoder(const std::wstring& outputName);
	~AvcodecVideoEncoder();

	void Init(int width, int height, int fps) override;
	void Finish() override;
	void WriteFrame(uint8_t* data, int stride) override;

private:
	int mWidth = 0;
	int mHeight = 0;
	std::wstring mOutputName;
	AVCodec* mCodec = nullptr;
	AVFormatContext* mContext = nullptr;
	AVFrame* mFrame = nullptr;
	AVOutputFormat* mOutputFormat = nullptr;
	AVStream* mStream = nullptr;
	SwsContext* mConvertCtx = nullptr;
	int mFrameNo = 0;

	std::string AvErrorStr(int errorCode) const;
};

std::unique_ptr<VideoEncoder> VideoEncoder::Create(const std::wstring& filename) {
	return std::make_unique<AvcodecVideoEncoder>(filename);
}

AvcodecVideoEncoder::AvcodecVideoEncoder(const std::wstring& outputName) : mOutputName(outputName) {
	av_register_all();
}

AvcodecVideoEncoder::~AvcodecVideoEncoder() {

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

void AvcodecVideoEncoder::Init(int width, int height, int fps) {

	mWidth = width;
	mHeight = height;

	// Setup the desired output format
	mOutputFormat = av_guess_format("mp4", nullptr, nullptr);

	mContext = avformat_alloc_context();
	mContext->oformat = mOutputFormat;

	//setup converter	
	strcpy_s(mContext->filename, sizeof(mContext->filename), ucs2_to_local(mOutputName).c_str());

	mCodec = avcodec_find_encoder(mOutputFormat->video_codec);
	if (!mCodec) {
		throw std::exception("Unable to initialize VP9 codec.");
	}

	mStream = avformat_new_stream(mContext, mCodec);
	if (!mStream) {
		throw std::exception("Unable to initialize stream");
	}

	mStream->time_base = AVRational{1, fps};
	auto c = mStream->codec;

	c->codec_id = mOutputFormat->video_codec;
	c->codec_type = AVMEDIA_TYPE_VIDEO;
	c->bit_rate = 8000000;
	c->width = width;
	c->height = height;
	c->time_base = {1, fps};
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
	av_opt_set(c->priv_data, "qmax", "0", 0);
	av_opt_set(c->priv_data, "qdiff", "4", 0);
	av_opt_set(c->priv_data, "directpred", "1", 0);
	av_opt_set(c->priv_data, "flags2", "+fastpskip", 0);
	av_opt_set(c->priv_data, "cqp", "0", 0);
	av_opt_set(c->priv_data, "wpredp", "2", 0);

	if (mOutputFormat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER ;

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

	ret = avio_open(&mContext->pb, ucs2_to_local(mOutputName).c_str(), AVIO_FLAG_WRITE);
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

void AvcodecVideoEncoder::Finish() {
	av_write_trailer(mContext);
}

void AvcodecVideoEncoder::WriteFrame(uint8_t* data, int stride) {

	// Convert ARGBA to YUV
	sws_scale(mConvertCtx,
	          &data, &stride,
	          0, mHeight,
	          mFrame->data, mFrame->linesize);

	AVPacket pkt = {0};
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

std::string AvcodecVideoEncoder::AvErrorStr(int errorCode) const {
	char errorBuf[AV_LOG_MAX_OFFSET] = {0,};
	return av_make_error_string(errorBuf, sizeof(errorBuf), errorCode);
}
