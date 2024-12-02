#include "graphics/video_player.hpp"

#include <iostream>


VideoPlayer::~VideoPlayer() {
    cleanup();
}

bool VideoPlayer::initialize(const std::string& filename, GLuint shader_program) {
    // std::cout << "Initializing with shader program: " << shader_program << std::endl;

    this->shader_program = shader_program;

    // Verify shader uniforms exist
    GLint projLoc = glGetUniformLocation(shader_program, "projection");
    GLint viewLoc = glGetUniformLocation(shader_program, "view");
    GLint texLoc = glGetUniformLocation(shader_program, "videoTexture");

    // std::cout << "Shader uniform locations - projection: " << projLoc
    //           << ", view: " << viewLoc
    //           << ", videoTexture: " << texLoc << std::endl;

    if (!initializeFFmpeg(filename)) {
        return false;
    }

    if (!initializeGL()) {
        cleanup();
        return false;
    }

    initialized = true;
    return true;
}

bool VideoPlayer::initializeFFmpeg(const std::string& filename) {
    const std::string file_path = video_path(filename);
    // std::cout << "Attempting to open video file: " << file_path << std::endl;

    int result = avformat_open_input(&fmt_ctx, file_path.c_str(), nullptr, nullptr);
    if (result < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
        // std::cerr << "Failed to open video file. Error: " << errbuf << std::endl;
        return false;
    }
    // std::cout << "Successfully opened video file" << std::endl;

    result = avformat_find_stream_info(fmt_ctx, nullptr);
    if (result < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
        // std::cerr << "Failed to find stream info. Error: " << errbuf << std::endl;
        return false;
    }
    // std::cout << "Found stream info" << std::endl;

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            video_stream = fmt_ctx->streams[i];
            break;
        }
    }

    if (video_stream_idx == -1) {
        std::cerr << "Could not find video stream in file" << std::endl;
        return false;
    }
    // std::cout << "Found video stream at index " << video_stream_idx << std::endl;

    const AVCodec* codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (!codec) {
        std::cerr << "Failed to find decoder for codec id: " << video_stream->codecpar->codec_id << std::endl;
        return false;
    }
    // std::cout << "Found decoder: " << codec->name << std::endl;

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "Failed to allocate codec context" << std::endl;
        return false;
    }

    // Fill codec context parameters from stream
    result = avcodec_parameters_to_context(codec_ctx, video_stream->codecpar);
    if (result < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
        // std::cerr << "Failed to copy codec params. Error: " << errbuf << std::endl;
        return false;
    }

    // Open codec
    result = avcodec_open2(codec_ctx, codec, nullptr);
    if (result < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
        // std::cerr << "Failed to open codec. Error: " << errbuf << std::endl;
        return false;
    }
    // std::cout << "Successfully opened codec" << std::endl;

    // Allocate frames
    av_frame = av_frame_alloc();
    gl_frame = av_frame_alloc();
    if (!av_frame || !gl_frame) {
        std::cerr << "Failed to allocate frames" << std::endl;
        return false;
    }

    // Allocate buffer for OpenGL frame
    int size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_ctx->width,
                                      codec_ctx->height, 1);
    uint8_t* internal_buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));
    if (!internal_buffer) {
        // std::cerr << "Failed to allocate internal buffer" << std::endl;
        return false;
    }

    result = av_image_fill_arrays(gl_frame->data, gl_frame->linesize, internal_buffer,
                        AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);
    if (result < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
        // std::cerr << "Failed to fill image arrays. Error: " << errbuf << std::endl;
        return false;
    }

    sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                            codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        std::cerr << "Failed to create scaling context" << std::endl;
        return false;
    }

    packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Failed to allocate packet" << std::endl;
        return false;
    }

    // std::cout << "Video initialization complete. Dimensions: " << codec_ctx->width
    //           << "x" << codec_ctx->height << std::endl;

    time_base = av_q2d(video_stream->time_base);
    frame_duration = av_q2d(video_stream->avg_frame_rate);
    frame_duration = 1.0 / frame_duration;
    last_frame_time = glfwGetTime();

    // std::cout << "Video frame rate: " << 1.0/frame_duration << " fps" << std::endl;

    return true;
}

bool VideoPlayer::initializeGL() {
     glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Get actual framebuffer size
    int base_width, base_height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &base_width, &base_height);

    // Double the dimensions for high DPI
    float window_width = base_width * 2.0f;
    float window_height = base_height * 2.0f;

    float video_aspect = static_cast<float>(codec_ctx->width) / static_cast<float>(codec_ctx->height);
    float window_aspect = window_width / window_height;

    float scale_width, scale_height;
    if (video_aspect > window_aspect) {
        scale_width = window_width;
        scale_height = scale_width / video_aspect;
    } else {
        scale_height = window_height;
        scale_width = scale_height * video_aspect;
    }

    float x_offset = (window_width - scale_width) / 2.0f;
    float y_offset = (window_height - scale_height) / 2.0f;

    float vertices[] = {
    // Position (x,y)  Texture coords
        x_offset,            y_offset,            0.0f, 0.0f,  // Bottom left
        x_offset,            y_offset + scale_height, 0.0f, 1.0f,  // Top left
        x_offset + scale_width, y_offset + scale_height, 1.0f, 1.0f,  // Top right
        x_offset + scale_width, y_offset,            1.0f, 0.0f   // Bottom right
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                         (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, codec_ctx->width, codec_ctx->height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    return true;
}


bool VideoPlayer::readFrame() {
    static int frame_count = 0;

    double current_time = glfwGetTime();
    if (current_time - last_frame_time < frame_duration) {
        return true;
    }

    while (av_read_frame(fmt_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_idx) {
            int response = avcodec_send_packet(codec_ctx, packet);
            if (response < 0) {
                av_packet_unref(packet);
                return false;
            }

            response = avcodec_receive_frame(codec_ctx, av_frame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                av_packet_unref(packet);
                continue;
            }
            else if (response < 0) {
                av_packet_unref(packet);
                return false;
            }

            last_frame_time = current_time;
            response = sws_scale(sws_ctx, av_frame->data, av_frame->linesize, 0,
                     codec_ctx->height, gl_frame->data, gl_frame->linesize);

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, codec_ctx->width,
                          codec_ctx->height, GL_RGB, GL_UNSIGNED_BYTE,
                          gl_frame->data[0]);

            frame_count++;
            av_packet_unref(packet);
            return true;
        }
        av_packet_unref(packet);
    }
    return false;
}

void VideoPlayer::draw(const glm::mat4& projection, const glm::mat4& view) {
    if (!initialized) {
        // std::cout << "Video player not initialized" << std::endl;
        return;
    }

    glUseProgram(shader_program);

    // // Debug prints
    // std::cout << "Drawing frame. Texture handle: " << texture << std::endl;
    // std::cout << "Video dimensions: " << codec_ctx->width << "x" << codec_ctx->height << std::endl;

    // Set uniforms
    GLint projLoc = glGetUniformLocation(shader_program, "projection");
    GLint viewLoc = glGetUniformLocation(shader_program, "view");
    if (projLoc == -1 || viewLoc == -1) {
        // std::cout << "Could not find uniforms in shader" << std::endl;
    }
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint texLoc = glGetUniformLocation(shader_program, "videoTexture");
    if (texLoc == -1) {
        // std::cout << "Could not find videoTexture uniform in shader" << std::endl;
    }
    glUniform1i(texLoc, 0);

    // Check OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        // std::cout << "OpenGL error before draw: " << err << std::endl;
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Check for errors again
    while ((err = glGetError()) != GL_NO_ERROR) {
        // std::cout << "OpenGL error after draw: " << err << std::endl;
    }


}

void VideoPlayer::cleanup() {
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    if (av_frame) {
        av_frame_free(&av_frame);
    }
    if (gl_frame) {
        av_frame_free(&gl_frame);
    }
    if (packet) {
        av_packet_free(&packet);
    }
    if (codec_ctx) {
        avcodec_free_context(&codec_ctx);
    }
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &texture);

    initialized = false;
}