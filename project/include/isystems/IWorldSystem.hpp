// IWorldSystem.hpp
#pragma once
#include "forward_types.hpp"
#include <glm/vec2.hpp>

class Entity;
enum class EnemyType;
using vec2 = glm::vec2;

class IWorldSystem {
public:
    virtual ~IWorldSystem() = default;

    // Core system functions
    virtual bool step(float elapsed_ms) = 0;
    virtual bool isOver() const = 0;
    virtual void initialize() = 0;
    virtual void restartGame() = 0;
    virtual void reloadGame() = 0;

    virtual void handleAI(float elapsed_ms) = 0;
    virtual void handleProjectiles(float elapsed_ms) = 0;
    virtual void handleTimers(float elapsed_ms) = 0;
    virtual void handleSpellStates(float elapsed_ms) = 0;
    virtual void handleMovements(float elapsed_ms) = 0;
    virtual void handleHealthBars() = 0;
    virtual void handleAnimations() = 0;

    virtual Entity getPlayer() const = 0;
    virtual void setRenderer(IRenderSystem* renderer) = 0;
};