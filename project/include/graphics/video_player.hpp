#pragma once

#include "core/common.hpp"
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

class VideoPlayer {
public:
    VideoPlayer() :
        initialized(false), fmt_ctx(nullptr), codec_ctx(nullptr),
        video_stream(nullptr), av_frame(nullptr), gl_frame(nullptr),
        packet(nullptr), sws_ctx(nullptr), video_stream_idx(-1),
        time_base(0), last_frame_time(0), frame_duration(0) {
    }
    ~VideoPlayer();

    bool initialize(const std::string& filename, GLuint shader_program);
    bool readFrame();
    void cleanup();
    void draw(const glm::mat4& projection, const glm::mat4& view);

    int getWidth() const { return codec_ctx ? codec_ctx->width : 0; }
    int getHeight() const { return codec_ctx ? codec_ctx->height : 0; }
    bool isInitialized() const { return initialized; }

private:
    bool initialized;
    AVFormatContext* fmt_ctx;
    AVCodecContext* codec_ctx;
    AVStream* video_stream;
    AVFrame* av_frame;
    AVFrame* gl_frame;
    AVPacket* packet;
    SwsContext* sws_ctx;
    int video_stream_idx;

    double time_base;
    double last_frame_time;
    double frame_duration;

    // OpenGL objects
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint texture;
    GLuint shader_program;

    bool initializeGL();
    bool initializeFFmpeg(const std::string& filename);
};