#pragma once

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

// NOTE: Need to add `const` for global constants
    // For non-const global vars, use `extern` to declare in a .hpp and define in a .cpp
const unsigned int QUEUE_SIZE = 8;

// Spell/Projectile Constants

// RANGED ENEMY
const float RANGED_BASIC_VELOCITY = 1.f;

// FIRE
const float FIRE_VELOCITY = 1.f;

// ICE
const float ICE_VELOCITY = 1.f;
const float ICE_DEGREE_DIFFERENCE = 15.f;
const int ICE_SHARD_COUNT = 4;

// LIGHTNING
// PLASMA
// WATER
// WIND
