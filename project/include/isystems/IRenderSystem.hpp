// IRenderSystem.hpp
#pragma once
#include "forward_types.hpp"
#include <glm/mat4x4.hpp>
#include <string>

class IRenderSystem
{
public:
    virtual ~IRenderSystem() = default;
    virtual bool initialize(IInputHandler &input_handler,
                            int width = 480,
                            int height = 500,
                            const char *title = "OpenGL Example") = 0;
    virtual void setUpView() const = 0;
    virtual void initializeCamera() = 0;
    virtual void drawFrame(float elapsed_ms) = 0;
    virtual GLFWwindow *getGLWindow() const = 0;
    virtual void drawText(const std::string &text,
                          const std::string &fontName,
                          float x, float y,
                          float scale,
                          const glm::vec3 &color) = 0;
    virtual float getTextWidth(const std::string &text,
                               const std::string &fontName,
                               float scale) = 0;
    virtual void drawParticles() = 0;
    virtual void setAssetManager(IAssetManager *asset_manager) = 0;
    virtual Mesh *getMesh(const AssetId &name) = 0;
    virtual IAssetManager &getAssetManager() = 0;
    virtual glm::mat4 getProjectionMatrix() = 0;
    virtual glm::mat4 getViewMatrix() = 0;
    virtual void updateRenderOrder(ComponentContainer<RenderRequest> &render_requests) = 0;
    virtual void addSubRenderer(const std::string &name, ISubRenderer *sub_renderer) = 0;
    virtual void removeSubRenderer(const std::string &name) = 0;
    virtual std::map<std::string, ISubRenderer *> &getSubRenderersMap() = 0;
};