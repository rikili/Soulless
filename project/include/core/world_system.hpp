#pragma once

#include "collision_system.hpp"
#include "core/common.hpp"
#include "render_system.hpp"

/**
 * Container for all our entities and game logic.
 */
class WorldSystem
{
public:
	explicit WorldSystem(RenderSystem* renderer);
	~WorldSystem();
	bool step(float elapsed_ms);
	bool is_over()const;
	void set_renderer(RenderSystem* renderer);
	void handle_movements(float elapsed_ms_since_last_update);
	void initialize();

private:
	// helper functions to set the world
	void createPlayer();
	void createEnemy(vec2 position, vec2 velocity);


	GLFWwindow* window{};
	RenderSystem* renderer;
	CollisionSystem* collision_system;
};
