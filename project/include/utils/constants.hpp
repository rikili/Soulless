#pragma once
#include <glm/vec2.hpp>
#include <core/common.hpp>
#include <float.h>

using glm::vec2;

// --- Game Logic Constants ---
const unsigned int MAX_ENEMIES = 50;
const float KNIGHT_SPAWN_INTERVAL_MS = 8000.f;
const float ARCHER_SPAWN_INTERVAL_MS = 16000.f;
const float PALADIN_SPAWN_INTERVAL_MS = 20000.f;
const float SLASHER_SPAWN_INTERVAL_MS = 20000.f;

const unsigned int QUEUE_SIZE = 6;

constexpr float LOW_HEALTH_THRESHOLD = 0.35f;
constexpr float BOSS_LOW_HEALTH_THRESHOLD = 0.5f;

const float ENEMY_INVINCIBILITY_TIMER = 800.f;
const float PLAYER_INVINCIBILITY_TIMER = 1500.f;

const int MAX_PARTICLES = 10000;
const float START_WORLD_TIME = 10 * 60000.f + 1000; // 10 minutes, plus a bit for showing 10 on the clock

// --- Damage Types ---
enum class DamageType
{
    fire,
    water,
    lightning,
    ice,
    plasma,
    wind,
    enemy,
    portal,
    elementless
};

enum class HitTypes
{
    notHit = 0,
    hit = 1,
    absorbed
};

// --- Animation ---
const float DEFAULT_LOOP_TIME = 50.f;

// --- Directions ---
// Note: These are ordered to correspond with the orientation of the spritesheets of our 2.5D characters
enum class Direction
{
    E,
    SE,
    S,
    SW,
    W,
    NW,
    N,
    NE,
};

// --- States ---
enum class AnimationState
{
    IDLE,
    WALKING,
    ATTACKING,
    DYING,
    BLOCKING,
    RUNNING,
    BATTLECRY,
};

// --- Player Constants ---
const float PLAYER_HEALTH = 100.f;
const float PLAYER_MAX_HEALTH = 100.f;
const float PLAYER_SPELL_COOLDOWN = 1000.f;
const float PLAYER_HEAL_AMOUNT = 5.f;
const float PLAYER_HEAL_COOLDOWN = 1500.f;
const float PLAYER_VELOCITY = 0.1f;

// --- Player Spells ---

enum class SpellType
{
    FIRE = 0,
    WATER = 1,
    LIGHTNING = 2,
    ICE = 3,
    WIND = 4,
    PLASMA = 5,
    // Add any new spells here
    COUNT = 6// Used to track how many spell types we have
};
const int NOT_DROPPED_SPELL_COUNT = 1;

// types of post hit resolutions for max spells
enum class PostResolution
{
    FIRE_PROJECTILE = 0,
    WATER_EXPLOSION = 1,
};

// Fire Constants
const float FIRE_DAMAGE = 25.f;
const float FIRE_VELOCITY = 0.8f;
const float FIRE_RANGE = 250.f;
const vec2 FIRE_SCALE = { 0.3, 0.3 };
const float FIRE_SCALE_FACTOR = 3.f;
const vec2 FIRE_COLLIDER = { 25, 25 };
const float FIRE_SCALING[4] = { 1, 1, 1, 1 };

// Max First Constants
const float MAX_FIRE_DAMAGE_DIRECT = 25.f;
const float MAX_FIRE_DAMAGE_SPLASH = 15.f;
const vec2 MAX_FIRE_SCALE = { 0.3, 0.3 };
const float MAX_FIRE_SCALE_FACTOR = 4.f;
const vec2 MAX_FIRE_COLLIDER = { 35, 35 };
const vec2 MAX_FIRE_SPLASH_COLLIDER = { 60, 60 };
const float FIRE_SPLASH_LIFETIME = 200.f;
const vec2 MAX_FIRE_SPLASH_SCALE = { 1.5, 1.5 };
const float MAX_FIRE_SPLASH_RANGE = FLT_MAX;

const float WATER_DAMAGE = 20.f;
const float WATER_VELOCITY = 0.f;
const float WATER_RANGE = FLT_MAX; // Range is "infinite" for barrier
const vec2 WATER_SCALE = { 0.6f, 0.6f };
const vec2 WATER_COLLIDER = { 25.f, 25.f };
const float WATER_LIFETIME = 3000.f; // Barrier spell lasts for 3.5 seconds
const vec2 WATER_EXPLOSION_COLLIDER = { 80, 80 };
const vec2 WATER_EXPLOSION_SCALE = { 1.8, 1.8 };
const float WATER_SPLASH_RANGE = FLT_MAX;
const float WATER_SPLASH_LIFETIME = 200.f;
const float WATER_ABSORB_DMG_BOOST[3] = { 1.2, 1.5, 1.8 };
const float WATER_SCALING[4] = { 1.1, 1, 1, 1.4 };

const float LIGHTNING_ACTIVE_DAMAGE = 30.f;
const float LIGHTNING_VELOCITY = 0.f;
const float LIGHTNING_RANGE = FLT_MAX; // Range is "infinite" for lightning
const vec2 LIGHTNING_SCALE = { 0.75f, 0.75f };
const float LIGHTNING_SCALE_FACTOR = 1.f;
const vec2 LIGHTNING_COLLIDER = { 37.5f, 20.f };
const float LIGHTNING_CASTING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_CHARGING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_ACTIVE_LIFETIME = 250.f; // Lightning bolt lasts 0.25 seconds before disappearing
const int MAX_LIGHTNING_ATTACK_COUNT = 4;
const vec2 MAX_LIGHTNING_DELAY_DIFFERENCE = { 100.f, 400.f };
const vec2 MAX_LIGHTNING_POS_DIFFERENCE = { -50.f, 50.f };
const float MAX_LIGHTNING_DAMAGE = 7.f;
const float LIGHTNING_SCALING[4] = { 1.0, 1.1, 1.1, 1.1 };

const float ICE_DAMAGE = 10.f;
const float ICE_SPEED = 0.4f;
const float ICE_RANGE = 75.f;
const vec2 ICE_SCALE = { 0.2, 0.2 };
const vec2 ICE_COLLIDER = { 8.f, 8.f };
const int ICE_SHARD_COUNT = 5;
const float ICE_DEGREE_DIFFERENCE = 10.f;
const float MAX_ICE_SPEED = 0.7f;
const vec2 MAX_ICE_SCALE = { 0.7, 0.2f };
const vec2 MAX_ICE_COLLIDER = { 15.f, 15.f };
const float MAX_ICE_RANGE = 400.f;
const float MAX_ICE_DAMAGE = 30.f;
const float ICE_SCALING[4] = { 1.0, 1.1, 1.1, 1.1 };

const float WIND_DAMAGE = 6.f;
const float WIND_RANGE = FLT_MAX;
const vec2 WIND_SCALE = { 0.75f, 0.75f };
const float WIND_SCALE_FACTOR = 1.f;
const vec2 WIND_COLLIDER = { 40.f, 40.f };
const float WIND_PLACEMENT_LIFETIME = 3000.f;

const float PLASMA_DAMAGE = 30.f;
const float PLASMA_SPEED = 0.01f;
const float PLASMA_MAX_SPEED = 0.6f;
const float PLASMA_RANGE = 225.f;
const vec2 PLASMA_SCALE = { 0.4f, 0.4f };
const vec2 PLASMA_COLLIDER = { 20.f, 20.f };
const float PLASMA_SCALE_FACTOR = 4.f;

// --- Enemy Types ---
enum class EnemyType
{
    KNIGHT,
    ARCHER,
    PALADIN,
    SLASHER,
    DARKLORD // Boss
};

// --- Enemy Constants ---
const float ENEMY_BASIC_RANGE = 100.f;


// Knight + Pitchfork
const float KNIGHT_HEALTH = 30.f;
const float KNIGHT_COOLDOWN = 4000.f;
const float KNIGHT_VELOCITY = 0.03f;
const float KNIGHT_RANGE = 100.f;
const float KNIGHT_DAMAGE = 5.f;
const float PITCHFORK_VELOCITY = 0.125f;
const float PITCHFORK_DAMAGE = 10.f;
const float KNIGHT_BLOCK_COOLDOWN = 4000.f;
const float KNIGHT_RETREAT_DISTANCE = 200.f;

// Archer + Arrow
const float ARCHER_HEALTH = 50.f;
const float ARCHER_COOLDOWN = 2500.f;
const float ARCHER_VELOCITY = 0.025f;
const float ARCHER_RANGE = 200.f;
const float ARCHER_DAMAGE = 5.f;
const float ARROW_VELOCITY = 0.175f;
const float ARROW_DAMAGE = 20.f;
const float ARCHER_RETREAT_DISTANCE = 250.f;

// Paladin + Sword
const float PALADIN_HEALTH = 100.f;
const float PALADIN_COOLDOWN = 3000.f;
const float PALADIN_VELOCITY = 0.03f;
const float PALADIN_RANGE = 20.f; // Melee range
const float PALADIN_DAMAGE = 25.f;
const float SWORD_VELOCITY = 0.175f;
const float SWORD_DAMAGE = 35.f;

// Slasher
const float SLASHER_HEALTH = 25.f;
const float SLASHER_COOLDOWN = 750.f;
const float SLASHER_VELOCITY = 0.075f;
const float SLASHER_RANGE = 50.f;
const float SLASHER_DAMAGE = 20.f;

// Dark Lord + Razor Wind + Claw Pull
const float DARKLORD_HEALTH = 750.f;
const float DARKLORD_VELOCITY = 0.025f;
const float DARKLORD_DAMAGE = 20.f;
const float DARKLORD_RANGE = 250.f;
const float DARKLORD_RAZOR_COOLDOWN = 3500.f;
const float DARKLORD_PORTAL_COOLDOWN = 20000.f;
const float DARKLORD_RAZOR_DAMAGE = 25.f;
const float DARKLORD_RAZOR_SPEED = 0.05f;
const float DARKLORD_RAZOR_MAX_SPEED = 0.75f;
const vec2 DARKLORD_SPAWN_POS = { window_width_px / 2.f, window_height_px / 2.f };
const vec2 DARKLORD_SPAWN_VEL = { 0, 0 };

// --- World Interactables ---
enum class InteractableType
{
    HEALER,
    POWER,
    BOSS,
    PLASMA
};

// --- Upgrade Requirements ---
const int MAX_SPELL_LEVEL = 5;
const int UPGRADE_KILL_COUNT[5] = { 10, 20, 30, 40, 40 };

// --- Interactable Timers ---
const float POWERUP_DECAY = 30000.f;
const float POWERUP_SPAWN_TIMER = 35000.f;
const float POWERUP_SPAWN_BUFFER = 180.f; // distance from edge to spawn
const float MIN_POWERUP_DIST = 80;

const std::string ALTAR_INTERACT = "Press R to meet (END) your fate";
const vec2 ALTAR_POSITION = vec2(window_width_px / 2.f, window_height_px / 2.f + 150.f);
const vec2 ALTAR_COLLIDER = vec2(50, 50);

const std::string PLASMA_ALTAR_INTERACT = "Press R to take a gamble (Need 5 spell upgrades)";
const vec2 PLASMA_ALTAR_POSITION = vec2(window_width_px / 2.f, window_height_px / 2.f - 150.f);
const vec2 PLASMA_ALTAR_COLLIDER = vec2({ 80, 60 });
const vec2 PLASMA_ALTAR_SCALE = { 2.5, 2.5 };
const float PLASMA_ALTAR_SPAWN = 2.5 * 60000.f;

const int PLASMA_SACRIFICE_COST = 5;
const vec2 PLASMA_SPAWN_LOCATION = vec2(PLASMA_ALTAR_POSITION.x, PLASMA_ALTAR_POSITION.y + 100.f);

// --- Offsets ---
const float HEALTH_BAR_Y_OFFSET = -15.f;
const float RENDER_PAST_SCREEN_OFFSET = 25.f;

// Draw order (largest number = frontmost)
enum
{
    BACK = 0,
    PROJECTILE = 1,
    ENEMY = 2,
    PLAYER = 3,
    OVER_PLAYER = 4,
};
