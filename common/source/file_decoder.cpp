#include "file_decoder.h"

FileDecoder::FileDecoder()
    : FrameWidth( 0 ),
      FrameHeight( 0 ),
      FrameIndex( -1 ),
      Timestamp( 0 ),
      Framerate( 0 ),
      VideoCodecID( AV_CODEC_ID_NONE ),
      PixelFormat( AV_PIX_FMT_RGBA ),
      SWSContext( nullptr ),
      VideoCodecContext( nullptr ),
      Packet( nullptr ),
      FileClosed( false ),
      ElapsedTimeInSec( 0.0 ),
      DecodedImageBuffer( nullptr ),
      DecodedFrame( nullptr ),
      RGBAFrame( nullptr ) {}

FileDecoder::~FileDecoder()
{
    if (VideoCodecContext != nullptr) avcodec_free_context( &VideoCodecContext );
    if (Packet != nullptr) av_packet_free( &Packet );
    if (SWSContext != nullptr) {
        sws_freeContext( SWSContext );
        SWSContext = nullptr;
    }
}

void FileDecoder::setCodecContext(AVCodecContext** CodecContext, const AVStream* stream)
{
    const AVCodec* decoder = avcodec_find_decoder( stream->codecpar->codec_id );
    if (decoder == nullptr) return;

    *CodecContext = avcodec_alloc_context3( decoder );
    if (*CodecContext == nullptr) return;
    if (avcodec_parameters_to_context( *CodecContext, stream->codecpar ) < 0) return;
    if (*CodecContext == VideoCodecContext) {
        FrameWidth = VideoCodecContext->width;
        FrameHeight = VideoCodecContext->height;

        RGBAFrame = av_frame_alloc();
        if (RGBAFrame == nullptr)
            throw std::runtime_error( "Could not allocate AVFrame" );

        DecodedFrame->width = RGBAFrame->width = FrameWidth;
        DecodedFrame->height = RGBAFrame->height = FrameHeight;
        DecodedFrame->format = VideoCodecContext->pix_fmt;
        RGBAFrame->format = static_cast<int>(PixelFormat);
    }
    avcodec_open2( *CodecContext, decoder, nullptr );
}

void FileDecoder::openVideo(const AVStream* video_stream)
{
    DecodedFrame = av_frame_alloc();
    if (DecodedFrame == nullptr)
        throw std::runtime_error( "Could not allocate AVFrame" );
    if (video_stream != nullptr) {
        setCodecContext( &VideoCodecContext, video_stream );
        SWSContext = sws_getContext(
            FrameWidth, FrameHeight, VideoCodecContext->pix_fmt,
            FrameWidth, FrameHeight, PixelFormat,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
        );
        Framerate = av_q2d( video_stream->avg_frame_rate );
    }
    Packet = av_packet_alloc();
}

void FileDecoder::close()
{
    if (DecodedFrame != nullptr) av_frame_free( &DecodedFrame );
    if (RGBAFrame != nullptr) av_frame_free( &RGBAFrame );
    if (DecodedImageBuffer != nullptr) {
        av_free( DecodedImageBuffer );
        DecodedImageBuffer = nullptr;
    }
    FileClosed = false;
    ElapsedTimeInSec = 0.0;
}

void FileDecoder::getRGBAImage(uint8_t* image_buffer)
{
    AVFrame* frame = DecodedFrame;
    const int buffer_size = av_image_get_buffer_size( PixelFormat, FrameWidth, FrameHeight, 1 );
    if (VideoCodecContext->pix_fmt != PixelFormat) {
        if (DecodedImageBuffer == nullptr) {
            DecodedImageBuffer = static_cast<uint8_t*>(av_malloc( buffer_size * sizeof( uint8_t ) ));
        }

        av_image_fill_arrays(
            RGBAFrame->data, RGBAFrame->linesize,
            DecodedImageBuffer,
            PixelFormat, FrameWidth, FrameHeight, 1
        );

        sws_scale(
            SWSContext,
            frame->data, frame->linesize, 0, FrameHeight,
            RGBAFrame->data, RGBAFrame->linesize
        );
        frame = RGBAFrame;
    }

    av_image_copy_to_buffer(
        image_buffer, buffer_size,
        frame->data, frame->linesize,
        PixelFormat, frame->width, frame->height, 1
    );
}

int FileDecoder::readFrame(int video_track_id)
{
    AVCodecContext* context;
    if (Packet->stream_index == video_track_id) context = VideoCodecContext;
    else return KEEP_GOING;

    if (avcodec_send_packet( context, Packet ) < 0)
        throw std::runtime_error( "Could not send AVPacket" );

    while (true) {
        AVFrame* frame = DecodedFrame;
        const int received = avcodec_receive_frame( context, frame );
        if (received == AVERROR_EOF) {
            av_packet_unref( Packet );
            break;
        }
        if (received < 0) break;

        Timestamp = frame->best_effort_timestamp;
        return VIDEO_DECODED;
    }
    return KEEP_GOING;
}

void FileDecoder::decode(AVFormatContext* format_context, int video_track_id)
{
    if (FileClosed) return;

    while (true) {
        FileClosed = av_read_frame( format_context, Packet ) < 0;
        if (FileClosed || readFrame( video_track_id ) == VIDEO_DECODED) break;
        av_packet_unref( Packet );
    }
    if (FileClosed) {
        Packet->data = nullptr;
        Packet->size = 0;
        readFrame( video_track_id );
    }
}

void FileDecoder::copyFrame(AVFrame** dst, const AVFrame* src)
{
    *dst = av_frame_alloc();
    (*dst)->width = src->width;
    (*dst)->height = src->height;
    (*dst)->channels = src->channels;
    (*dst)->nb_samples = src->nb_samples;
    (*dst)->format = src->format;
    (*dst)->channel_layout = src->channel_layout;
    av_frame_get_buffer( *dst, 0 );
    av_frame_copy( *dst, src );
    av_frame_copy_props( *dst, src );
}