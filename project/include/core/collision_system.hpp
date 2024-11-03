#pragma once
#include "render_system.hpp"


class CollisionSystem
{

public:
    explicit CollisionSystem(RenderSystem* renderer);
    void init();
    void detect_collisions();
    void resolve_collisions();
    void applyDamage(Entity attacker, Entity victim);

private:
    RenderSystem* renderer;
    bool is_mesh_colliding(const Entity& player, const Entity& other_entity);
};
