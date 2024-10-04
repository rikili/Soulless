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
		motion.position += motion.velocity * elapsed_ms_since_last_update;
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
	motion.velocity = { 0.1f, 0.1f };
	motion.scale = { 0.5f, 0.5f };
	this->renderer->addRenderRequest(player, "basic");
}


// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
}
