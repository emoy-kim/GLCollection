#include "video_reader.h"

VideoReader::VideoReader()
    : StartFrame( 0 ),
      VideoTrackID( -1 ),
      TotalFrameNumber( 0 ),
      FrameWidth( 0 ),
      FrameHeight( 0 ),
      Framerate( 0.0 ) {}

void VideoReader::close()
{
    if (VideoInfo.FormatContext != nullptr) avformat_close_input( &VideoInfo.FormatContext );
    if (VideoInfo.Decoder != nullptr) VideoInfo.Decoder->close();

    VideoTrackID = -1;
    TotalFrameNumber = 0;
    FrameWidth = 0;
    FrameHeight = 0;
    Framerate = 0.0;
    StartFrame = 0;
}

bool VideoReader::open(const std::string& video_file_path)
{
    avformat_open_input( &VideoInfo.FormatContext, video_file_path.c_str(), nullptr, nullptr );
    if (VideoInfo.FormatContext == nullptr || avformat_find_stream_info( VideoInfo.FormatContext, nullptr ) < 0) {
        close();
        throw std::runtime_error( video_file_path );
    }

    AVFormatContext* format_context = VideoInfo.FormatContext;
    VideoTrackID = av_find_best_stream(
        format_context, AVMEDIA_TYPE_VIDEO,
        -1, -1, nullptr, 0
    );
    if (VideoTrackID == AVERROR_STREAM_NOT_FOUND) {
        close();
        const std::string message = "Could not find a video stream from " + std::string( format_context->url );
        throw std::runtime_error( message.c_str() );
    }
    const AVStream* video_stream = format_context->streams[VideoTrackID];
    FrameWidth = video_stream->codecpar->width;
    FrameHeight = video_stream->codecpar->height;
    Framerate = av_q2d( video_stream->avg_frame_rate );
    TotalFrameNumber = static_cast<int>(video_stream->nb_frames);

    VideoInfo.Decoder = std::make_shared<FileDecoder>();
    VideoInfo.Decoder->openVideo( video_stream );
    StartFrame = av_index_search_timestamp(
        format_context->streams[VideoTrackID],
        video_stream->start_time,
        AVSEEK_FLAG_FRAME | AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD
    );
    VideoInfo.NextKeyFrameIndex = searchNextKeyFrameIndex( VideoInfo, 0 );
    return true;
}

bool VideoReader::read(uint8_t* image_buffer, int frame_index_to_decode)
{
    if (VideoInfo.DecodedFrameIndex != frame_index_to_decode && frame_index_to_decode < TotalFrameNumber) {
        nextUntil( VideoInfo, frame_index_to_decode );
        VideoInfo.Decoder->getFrame( image_buffer );
        return true;
    }
    return false;
}

void VideoReader::nextUntil(Video& video, int frame_index_to_decode) const
{
    if (needToSeek( video, frame_index_to_decode )) seek( video, frame_index_to_decode );

    auto* decoder = video.Decoder.get();
    do {
        if (!decoder->decodeVideo( video.FormatContext, VideoTrackID ))
            throw std::runtime_error( "Decoding is not properly processed" );
        video.DecodedFrameIndex = convertTimestampToFrameIndex( video, decoder->getTimestamp() );
    } while (canSkipUntil( video, frame_index_to_decode ));
    video.DecodedFrameIndex = convertTimestampToFrameIndex( video, decoder->getTimestamp() );
}

bool VideoReader::needToSeek(Video& video, int frame_index_to_decode) const
{
    if (video.DecodedFrameIndex < frame_index_to_decode && frame_index_to_decode < video.NextKeyFrameIndex) {
        return false;
    }
    if (frame_index_to_decode == video.NextKeyFrameIndex) {
        video.NextKeyFrameIndex = searchNextKeyFrameIndex( video, frame_index_to_decode );
        return false;
    }
    return true;
}

void VideoReader::seek(Video& video, int frame_index_to_decode) const
{
    const int64_t timestamp = convertFrameIndexToTimestamp( video, frame_index_to_decode );
    av_seek_frame(
        video.FormatContext,
        VideoTrackID,
        timestamp,
        AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD
    );
    video.Decoder->reset();
    video.NextKeyFrameIndex = searchNextKeyFrameIndex( video, frame_index_to_decode );
    video.DecodedFrameIndex = -1;
}

int VideoReader::convertTimestampToFrameIndex(const Video& video, int64_t timestamp) const
{
    const int frame_index_to_decode =
        av_index_search_timestamp(
            video.FormatContext->streams[VideoTrackID],
            timestamp,
            AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD
        ) - StartFrame;
    return std::clamp( frame_index_to_decode, 0, TotalFrameNumber - 1 );
}

int64_t VideoReader::convertFrameIndexToTimestamp(const Video& video, int frame_index_to_decode) const
{
    frame_index_to_decode = std::clamp( frame_index_to_decode + StartFrame, 0, TotalFrameNumber - 1 );
    return video.FormatContext->streams[VideoTrackID]->index_entries[frame_index_to_decode].timestamp;
}

int VideoReader::searchNextKeyFrameIndex(const Video& video, int frame_index_to_decode) const
{
    frame_index_to_decode = std::clamp( ++frame_index_to_decode, 0, TotalFrameNumber );
    const AVIndexEntry* index_entry = video.FormatContext->streams[VideoTrackID]->index_entries;
    while (frame_index_to_decode < TotalFrameNumber) {
        if (index_entry[frame_index_to_decode].flags == AVINDEX_KEYFRAME) return frame_index_to_decode;
        ++frame_index_to_decode;
    }
    return TotalFrameNumber;
}