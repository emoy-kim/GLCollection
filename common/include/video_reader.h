#pragma once

#include "file_decoder.h"

class VideoReader final
{
public:
    VideoReader();
    ~VideoReader() { close(); }

    VideoReader(const VideoReader&) = delete;
    VideoReader(const VideoReader&&) = delete;
    VideoReader& operator=(const VideoReader&) = delete;
    VideoReader& operator=(const VideoReader&&) = delete;

    void close();
    bool open(const std::string& video_file_path);
    bool read(uint8_t* image_buffer, int frame_index_to_decode);
    [[nodiscard]] int getFrameWidth() const { return FrameWidth; }
    [[nodiscard]] int getFrameHeight() const { return FrameHeight; }
    [[nodiscard]] double getFramerate() const { return Framerate; }
    [[nodiscard]] int getFrameBufferSize() const { return FrameWidth * FrameHeight * 4; }

private:
    struct Video
    {
        int DecodedFrameIndex;
        int NextKeyFrameIndex;
        AVFormatContext* FormatContext;
        std::shared_ptr<FileDecoder> Decoder;

        Video()
            : DecodedFrameIndex( -1 ),
              FormatContext( nullptr ),
              Decoder( nullptr ),
              NextKeyFrameIndex( 0 ) {}
    };

    int StartFrame;
    int VideoTrackID;
    int TotalFrameNumber;
    int FrameWidth;
    int FrameHeight;
    double Framerate;
    Video VideoInfo;

    void seek(Video& video, int frame_index_to_decode) const;
    void nextUntil(Video& video, int frame_index_to_decode) const;

    [[nodiscard]] bool canSkipUntil(const Video& video, int frame_index_to_decode) const
    {
        return !video.Decoder->isFileClosed() &&
            video.DecodedFrameIndex < frame_index_to_decode &&
            video.DecodedFrameIndex < TotalFrameNumber;
    }

    [[nodiscard]] bool needToSeek(Video& video, int frame_index_to_decode) const;
    [[nodiscard]] int convertTimestampToFrameIndex(const Video& video, int64_t timestamp) const;
    [[nodiscard]] int64_t convertFrameIndexToTimestamp(const Video& video, int frame_index_to_decode) const;
    [[nodiscard]] int searchNextKeyFrameIndex(const Video& video, int frame_index_to_decode) const;
};