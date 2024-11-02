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
    ice,
    lightning,
    water,
    plasma,
    wind,
    enemy,
    elementless
};

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

// --- Player Constants ---
const float PLAYER_HEALTH = 100.f;
const float PLAYER_MAX_HEALTH = 100.f;

// --- Player Spells ---

const float FIRE_DAMAGE = 25.f;
const float FIRE_VELOCITY = 1.f;
const float FIRE_RANGE = 250.f;
const vec2 FIRE_SCALE = { 0.2, 0.2 };
const float FIRE_SCALE_FACTOR = 3.f;
const vec2 FIRE_COLLIDER = { 150, 150 };

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
    PLAYER = 3
};
