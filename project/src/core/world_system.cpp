#include "core/world_system.hpp"

#include "entities/ecs_registry.hpp"

WorldSystem::WorldSystem() {}
WorldSystem::~WorldSystem() {}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	ComponentContainer<Motion> &motions_registry = registry.motions;
	// Update all motions
	for (Entity entity : motions_registry.entities) {
		Motion& motion = motions_registry.get(entity);

		float seconds = elapsed_ms_since_last_update / 1000.0f;
		motion.position += motion.velocity * seconds;
	}

	return true;
}

void WorldSystem::set_renderer(RenderSystem* renderer)
{
	this->renderer = renderer;
}


/**
 * Initialize the game world
 * TODO: Add your game initialization code here!
 */
void WorldSystem::initialize()
{
	Entity player;
	registry.players.emplace(player);
	Motion& motion = registry.motions.emplace(player);
	motion.position = { 0.0f, 0.0f };  // Center of the screen
	motion.velocity = { 0.25f, 0.25f };
	motion.scale = { 0.5f, 0.5f };
	this->renderer->addRenderRequest(player, "basic", "", "basic");
}
