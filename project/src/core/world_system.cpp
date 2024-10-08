#include "core/world_system.hpp"

#include "entities/ecs_registry.hpp"

WorldSystem::WorldSystem(RenderSystem* renderer)
{
	this->renderer = renderer;
	this->collision_system = new CollisionSystem(renderer);
}

WorldSystem::~WorldSystem() {}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	this->handle_timers(elapsed_ms_since_last_update);
	this->handle_movements(elapsed_ms_since_last_update);
	this->collision_system->handle_collisions();

	return true;
}

void WorldSystem::set_renderer(RenderSystem* renderer)
{
	this->renderer = renderer;
}

/**
 * @brief In charge of updating the position of all entities with a motion component
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handle_movements(float elapsed_ms_since_last_update)
{
	ComponentContainer<Motion> &motions_registry = registry.motions;
	// Update all motions
	for (Entity entity : motions_registry.entities) {
		Motion& motion = motions_registry.get(entity);
		motion.position += motion.velocity * elapsed_ms_since_last_update;
	}
}

/**
 * @brief In charge of updating timers and their side effects
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handle_timers(float elapsed_ms_since_last_update)
{
	for (Entity& hit_ent : registry.onHits.entities)
	{
		OnHit& hit = registry.onHits.get(hit_ent);
		hit.invincibility_timer -= elapsed_ms_since_last_update;
		if (hit.invincibility_timer < 0)
		{
			registry.onHits.remove(hit_ent);
		}
	}

	for (Entity& dead_ent : registry.deaths.entities)
	{
		Death& death = registry.deaths.get(dead_ent);
		death.timer -= elapsed_ms_since_last_update;
		if (death.timer < 0)
		{
			// TODO: add custom player death
			if (!registry.players.has(dead_ent))
			{
				registry.remove_all_components_of(dead_ent);
			}
		}
	}

	for (Entity& player_ent : registry.players.entities)
	{
		Player& player = registry.players.get(player_ent);
		player.cooldown -= elapsed_ms_since_last_update;
		if (player.cooldown < 0)
		{
			player.cooldown = 0;
		}
	}
}

/**
 * Initialize the game world
 * TODO: Add your game initialization code here!
 */
void WorldSystem::initialize()
{
	// Create a player
	player_mage = this->createPlayer();

	// Create an enemy
	constexpr int num_enemies = 0;
	std::random_device rd;  // Random device
	std::mt19937 gen(rd()); // Mersenne Twister generator
	std::uniform_real_distribution<float> dis(0.f, 1.0f); // Distribution range [0, 1]

	//for (int i = 0; i < num_enemies; i++)
	//{
	//	float x = dis(gen) * (float)window_height_px;
	//	float y = dis(gen) * (float)window_width_px;
	//	float vx = dis(gen) * (dis(gen) > 0.5f ? 1 : -1) * 50;
	//	float vy = dis(gen) * (dis(gen) > 0.5f ? 1 : -1) * 50;
	//	this->createEnemy({ x, y }, { vx, vy });
	//}
	
	this->createEnemy({ 900, 400 }, { 0, 0 });
}


Entity WorldSystem::createPlayer() {
	auto player = Entity();

	registry.players.emplace(player);
	Motion& motion = registry.motions.emplace(player);
	motion.position = { window_width_px / 2.0f, window_height_px / 2.0f };  // Center of the screen
	motion.velocity = { 0.0f, 0.0f };
	motion.scale = { 0.5f, 0.5f };

	Health& health = registry.healths.emplace(player);
	health.health = 100;
	health.maxHealth = 100;
	// TODO: Add resistances here!

	// Player& player_component = registry.players.emplace(player);
	// // TODO: Add player initialization code here!

	RenderRequest& request = registry.render_requests.emplace(player);
	request.mesh = "sprite";
	request.texture = "mage";
	request.shader = "sprite";
	request.type = PLAYER;

	return player;
}


void WorldSystem::createEnemy(vec2 position, vec2 velocity)
{
	Entity enemy;
	registry.enemies.emplace(enemy);
	Motion& motion = registry.motions.emplace(enemy);
	motion.position = position;
	motion.velocity = velocity;
	motion.scale = { 1.f, 1.f };

	Health& health = registry.healths.emplace(enemy);
	health.health = 100;
	health.maxHealth = 100;

	Deadly& deadly = registry.deadlies.emplace(enemy);
	deadly.to_projectile = true;
	deadly.to_enemy = false;
	deadly.to_player = false;

	Damage& damage = registry.damages.emplace(enemy);
	damage.value = 10.f;
	damage.type = DamageType::enemy;

	RenderRequest& request = registry.render_requests.emplace(enemy);
	request.mesh = "basic";
	request.shader = "basic";
	request.type = ENEMY;
}
