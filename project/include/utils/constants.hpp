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
const float DARKLORD_SPAWN_INTERVAL_MS = 270000.f;

const unsigned int QUEUE_SIZE = 6;

constexpr float LOW_HEALTH_THRESHOLD = 0.3f;
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
const float PLAYER_VELOCITY = 0.4f * zoomFactor;

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
    COUNT // Used to track how many spell types we have
};
const int NOT_DROPPED_SPELL_COUNT = 2;

const float FIRE_DAMAGE = 15.f;
const float FIRE_VELOCITY = 2.f * zoomFactor;
const float FIRE_RANGE = 600.f * zoomFactor;
const vec2 FIRE_SCALE = { 0.3, 0.3 };
const float FIRE_SCALE_FACTOR = 3.f;
const vec2 FIRE_COLLIDER = { 100 * zoomFactor, 100 * zoomFactor };

const float WATER_DAMAGE = 0.f;
const float WATER_VELOCITY = 0.f;
const float WATER_RANGE = FLT_MAX; // Range is "infinite" for barrier
const vec2 WATER_SCALE = { 0.6f, 0.6f };
// const float WATER_SCALE_FACTOR = 1.f;
const vec2 WATER_COLLIDER = { 120.f * zoomFactor, 120.f * zoomFactor };
const float WATER_LIFETIME = 1000.f; // Barrier spell lasts for 1 second (or if it collides with enemy projectile)

const float LIGHTNING_CASTING_DAMAGE = 0.f;
const float LIGHTNING_ACTIVE_DAMAGE = 35.f;
const float LIGHTNING_VELOCITY = 0.f;
const float LIGHTNING_RANGE = FLT_MAX; // Range is "infinite" for lightning
const vec2 LIGHTNING_SCALE = { 0.75f, 0.75f };
const float LIGHTNING_SCALE_FACTOR = 1.f;
const vec2 LIGHTNING_COLLIDER = { 150.f * zoomFactor, 80.f * zoomFactor };
const float LIGHTNING_CASTING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_CHARGING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_ACTIVE_LIFETIME = 250.f; // Lightning bolt lasts 0.25 seconds before disappearing

const float ICE_DAMAGE = 10.f;
const float ICE_SPEED = 1.6f * zoomFactor;
const float ICE_RANGE = 300.f * zoomFactor;
const vec2 ICE_SCALE = { 0.2, 0.2 };
const vec2 ICE_COLLIDER = { 26.f * zoomFactor, 26.f * zoomFactor };
const int ICE_SHARD_COUNT = 5;
const float ICE_DEGREE_DIFFERENCE = 10.f;

const float WIND_DAMAGE = 4.f;
const float WIND_RANGE = FLT_MAX;
const vec2 WIND_SCALE = { 0.75f, 0.75f };
const float WIND_SCALE_FACTOR = 1.f;
const vec2 WIND_COLLIDER = { 160.f * zoomFactor, 160.f * zoomFactor };
const float WIND_PLACEMENT_LIFETIME = 10000.f;

const float PLASMA_DAMAGE = 25.f;
const float PLASMA_SPEED = 0.01f;
const float PLASMA_MAX_SPEED = 2.0f * zoomFactor;
const float PLASMA_RANGE = 800.f * zoomFactor;
const vec2 PLASMA_SCALE = { 0.4f, 0.4f };
const vec2 PLASMA_COLLIDER = { 80.f * zoomFactor, 80.f * zoomFactor };

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
const float KNIGHT_VELOCITY = 0.08f * zoomFactor;
const float KNIGHT_RANGE = 400.f * zoomFactor;
const float KNIGHT_DAMAGE = 5.f;
const float PITCHFORK_VELOCITY = 0.5f * zoomFactor;
const float PITCHFORK_DAMAGE = 10.f;

// Archer + Arrow
const float ARCHER_HEALTH = 50.f;
const float ARCHER_COOLDOWN = 2500.f;
const float ARCHER_VELOCITY = 0.06f * zoomFactor;
const float ARCHER_RANGE = 800.f * zoomFactor;
const float ARCHER_DAMAGE = 5.f;
const float ARROW_VELOCITY = 0.7f * zoomFactor;
const float ARROW_DAMAGE = 20.f;

// Paladin + Sword
const float PALADIN_HEALTH = 100.f;
const float PALADIN_COOLDOWN = 3000.f;
const float PALADIN_VELOCITY = 0.05f * zoomFactor;
const float PALADIN_RANGE = 70.f * zoomFactor; // Melee range
const float PALADIN_DAMAGE = 15.f;
const float SWORD_VELOCITY = 0.7f * zoomFactor;
const float SWORD_DAMAGE = 35.f;

// Slasher
const float SLASHER_HEALTH = 25.f;
const float SLASHER_COOLDOWN = 750.f;
const float SLASHER_VELOCITY = 0.3f * zoomFactor;
const float SLASHER_RANGE = 200.f * zoomFactor;
const float SLASHER_DAMAGE = 20.f;

// Dark Lord + Razor Wind + Claw Pull
const float DARKLORD_HEALTH = 750.f;
const float DARKLORD_VELOCITY = 0.04f * zoomFactor;
const float DARKLORD_DAMAGE = 20.f;
const float DARKLORD_RANGE = 1000.f * zoomFactor;
const float DARKLORD_COOLDOWN = 5000.f;
// const float DARKLORD_RAZOR_COOLDOWN = 5000.f;
// const float DARKLORD_CLAW_COOLDOWN = 10000.f;
// const float DARKLORD_RAZOR_RANGE = 350.f;
// const float DARKLORD_CLAW_RANGE = 400.f;
const float DARKLORD_RAZOR_DAMAGE = 25.f;
// const float DARKLORD_CLAW_DAMAGE = 10.f;
const float DARKLORD_RAZOR_SPEED = 0.2f * zoomFactor;
const float DARKLORD_RAZOR_MAX_SPEED = 3.0f * zoomFactor;
// const float DARKLORD_CLAW_VELOCITY = 0.3f;

// --- World Interactables ---
enum class InteractableType
{
    HEALER,
    POWER
};

// --- Interactable Timers ---
const float POWERUP_DECAY = 30000.f;
const float POWERUP_SPAWN_TIMER =  35000.f;
const float POWERUP_SPAWN_BUFFER = 180.f; // distance from edge to spawn
const float MIN_POWERUP_DIST = 80;

// --- Offsets ---
const float HEALTH_BAR_Y_OFFSET = -60.f * zoomFactor;
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
