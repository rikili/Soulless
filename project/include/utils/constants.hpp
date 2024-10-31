#pragma once

// --- Game Logic Constants ---
const unsigned int MAX_ENEMIES = 100;
const float ENEMY_SPAWN_INTERVAL_MS = 5000.f;
const unsigned int QUEUE_SIZE = 8;

const float ENEMY_INVINCIBILITY_TIMER = 800.f;
const float PLAYER_INVINCIBILITY_TIMER = 1500.f;

// --- Damage Types ---
enum class DamageType
{
    fire,
    ice,
    lightning,
    water,
    plasma,
    wind,
    enemy,
    elementless
};

// --- Player Constants ---
const float PLAYER_HEALTH = 100.f;
const float PLAYER_MAX_HEALTH = 100.f;

// --- Player Spells ---

const float FIRE_DAMAGE = 25.f;
const float FIRE_VELOCITY = 1.f;
const float FIRE_RANGE = 400.f;
const vec2 FIRE_SCALE = {0.2, 0.2};
const float FIRE_SCALE_FACTOR = 3.f;
const vec2 FIRE_COLLIDER = {150, 150};

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
    CLERIC
};

// --- Enemy Constants ---
const float ENEMY_BASIC_VELOCITY = 0.05f;
const float ENEMY_BASIC_RANGE = 100.f;

// Farmer + Pitchfork
const float FARMER_HEALTH = 100.f;
const float FARMER_COOLDOWN = 1000.f;
const float FARMER_VELOCITY = 0.05f;
const float FARMER_RANGE = 200.f;
const float FARMER_DAMAGE = 5.f;
const float PITCHFORK_VELOCITY = 0.25f;
const float PITCHFORK_DAMAGE = 10.f;

// Draw order (largest number = frontmost)
enum
{
    BACK = 0,
    PROJECTILE = 1,
    ENEMY = 2,
    PLAYER = 3
};
