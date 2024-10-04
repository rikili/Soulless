#pragma once

#include "core/common.hpp"
#include "render_system.hpp"

/**
 * Container for all our entities and game logic. Individual rendering / update is
 * deferred to the relative update() methods
 */
class WorldSystem
{
public:
	WorldSystem();
	~WorldSystem();
	bool step(float elapsed_ms);
	bool is_over()const;
	void set_renderer(RenderSystem* renderer);
	void initialize();

private:
	GLFWwindow* window{};
	RenderSystem* renderer;
};
