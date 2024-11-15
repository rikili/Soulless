#pragma once
#include "isystems/IRenderSystem.hpp"
#include "core/common.hpp"
#include <glm/gtc/matrix_transform.hpp>



class CollisionSystem
{

public:
    explicit CollisionSystem(IRenderSystem* renderer);
    void init();
    void detect_collisions();
    void resolve_collisions();
    void applyDamage(Entity attacker, Entity victim);
    void applyHealing(Entity target);

private:
    IRenderSystem* renderer;
    bool is_mesh_colliding(const Entity& player, const Entity& other_entity);
};
