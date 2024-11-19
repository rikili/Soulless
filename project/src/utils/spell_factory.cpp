#include "utils/spell_factory.hpp"

namespace SpellFactory {

  void createSpellProjectile(ECSRegistry& registry, Entity& player, SpellType spell, double x, double y) {
    Motion& player_motion = registry.motions.get(player);
    Animation& player_animation = registry.animations.get(player);

    player_animation.state = EntityState::ATTACKING;
    player_animation.frameTime = 30.f;
    player_motion.currentDirection = angleToDirection(player_motion.angle);
    player_animation.initializeAtRow((int)player_motion.currentDirection);

    switch (spell) {
    case SpellType::FIRE:
    {
      Entity spell_ent = initSpellEntity(registry, player_motion.position, player_motion.angle, vec2({ cos(player_motion.angle), sin(player_motion.angle) }));
      configureFireSpell(registry, spell_ent);
      break;
    }
    case SpellType::WATER:
    {
      Entity spell_ent = initSpellEntity(registry, player_motion.position, player_motion.angle, vec2({ cos(player_motion.angle), sin(player_motion.angle) }));
      configureWaterSpell(registry, spell_ent);
      break;
    }
    case SpellType::LIGHTNING:
    {
      Entity spell_ent = initSpellEntity(registry, { x, y }, 0.f, vec2(0.f));
      configureLightningSpell(registry, spell_ent);
      break;
    }
    case SpellType::ICE:
    {
      vec2 initial_player_pos = player_motion.position;
      float initial_player_angle = player_motion.angle;
      for (int i = -ICE_SHARD_COUNT / 2; i <= ICE_SHARD_COUNT / 2; ++i) {
        float modified_angle = initial_player_angle + glm::radians(i * ICE_DEGREE_DIFFERENCE);
        Entity spell_ent = initSpellEntity(registry, initial_player_pos, modified_angle, vec2({ cos(modified_angle), sin(modified_angle) }) * ICE_SPEED);
        configureIceSpell(registry, spell_ent);
      }
      break;
    }
    default:
      break;
    }
  }

  Entity initSpellEntity(ECSRegistry& registry, vec2 position = { 0.f, 0.f }, float angle = 0.f, vec2 velocity = { 0.f, 0.f }) {
    const Entity ent;

    registry.projectiles.emplace(ent);
    Motion& proj_motion = registry.motions.emplace(ent);
    registry.deadlies.emplace(ent);
    registry.damages.emplace(ent);
    RenderRequest& request = registry.render_requests.emplace(ent);

    proj_motion.position = position;
    proj_motion.angle = angle;
    proj_motion.velocity = velocity;

    request.mesh = "sprite";
    request.shader = "sprite";

    return ent;
  }

  void configureFireSpell(ECSRegistry& registry, Entity& spell_ent) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    spell_motion.scale = FIRE_SCALE;
    spell_motion.collider = FIRE_COLLIDER;

    projectile.type = DamageType::fire;
    projectile.range = FIRE_RANGE;

    deadly.to_enemy = true;

    damage.value = FIRE_DAMAGE;

    request.texture = "fireball";
    request.type = PROJECTILE;
  }

  void configureWaterSpell(ECSRegistry& registry, Entity& spell_ent) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    spell_motion.scale = WATER_SCALE;
    spell_motion.collider = WATER_COLLIDER;

    projectile.type = DamageType::water;
    projectile.range = WATER_RANGE;

    deadly.to_projectile = true;

    damage.value = WATER_DAMAGE;

    SpellState& spellState = registry.spellStates.emplace(spell_ent);
    spellState.state = State::ACTIVE;
    spellState.timer = WATER_LIFETIME;

    request.texture = "barrier";
    request.type = OVER_PLAYER;
  }

  void configureLightningSpell(ECSRegistry& registry, Entity& spell_ent) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    spell_motion.scale = LIGHTNING_SCALE;
    spell_motion.collider = LIGHTNING_COLLIDER;

    projectile.type = DamageType::lightning;
    projectile.range = LIGHTNING_RANGE;

    damage.value = LIGHTNING_CASTING_DAMAGE;

    SpellState& spellState = registry.spellStates.emplace(spell_ent);
    spellState.state = State::CASTING;
    spellState.timer = LIGHTNING_CASTING_LIFETIME;

    request.texture = "lightning1";
    request.type = PROJECTILE;
  }

  void configureIceSpell(ECSRegistry& registry, Entity& spell_ent) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    spell_motion.scale = ICE_SCALE;
    spell_motion.collider = ICE_COLLIDER;

    projectile.type = DamageType::ice;
    projectile.range = ICE_RANGE;

    deadly.to_enemy = true;

    damage.value = ICE_DAMAGE;

    request.texture = "ice";
    request.type = PROJECTILE;
  }

}
