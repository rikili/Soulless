// RenderSystem.hpp
#pragma once
#include "isystems/IRenderSystem.hpp"
#include "isystems/IInputHandler.hpp"
#include "isystems/IAssetManager.hpp"
#include "isystems/ISubRenderer.hpp"
#include "common.hpp"
#include <array>
#include <vector>
#include <map>

#include "graphics/video_player.hpp"

extern "C" {
#include <libavcodec/version.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
}

class RenderSystem final : public IRenderSystem {
public:
    bool initialize(IInputHandler& input_handler,
        int width = 480,
        int height = 500,
        const char* title = "OpenGL Example") override;
    void setUpView() const override;
    void initializeCamera() override;
    GLFWwindow* getGLWindow() const override;
    void drawFrame(float elapsed_ms) override;

    void drawText(const std::string& text,
        const std::string& fontName,
        float x, float y,
        float scale,
        const glm::vec3& color,
        bool centered = true) override;

    float getTextWidth(const std::string& text,
        const std::string& fontName,
        float scale) override;

    void drawParticles() override;

    void setAssetManager(IAssetManager* asset_manager) override {
        this->asset_manager = asset_manager;
    }

    Mesh* getMesh(const AssetId& name) override {
        return asset_manager->getMesh(name);
    }

    IAssetManager& getAssetManager() override {
        return *asset_manager;
    }

    glm::mat4 getProjectionMatrix() override {
        return projectionMatrix;
    }

    glm::mat4 getViewMatrix() override {
        return viewMatrix;
    }

    void updateRenderOrder(ComponentContainer<RenderRequest>& render_requests) override;

    void addSubRenderer(const std::string& name, ISubRenderer* sub_renderer) override {
        sub_renderers[name] = sub_renderer;
    }

    void removeSubRenderer(const std::string& name) override {
        sub_renderers.erase(name);
    }

    std::map<std::string, ISubRenderer*>& getSubRenderersMap() override {
        return sub_renderers;
    }

    ~RenderSystem() override {
        for (auto& pair : sub_renderers) {
            delete pair.second;
        }
        sub_renderers.clear();

        if (cursor) {
            glfwDestroyCursor(cursor);
            cursor = nullptr;
        }
    }

    bool playVideo(const std::string& filename) override;
    void stopVideo() override;
    void updateVideo() override;
    bool isPlayingVideo() const override;

    void playCutscene(const std::string& filename, Song song) override;

private:
    struct RenderIndex {
        size_t index;
        float render_y;
        RenderIndex(size_t i, float y) : index(i), render_y(y) {}
    };

    void updateCameraPosition(float x, float y);
    void drawBackgroundObjects();
    void drawHealthBars();
    void drawHUDElement(std::string textureId, vec2 translation, vec2 scale);
    void drawHUD();
    void drawCooldown(const Player& player);
    void drawCooldownElement(vec2 translation, vec2 scale);
    void drawSpellProgress(Player& player);
    void drawProgressBar(const mat4 transform_mat, float progress, bool is_vertical, const vec3& progress_color, const vec3& non_progress_color);
    void drawTimer();
    void drawInteractions();

    std::vector<RenderIndex> sorted_indices;
    Entity screen_state_entity;
    GLFWwindow* window = nullptr;
    IAssetManager* asset_manager = nullptr;
    IInputHandler* input_handler = nullptr;

    std::unique_ptr<VideoPlayer> video_player;
    bool is_playing_video;

    std::map<std::string, ISubRenderer*> sub_renderers;
    std::vector<RenderRequest> render_requests;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    Entity camera;

    // Spell queue rendering constants
    const vec2 QUEUE_TRANSLATE = vec2(window_width_px / 2.f, 720);
    const vec2 QUEUE_SCALE = vec2(260, 260);
    const vec2 TRANSLATE_QUEUE_SPELLS = vec2(440, 720);
    const vec2 SCALE_QUEUE_SPELLS = vec2(30, 30);
    const float QUEUE_SPACING = 65.f;
    const vec2 LEFT_SLOT_TRANSLATE = vec2(408, 621);
    const vec2 RIGHT_SLOT_TRANSLATE = vec2(798, 621);
    const vec2 LEFT_COOLDOWN_TRANSLATE = LEFT_SLOT_TRANSLATE;
    const vec2 RIGHT_COOLDOWN_TRANSLATE = RIGHT_SLOT_TRANSLATE;
    const vec2 COOLDOWN_SCALE = SCALE_QUEUE_SPELLS;

    // Progress Gauge rendering constants
    const std::string MAX_LEVEL_STR = "M";
    const vec3 LEFTMOST_GAUGE_POS = vec3(30, 710, 0);
    const vec3 LEFTMOST_GAUGE_TEXT_POS = vec3(30, window_height_px - LEFTMOST_GAUGE_POS.y + 31.f, 0.f);
    const float NON_PROGRESS_DARKEN_FACTOR = -0.5f;
    const float TEXT_DARKEN_FACTOR = -0.7f;
    const float GAUGE_TEXT_SCALE = 0.3f;
    const vec3 GAUGE_SCALE = vec3(20, 50, 1);
    const float GAUGE_SPACING = 50.f;
    const vec3 GAUGE_TEXTURE_TRANSLATE = vec3(140, 700, 0);
    const vec3 GAUGE_TEXTURE_SCALE = vec3(150, 140, 0);

    // Timer rendering constants
    const vec2 TIMER_POS = vec2(window_width_px / 2.f, 66);
    const vec2 TIMER_SCALE = vec2(300, 240);
    const vec3 TIMER_BAR_TRANSLATE = vec3(window_width_px / 2.f, 60, 0);
    const vec3 TIMER_BAR_SCALE = vec3(207, 15, 1);
    const vec3 TIMER_TEXT_TRANSLATE = vec3(window_width_px / 2.f, window_height_px + 27, 0);
    const float TIMER_TEXT_SCALE = 0.3f;
    const vec3 TIMER_TEXT_COLOR = vec3(.8f, .1f, .1f);
    const vec3 TIMER_BAR_COLOR_PROGRESS = vec3(.8f, .1f, .1f);
    const vec3 TIMER_BAR_COLOR_NON_PROGRESS = vec3(.5f, 0, 0);

    const vec3 ALTAR_TEXT_POSITON = vec3(window_width_px / 2.f, window_height_px / 2.f - 50.f, 0.f);
    const float ALTAR_TEXT_SCALE = 1.f;
    const vec3 ALTAR_TEXT_COLOR = vec3({ 1.f, .3f, .3f });

    const float PLASMA_TEXT_SCALE = 1.f;
    const vec3 PLASMA_TEXT_COLOR = vec3({ 0.949f, 0.082f, 0.957f });

    
    void setCustomCursor();
    GLFWcursor* cursor;

    SoundManager* sound_manager;
};