#pragma once

#include "collision_system.hpp"
#include "core/common.hpp"
#include "render_system.hpp"

/**
 * Container for all our entities and game logic. Individual rendering / update is
 * deferred to the relative update() methods
 */
class WorldSystem
{
public:
	explicit WorldSystem(RenderSystem* renderer);
	~WorldSystem();
	bool step(float elapsed_ms);
	bool is_over()const;
	void set_renderer(RenderSystem* renderer);
	void initialize();

private:
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	GLFWwindow* window{};
	RenderSystem* renderer;
	CollisionSystem* collision_system;
};
