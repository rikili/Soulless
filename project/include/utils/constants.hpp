#pragma once
#include <glm/vec2.hpp>
#include <float.h>

using glm::vec2;

// --- Game Logic Constants ---
const unsigned int MAX_ENEMIES = 50;
const float KNIGHT_SPAWN_INTERVAL_MS = 8000.f;
const float ARCHER_SPAWN_INTERVAL_MS = 16000.f;
const float PALADIN_SPAWN_INTERVAL_MS = 20000.f;
const unsigned int QUEUE_SIZE = 7;

constexpr float LOW_HEALTH_THRESHOLD = 30.0f;
constexpr float HEALTH_RECOVERY_RATE = 0.2f;

const float ENEMY_INVINCIBILITY_TIMER = 800.f;
const float PLAYER_INVINCIBILITY_TIMER = 1500.f;

const int MAX_PARTICLES = 10000;

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
    elementless
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
enum class EntityState
{
    IDLE,
    WALKING,
    ATTACKING
};

// --- Player Constants ---
const float PLAYER_HEALTH = 100.f;
const float PLAYER_MAX_HEALTH = 100.f;
const float PLAYER_HEAL_COOLDOWN = 10000.f;
const float PLAYER_VELOCITY = 0.2f;

// --- Player Spells ---

enum class SpellType
{
    FIRE = 0,
    WATER = 1,
    LIGHTNING = 2,
    ICE = 3,
    // Add any new spells here
    COUNT // Used to track how many spell types we have
};

const float FIRE_DAMAGE = 25.f;
const float FIRE_VELOCITY = 0.8f;
const float FIRE_RANGE = 250.f;
const vec2 FIRE_SCALE = { 0.3, 0.3 };
const float FIRE_SCALE_FACTOR = 3.f;
const vec2 FIRE_COLLIDER = { 50, 50 };

const float WATER_DAMAGE = 0.f;
const float WATER_VELOCITY = 0.f;
const float WATER_RANGE = FLT_MAX; // Range is "infinite" for barrier
const vec2 WATER_SCALE = { 0.6f, 0.6f };
// const float WATER_SCALE_FACTOR = 1.f;
const vec2 WATER_COLLIDER = { 60.f, 60.f };
const float WATER_LIFETIME = 1000.f; // Barrier spell lasts for 1 second (or if it collides with enemy projectile)

const float LIGHTNING_CASTING_DAMAGE = 0.f;
const float LIGHTNING_ACTIVE_DAMAGE = 35.f;
const float LIGHTNING_VELOCITY = 0.f;
const float LIGHTNING_RANGE = FLT_MAX; // Range is "infinite" for lightning
const vec2 LIGHTNING_SCALE = { 0.75f, 0.75f };
const float LIGHTNING_SCALE_FACTOR = 1.f;
const vec2 LIGHTNING_COLLIDER = { 75.f, 40.f };
const float LIGHTNING_CASTING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_CHARGING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_ACTIVE_LIFETIME = 250.f; // Lightning bolt lasts 0.25 seconds before disappearing

const float ICE_DAMAGE = 10.f;
const float ICE_SPEED = 0.4f;
const float ICE_RANGE = 100.f;
const vec2 ICE_SCALE = { 0.2, 0.2 };
const vec2 ICE_COLLIDER = { 13.f, 13.f };
const int ICE_SHARD_COUNT = 5;
const float ICE_DEGREE_DIFFERENCE = 10.f;

// LIGHTNING
// PLASMA
// WATER
// WIND

// --- Enemy Types ---
enum class EnemyType
{
    KNIGHT,
    ARCHER,
    PALADIN
};

// --- Enemy Constants ---
const float ENEMY_BASIC_RANGE = 100.f;

// Knight + Pitchfork
const float KNIGHT_HEALTH = 30.f;
const float KNIGHT_COOLDOWN = 4000.f;
const float KNIGHT_VELOCITY = 0.04f;
const float KNIGHT_RANGE = 200.f;
const float KNIGHT_DAMAGE = 5.f;
const float PITCHFORK_VELOCITY = 0.25f;
const float PITCHFORK_DAMAGE = 10.f;

// Archer + Arrow
const float ARCHER_HEALTH = 50.f;
const float ARCHER_COOLDOWN = 2500.f;
const float ARCHER_VELOCITY = 0.03f;
const float ARCHER_RANGE = 400.f;
const float ARCHER_DAMAGE = 5.f;
const float ARROW_VELOCITY = 0.35f;
const float ARROW_DAMAGE = 20.f;

// Paladin + Sword
const float PALADIN_HEALTH = 200.f;
const float PALADIN_COOLDOWN = 3000.f;
const float PALADIN_VELOCITY = 0.025f;
const float PALADIN_RANGE = 35.f; // Melee range
const float PALADIN_DAMAGE = 15.f;
const float SWORD_VELOCITY = 0.35f;
const float SWORD_DAMAGE = 35.f;

// --- World Interactables ---
enum class InteractableType
{
    HEALER,
    POWER
};

// --- Interactable Timers ---
const float POWERUP_DECAY = 30000.f;
const float POWERUP_SPAWN_TIMER = 2 * 60000.f;
const float POWERUP_SPAWN_BUFFER = 180.f; // distance from edge to spawn
const float MIN_POWERUP_DIST = 80;

// --- Offsets ---
const float HEALTH_BAR_Y_OFFSET = -30.f;
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
