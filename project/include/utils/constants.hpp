#pragma once
#include <glm/vec2.hpp>

using glm::vec2;

// --- Game Logic Constants ---
const unsigned int MAX_ENEMIES = 100;
const float FARMER_SPAWN_INTERVAL_MS = 5000.f;
const float ARCHER_SPAWN_INTERVAL_MS = 10000.f;
const float KNIGHT_SPAWN_INTERVAL_MS = 15000.f;
const unsigned int QUEUE_SIZE = 8;

const float ENEMY_INVINCIBILITY_TIMER = 800.f;
const float PLAYER_INVINCIBILITY_TIMER = 1500.f;

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

// --- Player Constants ---
const float PLAYER_HEALTH = 100.f;
const float PLAYER_MAX_HEALTH = 100.f;

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
const float FIRE_VELOCITY = 1.f;
const float FIRE_RANGE = 400.f;
const vec2 FIRE_SCALE = { 0.2, 0.2 };
const float FIRE_SCALE_FACTOR = 3.f;
const vec2 FIRE_COLLIDER = { 150, 150 };

const float WATER_DAMAGE = 0.f;
const float WATER_VELOCITY = 0.f;
const float WATER_RANGE = __FLT_MAX__; // Range is "infinite" for barrier
const vec2 WATER_SCALE = { 1.f, 1.f };
const float WATER_SCALE_FACTOR = 1.f;
const vec2 WATER_COLLIDER = { 250, 250 };
const float WATER_LIFETIME = 1000.f; // Barrier spell lasts for 1 second (or if it collides with enemy projectile)

const float LIGHTNING_CASTING_DAMAGE = 0.f;
const float LIGHTNING_ACTIVE_DAMAGE = 35.f;
const float LIGHTNING_VELOCITY = 0.f;
const float LIGHTNING_RANGE = __FLT_MAX__; // Range is "infinite" for lightning
const vec2 LIGHTNING_SCALE = { 0.75f, 0.75f };
const float LIGHTNING_SCALE_FACTOR = 1.f;
const vec2 LIGHTNING_COLLIDER = { 150, 150 };
const float LIGHTNING_CASTING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_CHARGING_LIFETIME = 500.f; // Lightning cast lasts 0.5 seconds before changing state
const float LIGHTNING_ACTIVE_LIFETIME = 250.f; // Lightning bolt lasts 0.25 seconds before disappearing

const float ICE_VELOCITY = 1.f;
const float ICE_DEGREE_DIFFERENCE = 15.f;
const int ICE_SHARD_COUNT = 4;

// LIGHTNING
// PLASMA
// WATER
// WIND

// --- Enemy Types ---
enum class EnemyType
{
    FARMER,
    ARCHER,
    KNIGHT
};

// --- Enemy Constants ---
const float ENEMY_BASIC_VELOCITY = 0.05f;
const float ENEMY_BASIC_RANGE = 100.f;

// Farmer + Pitchfork
const float FARMER_HEALTH = 25.f;
const float FARMER_COOLDOWN = 1000.f;
const float FARMER_VELOCITY = 0.10f;
const float FARMER_RANGE = 200.f;
const float FARMER_DAMAGE = 5.f;
const float PITCHFORK_VELOCITY = 0.25f;
const float PITCHFORK_DAMAGE = 10.f;

// Archer + Arrow
const float ARCHER_HEALTH = 50.f;
const float ARCHER_COOLDOWN = 2500.f;
const float ARCHER_VELOCITY = 0.07f;
const float ARCHER_RANGE = 500.f;
const float ARCHER_DAMAGE = 5.f;
const float ARROW_VELOCITY = 0.35f;
const float ARROW_DAMAGE = 20.f;

// Knight + Sword
const float KNIGHT_HEALTH = 200.f;
const float KNIGHT_COOLDOWN = 3000.f;
const float KNIGHT_VELOCITY = 0.05f;
const float KNIGHT_RANGE = 35.f; // Melee range
const float KNIGHT_DAMAGE = 15.f;
const float SWORD_VELOCITY = 0.35f;
const float SWORD_DAMAGE = 35.f;

// Draw order (largest number = frontmost)
enum
{
    BACK = 0,
    PROJECTILE = 1,
    ENEMY = 2,
    PLAYER = 3,
    OVER_PLAYER = 4,
};
