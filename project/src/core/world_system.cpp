#include "core/world_system.hpp"

#include "entities/ecs_registry.hpp"
#include "sound/sound_manager.hpp"
#include "utils/isometric_helper.hpp"
#include "graphics/tile_generator.hpp"
#include "utils/serializer.hpp"
#include "utils/enemy_factory.hpp"
#include <utils/spell_factory.hpp>

WorldSystem::WorldSystem(IRenderSystem* renderer)
{
	this->renderer = renderer;
	this->collision_system = new CollisionSystem(renderer);
}

WorldSystem::~WorldSystem() {}

// Should the game be over ?
bool WorldSystem::isOver() const {
	return bool(glfwWindowShouldClose(window));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	if (!registry.players.has(player_mage) || registry.game_over) {
		printd("\n----------------\nGame Over! Resetting...\n----------------\n");
		registry.game_over = false;
		registry.clear_all_components();
		this->restartGame();
		return true;
	}

	this->handleProjectiles(elapsed_ms_since_last_update);
	this->handle_enemy_logic(elapsed_ms_since_last_update);
	this->handleMovements(elapsed_ms_since_last_update);
	this->collision_system->detect_collisions();
	this->collision_system->resolve_collisions();
	this->handleAnimations();
	this->handleHealthBars();
	this->handleRain();
	this->handleTimers(elapsed_ms_since_last_update);
	this->handleAI(elapsed_ms_since_last_update);
	this->handleSpellStates(elapsed_ms_since_last_update);
	particleSystem.updateParticles(elapsed_ms_since_last_update);
	registry.collision_registry.clear_collisions();
	return true;
}

void WorldSystem::handleAI(float elapsed_ms_since_last_update) {

	if (registry.game_over) {
		return;
	}

	for (Entity& entity : registry.ai_systems.entities) {
		AI_SYSTEM::tickForEntity(&entity, elapsed_ms_since_last_update);
	}
}

void WorldSystem::handleRain() {
	std::random_device rd;	// Random device
	std::mt19937 gen(rd()); // Mersenne Twister generator
	std::uniform_real_distribution<float> pos_distr(0, window_width_px);
	std::uniform_real_distribution<float> vel_distr(0.1, 0.4);
	for (int i = 0; i < 10; i++) {
		particleSystem.emitParticle({ pos_distr(gen), 0 }, { 0, vel_distr(gen) }, 4000, 4);
	}
}

void WorldSystem::setRenderer(IRenderSystem* renderer)
{
	this->renderer = renderer;
}

void WorldSystem::handleHealthBars() {

	// For each healthbar
	for (Entity& entity : registry.healthBars.entities) {

		// Needed to get entity to which the healthbar is assigned
		HealthBar& healthbar = registry.healthBars.get(entity);

		// Check to ensure healthbar is assigned
		if (healthbar.assigned) {

			// Update healthbar position to match the entity it's assigned to
			Entity assignedTo = healthbar.assignedTo;

			// If enemy has died during collision, destroy its health bar
			if (registry.deaths.has(assignedTo) && registry.enemies.has(assignedTo)) {
				registry.remove_all_components_of(entity);
			}

			if (!registry.motions.has(assignedTo)) {
				printd("Error updating health bar: it is assigned to an entity without a motion component\n");
				return;
			}
			healthbar.position.x = registry.motions.get(assignedTo).position.x;
			healthbar.position.y = registry.motions.get(assignedTo).position.y + HEALTH_BAR_Y_OFFSET;
		}
		else {
			printd("Error updating health bar: it is unassigned.\n");
		}
	}
}

void WorldSystem::handleAnimations() {
	for (Entity e : registry.animations.entities) {
		if (!registry.players.has(e) && !registry.enemies.has(e)) {
			continue;
		}

		Motion& motion = registry.motions.get(e);
		Animation& animation = registry.animations.get(e);
		RenderRequest& rr = registry.render_requests.get(e);

		if (animation.state == EntityState::ATTACKING) {
			animation.oneTime = true;
			rr.texture = peToString(e) + "-attack";
		}
		else {

			if (motion.velocity.x == 0 && motion.velocity.y == 0) {
				rr.texture = peToString(e) + "-idle";
			}
			else {
				rr.texture = peToString(e) + "-walk";
			}

			if (motion.currentDirection == motion.oldDirection) {
				continue;
			}

			animation.initializeAtRow((int)motion.currentDirection);
		}
	}
	// printd("Current: %d\n", playerMotion.currentDirection);
}
/**
 * @brief Handle projectiles to reduce their range at each step and mark for
 * deletion if they are out of range
 * @param
 */
void WorldSystem::handleProjectiles(float elapsed_ms_since_last_update)
{
	for (Entity& projectile_ent : registry.projectiles.entities)
	{
		if (registry.deaths.has(projectile_ent))
		{
			continue;
		}

		Projectile& projectile = registry.projectiles.get(projectile_ent);
		Motion& motion = registry.motions.get(projectile_ent);
		Deadly& deadly = registry.deadlies.get(projectile_ent);
		bool doLinear = registry.spellProjectiles.has(projectile_ent) ? !registry.spellProjectiles.get(projectile_ent).isPostAttack : true;

		projectile.range -= projectile.type == DamageType::portal ? elapsed_ms_since_last_update : sqrt(motion.velocity.x * motion.velocity.x + motion.velocity.y * motion.velocity.y) * elapsed_ms_since_last_update;

		if (deadly.to_enemy && projectile.type == DamageType::fire && doLinear)
		{
			vec2 scale_factor = FIRE_SCALE + ((FIRE_RANGE - projectile.range) / FIRE_RANGE) * (FIRE_SCALE_FACTOR * FIRE_SCALE - FIRE_SCALE);
			motion.scale.x = scale_factor.x;
			motion.scale.y = scale_factor.y;
			// particleSystem.emitParticle(motion.position, {-motion.velocity.x / 4, -motion.velocity.y / 4}, 100, 5);
		}
		else if (projectile.type == DamageType::plasma) {
			float MAX_SPEED;         											// max speed
			float MAX_RANGE;       												// max range of projectile
			if (deadly.to_enemy) {
				MAX_SPEED = PLASMA_MAX_SPEED;
				MAX_RANGE = PLASMA_RANGE;
			}
			else {
				MAX_SPEED = DARKLORD_RAZOR_MAX_SPEED;
				MAX_RANGE = DARKLORD_RANGE;
			}

			const float SCALE_FACTOR = 25.0f;     				// steepness of curve / ramp-up
			const float SHIFT = 0.2f;             				// adjusts when the ramp-up starts
			const float TIME_FACTOR = 0.01f;      				// time scaling

			// Calculate the base speed dynamically
			float speed = sqrt(motion.velocity.x * motion.velocity.x + motion.velocity.y * motion.velocity.y);

			float range_progress = 1.0f - (projectile.range / MAX_RANGE);
			range_progress = clamp(range_progress, 0.0f, 1.0f);

			float log_factor = 1.0f / (1.0f + exp(-SCALE_FACTOR * (range_progress - SHIFT)));
			log_factor = pow(log_factor, 2.0f); // square to make steeper

			float new_speed = speed + (MAX_SPEED - speed) * log_factor;

			// printf("Range Progress: %f, Logistic Factor: %f, Plasma Speed: %f\n", range_progress, log_factor, new_speed);

			motion.velocity = normalize(motion.velocity) * new_speed;
		}

		if (projectile.range <= 0)
		{
			registry.deaths.emplace(projectile_ent);
			// printd("Marked for removal due to distance travelled - Entity value:
			// %u\n", static_cast<unsigned>(projectile_ent));
		}
	}
}

/**
 * @brief In charge of updating the position of all entities with a motion
 * component
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handleMovements(float elapsed_ms_since_last_update)
{
	ComponentContainer<Motion>& motions_registry = registry.motions;
	Motion& player_motion = motions_registry.get(player_mage);

	// Update all motions
	for (Entity entity : motions_registry.entities)
	{
		Motion& motion = motions_registry.get(entity);

		if (registry.players.has(entity) || registry.enemies.has(entity))
		{
			float x_offset = motion.collider.x * motion.scale.x;
			float y_offset = motion.collider.y * motion.scale.y;

			float slowFactor = 1;
			if (registry.debuffs.has(entity)) {
					Debuff &debuff = registry.debuffs.get(entity);
					if (debuff.type == DebuffType::SLOW) {
						slowFactor = debuff.strength;
					}
			}

			motion.position = glm::clamp(motion.position + motion.velocity * slowFactor * elapsed_ms_since_last_update, { x_offset, y_offset }, { window_width_px - x_offset, window_height_px - y_offset });

			if (registry.enemies.has(entity))
			{
				// Enemy& enemy = registry.enemies.get(entity);
				motion.angle = atan2(player_motion.position.y - motion.position.y,
					player_motion.position.x - motion.position.x);

				// printd("Enemy angle towards player: %f\n", motion.angle);
			}

			if (registry.enemies.has(entity) && registry.enemies.get(entity).movementRestricted) {

			}
			else {
				computeNewDirection(entity);
			}
		}

		// not a player nor enemy
		else if (registry.projectiles.has(entity)) {
			Projectile& projectile = registry.projectiles.get(entity);

			if (projectile.type == DamageType::water) {
				motion.position = player_motion.position;
			}
			else if (projectile.type == DamageType::lightning) {
				// lightning attack doesn't move
			}
			else {
				motion.position += motion.velocity * elapsed_ms_since_last_update;
			}
		}

		RenderRequest& render_request = registry.render_requests.get(entity);
		render_request.smooth_position.update(motion.position.y);
	}
}

void WorldSystem::computeNewDirection(Entity e) {

	// Do not recompute direction while attack animation is in progress, otherwise the direction of that animation will be lost before it's finished
	if (registry.animations.has(e) && registry.animations.get(e).state == EntityState::ATTACKING) {
		return;
	}

	Motion& motion = registry.motions.get(e);
	motion.oldDirection = motion.currentDirection;


	float xVel = motion.velocity.x;
	float yVel = motion.velocity.y;

	if (xVel == 0 && yVel == 0) {
		return;
	}

	if (xVel == 0) {
		if (yVel < 0) {
			motion.currentDirection = Direction::N;
		}
		else {
			motion.currentDirection = Direction::S;
		}
		return;
	}

	if (yVel == 0) {
		if (xVel > 0) {
			motion.currentDirection = Direction::E;
		}
		else {
			motion.currentDirection = Direction::W;
		}
		return;
	}

	if (xVel < 0) {
		if (yVel < 0) {
			motion.currentDirection = Direction::NW;
		}
		else {
			motion.currentDirection = Direction::SW;
		}
		return;
	}

	if (xVel > 0) {
		if (yVel < 0) {
			motion.currentDirection = Direction::NE;
		}
		else {
			motion.currentDirection = Direction::SE;
		}
		return;
	}

}
/**
 * @brief In charge of updating timers and their side effects
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handleTimers(float elapsed_ms_since_last_update)
{
	handleCollectible(elapsed_ms_since_last_update);

	for (Entity& hit_ent : registry.onHits.entities)
	{
		OnHit& onHit = registry.onHits.get(hit_ent);
		std::unordered_set<int> to_remove;
		for (auto& hit : onHit.invuln_tracker)
		{
			if (registry.players.has(hit_ent))
			{
				if (hit.second < PLAYER_INVINCIBILITY_TIMER - 200.f) onHit.invicibilityShader = true;
				else onHit.invicibilityShader = false;
			}

			if (hit.second < ENEMY_INVINCIBILITY_TIMER - 200.f)
			{
				onHit.invicibilityShader = false;
			}
			else
			{
				onHit.invicibilityShader = true;
			}

			hit.second -= elapsed_ms_since_last_update;

			if (hit.second <= 0)
			{
				to_remove.insert(hit.first);
			}
		}

		for (auto& remove : to_remove)
		{
			onHit.invuln_tracker.erase(remove);
		}

		onHit.isInvincible = onHit.invuln_tracker.size() > 0;
	}

	for (Entity& healed_ent : registry.onHeals.entities)
	{
		OnHeal& heal = registry.onHeals.get(healed_ent);
		heal.heal_time -= elapsed_ms_since_last_update;
		if (heal.heal_time < 0)
		{
			registry.onHeals.remove(healed_ent);
		}
	}

	for (Entity& dead_ent : registry.deaths.entities)
	{
		Death& death = registry.deaths.get(dead_ent);
		death.timer -= elapsed_ms_since_last_update;
		if (death.timer < 0)
		{
			if (registry.players.has(dead_ent))
			{
				registry.game_over = true;
			}
			if (registry.enemies.has(dead_ent)
				&& registry.enemies.get(dead_ent).type == EnemyType::DARKLORD)
			{
				Motion& motion = registry.motions.get(dead_ent);
				createCollectible(motion.position, SpellType::PLASMA);
			}
			registry.remove_all_components_of(dead_ent);
		}
	}

	for (Entity& decay_ent : registry.decays.entities)
	{
		Decay& decay = registry.decays.get(decay_ent);
		decay.timer -= elapsed_ms_since_last_update;
		if (decay.timer < 0)
		{
			if (registry.spellProjectiles.has(decay_ent) && registry.spellProjectiles.get(decay_ent).type == SpellType::WIND) {
				for (Entity e : registry.spellProjectiles.get(decay_ent).victims) {
					if (registry.enemies.has(e)) {
						registry.enemies.get(e).movementRestricted = false;
					}
				}
				registry.spellProjectiles.get(decay_ent).victims.clear();
			}
			Death& death = registry.deaths.emplace(decay_ent);
			death.timer = 0;
		}
	}

	for (Entity& debuff_ent : registry.debuffs.entities)
	{
		Debuff& debuff = registry.debuffs.get(debuff_ent);
		debuff.timer -= elapsed_ms_since_last_update;
		if (debuff.timer < 0)
		{
			registry.debuffs.remove(debuff_ent);
		}
	}

	for (Entity& player_ent : registry.players.entities)
	{
		Player& player = registry.players.get(player_ent);
		player.leftCooldown -= elapsed_ms_since_last_update;
		player.rightCooldown -= elapsed_ms_since_last_update;

		if (player.leftCooldown < 0)
		{
			player.leftCooldown = 0;
			player.leftCooldownTotal = 0;
		}
		if (player.rightCooldown < 0)
		{
			player.rightCooldown = 0;
			player.rightCooldownTotal = 0;
		}
	}

	for (Entity& enemy_ent : registry.enemies.entities)
	{
		Enemy& enemy = registry.enemies.get(enemy_ent);
		enemy.cooldown -= elapsed_ms_since_last_update;
		enemy.secondCooldown -= elapsed_ms_since_last_update;
		if (enemy.cooldown < 0)
		{
			// printd("Enemy cooldown is less than 0\n");
			enemy.cooldown = 0;
		}
		if (enemy.secondCooldown < 0) {
			enemy.secondCooldown = 0;
		}
	}
}

/**
 * @brief Handle spell states. Update spells and their states based on timers.
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handleSpellStates(float elapsed_ms_since_last_update)
{
	if (lightnings_to_create.size() > 0)
	{
		vec2 position = lightnings_to_create.front();
		lightnings_to_create.pop();

		std::random_device rand;
		std::mt19937 gen(rand());
		std::uniform_real_distribution<float> distance(MAX_LIGHTNING_POS_DIFFERENCE.x, MAX_LIGHTNING_POS_DIFFERENCE.y);
		vec2 new_position = vec2(position);
		SpellFactory::createSpellProjectile(
			registry,
			registry.players.entities[0],
			SpellType::LIGHTNING,
			-1,
			(position.x + distance(gen)),
			(position.y + distance(gen)),
			true);
	}

	for (Entity& spell_ent : registry.spellStates.entities) {
		SpellState& spell_state = registry.spellStates.get(spell_ent);
		RenderRequest& request = registry.render_requests.get(spell_ent);
		Damage& damage = registry.damages.get(spell_ent);
		Projectile& projectile = registry.projectiles.get(spell_ent);
		Deadly& deadly = registry.deadlies.get(spell_ent);
		SpellProjectile& spell_proj = registry.spellProjectiles.get(spell_ent);

		spell_state.timer -= elapsed_ms_since_last_update;

		// explicit state change when timer is done
		if (spell_state.timer <= 0) {

			State current_state = spell_state.state;

			switch (current_state) {
			case State::CASTING: {
				spell_state.state = State::CHARGING;

				if (projectile.type == DamageType::lightning) {
					request.texture = "lightning2";
					spell_state.timer = LIGHTNING_CHARGING_LIFETIME;
				}
				break;
			}
			case State::CHARGING: {
				spell_state.state = State::ACTIVE;

				if (projectile.type == DamageType::lightning) {
					deadly.to_enemy = true;
					projectile.isActive = true;
					request.texture = "lightning3";
					spell_state.timer = LIGHTNING_ACTIVE_LIFETIME;
				}
				break;
			}
			case State::ACTIVE: {
				spell_state.state = State::COMPLETE;
				break;
			}
			case State::COMPLETE: {
				if (!registry.deaths.has(spell_ent))
				{
					if (spell_proj.type == SpellType::WATER)
					{
						Motion& motion = registry.motions.get(registry.players.entities[0]);
						SpellFactory::createSpellResolution(registry, motion.position, PostResolution::WATER_EXPLOSION, spell_ent);
					}

					if (spell_proj.type == SpellType::LIGHTNING
						&& registry.spellProjectiles.has(spell_ent)
						&& registry.spellProjectiles.get(spell_ent).level >= MAX_SPELL_LEVEL)
					{
						if (!spell_state.isChild)
						{
							Motion& motion = registry.motions.get(spell_ent);
							for (int x = 0; x < MAX_LIGHTNING_ATTACK_COUNT; x++)
							{
								lightnings_to_create.push(vec2(motion.position));
							}
						}
					}
					registry.deaths.emplace(spell_ent);
				}
				break;
			}
			default: {
				break;
			}
			}

		}
	}
}


/**
 * Initialize the game world
 */
void WorldSystem::initialize() {
	restartGame();
}

void WorldSystem::restartGame() {
	if (registry.players.entities.size() > 0)
	{
		registry.clear_all_components();
	}
	if (!globalOptions.tutorial && !globalOptions.pause) {
		SoundManager* soundManager = SoundManager::getSoundManager();
		soundManager->playMusic(Song::MAIN);
	}
	player_mage = this->createPlayer();
	this->createTileGrid();
	// loadBackgroundObjects(); // Not used since we removed campfire
	this->renderer->initializeCamera();
	powerup_timer = POWERUP_SPAWN_TIMER;

	//createCollectible({ 1000, 200 }, SpellType::LIGHTNING);

	// Reset enemy spawn timers (rework this if needed)
	enemySpawnTimers.knight = 0.0f;
	enemySpawnTimers.archer = 60000.f;
	enemySpawnTimers.paladin = 120000.f;
	enemySpawnTimers.slasher = 180000.f;
	enemySpawnTimers.darklord = 0.0f;

	// Spawn all at start (for debug)
	/*
	enemySpawnTimers.archer = 0.f;
	enemySpawnTimers.paladin = 0.f;
	enemySpawnTimers.slasher = 0.f;
	enemySpawnTimers.darklord = 0.f;
	*/
}

void WorldSystem::reloadGame() {
	restartGame();
	Serializer::deserialize();
}

void WorldSystem::createTileGrid() {
	int w, h;
	glfwGetFramebufferSize(renderer->getGLWindow(), &w, &h);
	vec2 gridDim = IsometricGrid::getGridDimensions(w, h);
	int numCols = static_cast<int>(gridDim.x);
	int numRows = static_cast<int>(gridDim.y);
	auto* batchRenderer = new BatchRenderer();

	TileGenerator tileGenerator(numCols, numRows, w, h, true);
	tileGenerator.generateTiles(batchRenderer);
	batchRenderer->finalizeBatches();
	renderer->addSubRenderer("tiles", batchRenderer);
}

Entity WorldSystem::createPlayer() {
	auto player = Entity();

	Player& player_component = registry.players.emplace(player);
	player_component.spell_queue = SpellQueue();


	Motion& motion = registry.motions.emplace(player);
	motion.position = { window_width_px / 2.0f,
										 window_height_px / 2.0f }; // Center of the screen
	motion.velocity = { 0.0f, 0.0f };
	motion.scale = { 1.0f, 1.0f };

	Health& health = registry.healths.emplace(player);
	health.health = PLAYER_HEALTH;
	health.maxHealth = PLAYER_MAX_HEALTH;

	auto healthBar = Entity();
	HealthBar& healthBarComp = registry.healthBars.emplace(healthBar);
	healthBarComp.assignHealthBar(player);
	healthBarComp.position = { motion.position.x, motion.position.y - HEALTH_BAR_Y_OFFSET };

	// TODO: Add resistances here!

	Animation& animation = registry.animations.emplace(player);
	animation.spriteCols = 15;
	animation.spriteRows = 8;
	animation.spriteCount = 120;
	animation.frameCount = 15;
	animation.initializeAtFrame(0.0f);

	RenderRequest& request = registry.render_requests.emplace(player);
	request.mesh = "sprite";
	request.texture = "mage-idle";
	request.shader = "animatedsprite";
	request.type = PLAYER;

	MeshCollider& collider = registry.mesh_colliders.emplace(player);
	collider.mesh = "mage_collider";

	return player;
}


void WorldSystem::createEnemy(EnemyType type, vec2 position, vec2 velocity)
{
	EnemyType enemy_type = type;

	switch (enemy_type) {
	case EnemyType::KNIGHT:
		EnemyFactory::createKnight(registry, position, velocity);
		break;
	case EnemyType::ARCHER:
		EnemyFactory::createArcher(registry, position, velocity);
		break;
	case EnemyType::PALADIN:
		EnemyFactory::createPaladin(registry, position, velocity);
		break;
	case EnemyType::SLASHER:
		EnemyFactory::createSlasher(registry, position, velocity);
		break;
	case EnemyType::DARKLORD:
		EnemyFactory::createDarkLord(registry, position, velocity);
		break;
	}
}

// Not used at the moment since we removed campfire
void WorldSystem::loadBackgroundObjects() {
	// createBackgroundObject({ window_width_px / 4, window_height_px / 4 }, { 0.75, 0.75 }, "tree", false);
}

Entity WorldSystem::createBackgroundObject(vec2 position, vec2 scale, AssetId texture, bool animate)
{
	Entity object;

	Motion& motion = registry.motions.emplace(object);
	motion.position = position;
	motion.velocity = { 0.f, 0.f };
	motion.scale = scale;

	RenderRequest& request = registry.render_requests.emplace(object);
	request.mesh = "sprite";
	request.texture = texture;
	if (animate) {
		request.shader = "animatedsprite";
	}
	else {
		request.shader = "sprite";
	}

	return object;
}

/**
 * @brief Handles the logic for spawning enemies and their movement direction
 * towards the player
 * @param elapsed_ms_since_last_update
 * @return void
 * If the enemy spawn timer has elapsed, a new enemy is spawned at a random
 * location Enemies are spawned outside the window and move towards the player
 */
void WorldSystem::handle_enemy_logic(const float elapsed_ms_since_last_update)
{
	enemySpawnTimers.knight -= elapsed_ms_since_last_update;
	enemySpawnTimers.archer -= elapsed_ms_since_last_update;
	enemySpawnTimers.paladin -= elapsed_ms_since_last_update;
	enemySpawnTimers.slasher -= elapsed_ms_since_last_update;
	enemySpawnTimers.darklord -= elapsed_ms_since_last_update;

	const bool should_spawn_knight = enemySpawnTimers.knight <= 0;
	const bool should_spawn_archer = enemySpawnTimers.archer <= 0;
	const bool should_spawn_paladin = enemySpawnTimers.paladin <= 0;
	const bool should_spawn_slasher = enemySpawnTimers.slasher <= 0;
	const bool should_spawn_darklord = enemySpawnTimers.darklord <= 0;

	if (should_spawn_knight || should_spawn_archer || should_spawn_paladin || should_spawn_slasher || should_spawn_darklord)
	{
		std::random_device rd;	// Random device
		std::mt19937 gen(rd()); // Mersenne Twister generator
		enum SIDE
		{
			TOP,
			RIGHT,
			BOTTOM,
			LEFT
		}; // Side of the window to spawn from
		std::uniform_int_distribution<int> side_dis(TOP, LEFT);
		const int side = side_dis(gen);

		float candidate_x = 0.f, candidate_y = 0.f;
		std::uniform_real_distribution<float> dis(0.f, 1.f);
		constexpr float offset_x = window_width_px / 10.f;
		constexpr float offset_y = window_height_px / 10.f;

		switch (side) {
		case TOP:
			candidate_x = dis(gen) * window_width_px;
			candidate_y = -offset_y;
			break;
		case RIGHT:
			candidate_x = window_width_px + offset_x;
			candidate_y = dis(gen) * window_height_px;
			break;
		case BOTTOM:
			candidate_x = dis(gen) * window_width_px;
			candidate_y = window_height_px + offset_y;
			break;
		case LEFT:
		default: // Should never happen but just in case
			candidate_x = -offset_x;
			candidate_y = dis(gen) * window_height_px;
			break;
		}
		const vec2 position = { candidate_x, candidate_y };

		if (should_spawn_knight) {
			enemySpawnTimers.knight = KNIGHT_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::KNIGHT, position, { 0, 0 });
		}

		if (should_spawn_archer) {
			enemySpawnTimers.archer = ARCHER_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::ARCHER, position, { 0, 0 });
		}

		if (should_spawn_paladin) {
			enemySpawnTimers.paladin = PALADIN_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::PALADIN, position, { 0, 0 });
		}

		if (should_spawn_slasher) {
			enemySpawnTimers.slasher = SLASHER_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::SLASHER, position, { 0, 0 });
		}

		if (should_spawn_darklord) {
			enemySpawnTimers.darklord = DARKLORD_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::DARKLORD, position, { 0, 0 });
		}
	}
}

void WorldSystem::handleCollectible(const float elapsed_ms_since_last_update)
{
	powerup_timer -= elapsed_ms_since_last_update;
	if (registry.debug && globalOptions.debugSpellSpawn)
	{
		powerup_timer = 0;
		globalOptions.debugSpellSpawn = false;
	}
	if (powerup_timer <= 0)
	{
		std::random_device rd;	// Random device
		std::mt19937 gen(rd()); // Mersenne Twister generator
		std::uniform_real_distribution<float> hor_distr(0 + POWERUP_SPAWN_BUFFER, window_width_px - POWERUP_SPAWN_BUFFER);
		std::uniform_real_distribution<float> ver_distr(0 + POWERUP_SPAWN_BUFFER, window_height_px - POWERUP_SPAWN_BUFFER);

		Motion& motion = registry.motions.get(player_mage);
		Player& player = registry.players.get(player_mage);
		SoundManager* sound_manager = SoundManager::getSoundManager();
		const std::vector<SpellType> missing_spells = player.spell_queue.getMissingSpells();
		int remaining_spells = missing_spells.size() - NOT_DROPPED_SPELL_COUNT;
		std::uniform_int_distribution<int> spell_choice(0, static_cast<int>(SpellType::COUNT) - 1 - NOT_DROPPED_SPELL_COUNT);
		while (true)
		{
			float x = hor_distr(gen);
			float y = ver_distr(gen);
			if (glm::distance(motion.position, { x , y }) > MIN_POWERUP_DIST)
			{
				if (registry.debug) printf("DEBUG: spawning powerup at %f %f\n", x, y);
				sound_manager->playSound(SoundEffect::POWERUP_SPAWN);
				createCollectible({ x, y }, static_cast<SpellType>(spell_choice(gen)));
				break;
			}
		}

		powerup_timer = POWERUP_SPAWN_TIMER;
	}
}

void WorldSystem::createCollectible(const vec2 position, const SpellType type)
{
	Entity entity;

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = { 0.5, 0.5 };

	RenderRequest& request = registry.render_requests.emplace(entity);
	request.mesh = "sprite";
	request.shader = "sprite";
	request.type = ENEMY;

	Interactable& interact = registry.interactables.emplace(entity);
	interact.type = InteractableType::POWER;
	SpellUnlock& unlock = registry.spellUnlocks.emplace(entity);
	unlock.type = type;
	Decay& decay = registry.decays.emplace(entity);
	decay.timer = POWERUP_DECAY;

	switch (type) {
	case SpellType::LIGHTNING:
		request.texture = "lightning-collect";
		break;
	case SpellType::WATER:
		request.texture = "water-collect";
		break;
	case SpellType::ICE:
		request.texture = "ice-collect";
		break;
	case SpellType::WIND:
		request.texture = "wind-collect";
		break;
	case SpellType::PLASMA:
		request.texture = "plasma-collect";
		break;
	default:
		registry.remove_all_components_of(entity);
	}
}

std::string WorldSystem::peToString(Entity e) {

	if (registry.players.has(e)) {
		return "mage";
	}
	else if (registry.enemies.has(e)) {
		switch (registry.enemies.get(e).type) {
		case EnemyType::KNIGHT: return "knight";
		case EnemyType::ARCHER: return "archer";
		case EnemyType::PALADIN: return "paladin";
		case EnemyType::SLASHER: return "slasher";
		case EnemyType::DARKLORD: return "darklord";
		default: return "unknown";
		}
	}
	else {
		return "invalid";
	}

}

Entity WorldSystem::getPlayer()  const {
	return player_mage;
}
