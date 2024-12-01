#pragma once
#include "isystems/IRenderSystem.hpp"
#include "core/common.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/particle_system.hpp"



class CollisionSystem
{

public:
    explicit CollisionSystem(IRenderSystem* renderer);
    void init();
    void detect_collisions();
    void resolve_collisions();
    HitTypes applyDamage(Entity attacker, Entity victim, std::unordered_map<SpellType, int, SpellTypeHash>& tracker, bool do_scaling = false);
    void applyHealing(Entity target);
    void pickupSpell(Entity target, SpellType type);

private:
    IRenderSystem* renderer;
    ParticleSystem particleSystem;
    bool is_mesh_colliding(const Entity& player, const Entity& other_entity);
    bool isWaterProtected(const Entity& player, const Entity& attacker);
    void resolve_post_effects(const std::unordered_map<PostResolution, std::pair<Entity, std::vector<Entity>>> resolutions);
};
