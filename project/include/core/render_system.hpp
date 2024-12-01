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
    }

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

    std::vector<RenderIndex> sorted_indices;
    Entity screen_state_entity;
    GLFWwindow* window = nullptr;
    IAssetManager* asset_manager = nullptr; 
    IInputHandler* input_handler = nullptr;

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
    const std::string MAX_LEVEL_STR = "*";
    const vec3 LEFTMOST_GAUGE_POS = vec3(30, 710, 0);
    const vec3 LEFTMOST_GAUGE_TEXT_POS = vec3(23, window_height_px - LEFTMOST_GAUGE_POS.y + 31.f, 0.f);
    const float NON_PROGRESS_DARKEN_FACTOR = -0.5f;
    const float TEXT_LIGHTEN_FACTOR = 0.2f;
    const vec3 GAUGE_SCALE = vec3(20, 50, 1);
    const float GAUGE_SPACING = 50.f;
    const vec3 GAUGE_TEXTURE_TRANSLATE = vec3(140, 700, 0);
    const vec3 GAUGE_TEXTURE_SCALE = vec3(150, 140, 0);
};