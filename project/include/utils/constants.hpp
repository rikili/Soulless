#pragma once

// Game Logic Constants
const float ENEMY_SPAWN_INTERVAL_MS = 5000.f;
const unsigned int QUEUE_SIZE = 8;

// Spell/Projectile Constants

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

// ENEMY
const float ENEMY_BASIC_VELOCITY = 0.05f;
const float ENEMY_BASIC_RANGE = 100.f;

// FIRE
const float FIRE_DAMAGE = 25.f;
const float FIRE_VELOCITY = 1.f;
const float FIRE_RANGE = 250.f;

// ICE
const float ICE_VELOCITY = 1.f;
const float ICE_DEGREE_DIFFERENCE = 15.f;
const int ICE_SHARD_COUNT = 4;

// LIGHTNING
// PLASMA
// WATER
// WIND

// Draw order (largest number = frontmost)
enum {
    BACK = 0,
    PROJECTILE = 1,
    ENEMY = 2,
    PLAYER = 3
};
