#pragma once

#include "core/common.hpp"

// Motion Component
struct Motion {
    vec2 position = { 0, 0 };
    vec2 velocity = { 0, 0 };
    vec2 scale = { 1, 1 };
    float mass = 0;
    float speedModifier = 1.f;
};

// Resistance Modifier
struct ResistanceModifier {};


// Health Component
struct Health
{
    float health = 0;
    float maxHealth = 0;
    ResistanceModifier resistance_modifier;
};

// Structure to store collision information
struct Collision
{
    // Note, the first object is stored in the ECS container.entities
    Entity other{}; // the second object involved in the collision
    explicit Collision(Entity& other) { this->other = other; };
};




