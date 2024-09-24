#pragma once

#include "base.h"

extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
}

class FileDecoder final
{
public:
    FileDecoder();
    ~FileDecoder();

    FileDecoder(const FileDecoder&) = delete;
    FileDecoder(const FileDecoder&&) = delete;
    FileDecoder& operator=(const FileDecoder&) = delete;
    FileDecoder& operator=(const FileDecoder&&) = delete;

    [[nodiscard]] int getFrameWidth() const { return FrameWidth; }
    [[nodiscard]] int getFrameHeight() const { return FrameHeight; }
    [[nodiscard]] int getFrameIndex() const { return FrameIndex; }
    [[nodiscard]] int64_t getTimestamp() const { return Timestamp; }
    [[nodiscard]] double getFramerate() const { return Framerate; }
    [[nodiscard]] bool isFileClosed() const { return FileClosed; }
    [[nodiscard]] double getElapsedTimeInSec() const { return ElapsedTimeInSec; }
    void openVideo(const AVStream* video_stream);
    void close();

    bool decodeVideo(AVFormatContext* format_context, int video_track_id)
    {
        decode( format_context, video_track_id );
        av_packet_unref( Packet );
        return true;
    }

    void reset() const { avcodec_flush_buffers( VideoCodecContext ); }

    void getFrame(uint8_t* image_buffer)
    {
        if (image_buffer != nullptr && !FileClosed) getRGBAImage( image_buffer );
    }

    void setVideoCodecContextFlag(uint flag) const
    {
        VideoCodecContext->flags = static_cast<int>(static_cast<uint>(VideoCodecContext->flags) | flag);
    }

    void setVideoCodecParameters(AVCodecParameters* parameters) const
    {
        avcodec_parameters_from_context( parameters, VideoCodecContext );
    }

    static void copyFrame(AVFrame** dst, const AVFrame* src);

private:
    enum DECODED_TYPE { KEEP_GOING = 0, AUDIO_DECODED, VIDEO_DECODED };

    int FrameWidth;
    int FrameHeight;
    int FrameIndex;
    int64_t Timestamp;
    double Framerate;
    AVCodecID VideoCodecID;
    AVPixelFormat PixelFormat;
    SwsContext* SWSContext;
    AVCodecContext* VideoCodecContext;
    AVPacket* Packet;
    bool FileClosed;
    double ElapsedTimeInSec;
    uint8_t* DecodedImageBuffer;
    AVFrame* DecodedFrame;
    AVFrame* RGBAFrame;

    void setCodecContext(AVCodecContext** CodecContext, const AVStream* stream);
    void getRGBAImage(uint8_t* image_buffer);
    int readFrame(int video_track_id);
    void decode(AVFormatContext* format_context, int video_track_id);

    static void flip(AVFrame* frame)
    {
        for (int c = 0; c < 4; ++c) {
            frame->data[c] += frame->linesize[c] * (frame->height - 1);
            frame->linesize[c] *= -1;
        }
    }
};