#pragma once

#include <array>
#include <utility>
#include "common.hpp"
#include "graphics/asset_manager.hpp"
#include "input/input_handler.hpp"

/**
 * Struct to hold the render request data for the render system
 * Does not need to be 1-to-1 with the entity struct
 * It tells the render system what to render and with what asset
 */
struct RenderRequest {
	Entity entity;
	AssetId asset_id;
};

/**
 * System responsible for setting up OpenGL and for rendering all the visual entities in the game
 */
class RenderSystem {

public:
	bool initialize(int width = 480, int height = 500, const char* title = "OpenGL Example");
	void setUpView() const;
	GLFWwindow* getGLWindow() const;
	void drawFrame();
	void setAssetManager(AssetManager* asset_manager) { this->asset_manager = *asset_manager; }
	void addRenderRequest(Entity entity, AssetId asset_id) { render_requests.push_back({entity, std::move(asset_id)}); }

private:
	InputHandler inputHandler;
	GLuint frame_buffer = 0;
	Entity screen_state_entity;
	GLFWwindow* window = nullptr;
	AssetManager asset_manager; // Holds all the assets
	std::vector<RenderRequest> render_requests; // Holds all the render requests
};
