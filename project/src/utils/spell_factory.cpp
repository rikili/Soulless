#include "utils/spell_factory.hpp"
#include "sound/sound_manager.hpp"

namespace SpellFactory {

  static bool handleWaterActive(vec2 angle)
  {
    for (auto& entity : registry.spellStates.entities)
    {
      SpellState& state = registry.spellStates.get(entity);
      if (state.isBarrier)
      {
        SpellProjectile& proj = registry.spellProjectiles.get(entity);
        state.timer = WATER_LIFETIME;
        if (proj.level >= MAX_SPELL_LEVEL)
        {
          activateMaxLevelWater(angle, entity);
        }
        return true;
      }
    }
    return false;
  }

  void createSpellProjectile(ECSRegistry& registry, Entity& player_ent, SpellType spell, int spell_level, double x, double y, bool skip_animation) {
    Motion& player_motion = registry.motions.get(player_ent);
    if (!skip_animation)
    {
      Animation& player_animation = registry.animations.get(player_ent);
      player_animation.state = AnimationState::ATTACKING;
      player_animation.frameTime = 30.f;
      player_motion.currentDirection = angleToDirection(find_closest_angle(player_motion.angle));
      player_animation.initializeAtRow((int)player_motion.currentDirection);

    }
    Player& player = registry.players.get(player_ent);

    switch (spell) {
    case SpellType::FIRE:
    {
      Entity spell_ent = initSpellEntity(registry, player_motion.position, player_motion.angle, vec2({ cos(player_motion.angle), sin(player_motion.angle) }) * FIRE_VELOCITY);
      configureFireSpell(registry, spell_ent, player.spell_queue.getSpellLevel(SpellType::FIRE));
      break;
    }
    case SpellType::WATER:
    {
      if (handleWaterActive(vec2({ cos(player_motion.angle), sin(player_motion.angle) }))) return;
      Entity spell_ent = initSpellEntity(registry, player_motion.position, player_motion.angle, vec2({ cos(player_motion.angle), sin(player_motion.angle) }));
      configureWaterSpell(registry, spell_ent, player.spell_queue.getSpellLevel(SpellType::WATER));
      break;
    }
    case SpellType::LIGHTNING:
    {
      bool is_chain = spell_level == -1;
      Entity spell_ent = initSpellEntity(registry, { x, y }, 0.f, vec2(0.f), is_chain ? 1 : spell_level);
      configureLightningSpell(registry, spell_ent, player.spell_queue.getSpellLevel(SpellType::LIGHTNING), vec2({ cos(player_motion.angle), sin(player_motion.angle) }), is_chain);
      break;
    }
    case SpellType::WIND:
    {
      Entity spell_ent = initSpellEntity(registry, { x, y }, 0.f, vec2(0.f));
      configureWindSpell(registry, spell_ent, player.spell_queue.getSpellLevel(SpellType::WIND));
      break;
    }
    case SpellType::ICE:
    {
      vec2 initial_player_pos = player_motion.position;
      float initial_player_angle = player_motion.angle;
      if (spell_level >= MAX_SPELL_LEVEL)
      {
        Entity spell_ent = initSpellEntity(registry, player_motion.position, player_motion.angle, vec2({ cos(player_motion.angle), sin(player_motion.angle) }) * MAX_ICE_SPEED);
        configureIceSpell(registry, spell_ent, player.spell_queue.getSpellLevel(SpellType::ICE));
      }
      else
      {
        for (int i = -ICE_SHARD_COUNT / 2; i <= ICE_SHARD_COUNT / 2; ++i) {
          float modified_angle = initial_player_angle + glm::radians(i * ICE_DEGREE_DIFFERENCE);
          Entity spell_ent = initSpellEntity(registry, initial_player_pos, modified_angle, vec2({ cos(modified_angle), sin(modified_angle) }) * ICE_SPEED);
          configureIceSpell(registry, spell_ent, player.spell_queue.getSpellLevel(SpellType::ICE));
        }
      }
      break;
    }
    case SpellType::PLASMA:
    {
      Entity spell_ent = initSpellEntity(registry, player_motion.position, player_motion.angle, vec2({ cos(player_motion.angle), sin(player_motion.angle) }) * PLASMA_SPEED);
      configurePlasmaSpell(registry, spell_ent, player.spell_queue.getSpellLevel(SpellType::PLASMA));
      break;
    }
    default:
      break;
    }

    SoundManager* soundManager = SoundManager::getSoundManager();
    soundManager->playSound(SoundManager::convertSpellToSoundEffect(spell, player.spell_queue.getSpellLevel(spell)));
  }

  void createSpellResolution(ECSRegistry& registry, vec2& position, PostResolution resolution, Entity& source_ent)
  {
    int level = registry.spellProjectiles.has(source_ent) ? clamp(registry.spellProjectiles.get(source_ent).level, 1, MAX_SPELL_LEVEL - 1) : 1;
    Entity spell_ent = initSpellEntity(registry, position, 0.f, { 0, 0 }, level);

    SoundManager* soundManager = SoundManager::getSoundManager();

    switch (resolution)
    {
    case PostResolution::FIRE_PROJECTILE:
    {
      configureMaxFireProjectile(registry, spell_ent);
      soundManager->playSound(SoundEffect::FIRE_MAX_EXPLODE);
      break;
    }
    case PostResolution::WATER_EXPLOSION:
    {
      int state_level = registry.spellStates.has(source_ent) ? clamp(registry.spellStates.get(source_ent).barrier_level, 1, 3) : 1;
      configureMaxWaterExplosion(registry, spell_ent, state_level);
      soundManager->playSound(SoundEffect::WATER_EXPLODE);
      break;
    }
    default:
      break;
    }
  }

  Entity initSpellEntity(ECSRegistry& registry, vec2 position, float angle, vec2 velocity, int level) {
    const Entity ent;

    registry.projectiles.emplace(ent);
    SpellProjectile& spell = registry.spellProjectiles.emplace(ent);
    Motion& proj_motion = registry.motions.emplace(ent);
    registry.deadlies.emplace(ent);
    registry.damages.emplace(ent);
    RenderRequest& request = registry.render_requests.emplace(ent);

    proj_motion.position = position;
    proj_motion.angle = angle;
    proj_motion.velocity = velocity;

    spell.level = level; // mainly useful for post resolution spells (not initial cast)

    request.mesh = "sprite";
    request.shader = "sprite";

    return ent;
  }

  void configureFireSpell(ECSRegistry& registry, Entity& spell_ent, int level) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    projectile.type = DamageType::fire;
    projectile.range = FIRE_RANGE;

    spell.type = SpellType::FIRE;
    spell.level = level;

    deadly.to_enemy = true;

    request.type = PROJECTILE;

    if (level >= MAX_SPELL_LEVEL)
    {
      spell_motion.scale = MAX_FIRE_SCALE;
      spell_motion.collider = MAX_FIRE_COLLIDER;
      request.texture = "fireball-max";
      damage.value = MAX_FIRE_DAMAGE_DIRECT;
      projectile.isActive = true;
    }
    else
    {
      spell_motion.scale = FIRE_SCALE;
      spell_motion.collider = FIRE_COLLIDER;
      damage.value = FIRE_DAMAGE;
      request.texture = "fireball";
      projectile.isActive = true;
    }
  }

  void activateMaxLevelWater(vec2& angle, Entity& source_ent)
  {
    Motion& motion = registry.motions.get(registry.players.entities[0]);
    float x_offset = motion.collider.x * motion.scale.x;
    float y_offset = motion.collider.y * motion.scale.y;
    motion.position = glm::clamp(motion.position + 100.f * angle, { x_offset, y_offset }, { window_width_px - x_offset, window_height_px - y_offset });
    createSpellResolution(registry, motion.position, PostResolution::WATER_EXPLOSION, source_ent);
  }

  void configureWaterSpell(ECSRegistry& registry, Entity& spell_ent, int level) {

    // reset player's onHits
    registry.onHits.remove(registry.players.entities[0]);

    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    spell_motion.scale = WATER_SCALE;
    spell_motion.collider = WATER_COLLIDER;

    projectile.type = DamageType::water;
    projectile.range = WATER_RANGE;

    spell.type = SpellType::WATER;
    spell.level = level;

    deadly.to_projectile = true;

    damage.value = WATER_DAMAGE;

    SpellState& spellState = registry.spellStates.emplace(spell_ent);
    spellState.state = State::ACTIVE;
    spellState.timer = WATER_LIFETIME;
    spellState.isBarrier = true;
    spellState.barrier_level = 1;
    request.texture = "barrier-1";
    request.type = OVER_PLAYER;

    if (level >= MAX_SPELL_LEVEL)
    {
      activateMaxLevelWater(spell_motion.velocity, spell_ent);
    }

  }

  void configureLightningSpell(ECSRegistry& registry, Entity& spell_ent, int level, vec2 direction, bool is_chain) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);
    SpellState& spellState = registry.spellStates.emplace(spell_ent);

    spell_motion.scale = LIGHTNING_SCALE;
    spell_motion.collider = LIGHTNING_COLLIDER;

    projectile.type = DamageType::lightning;
    projectile.range = LIGHTNING_RANGE;

    spell.type = SpellType::LIGHTNING;
    damage.value = LIGHTNING_ACTIVE_DAMAGE;

    spellState.state = State::CASTING;

    if (is_chain)
    {
      std::random_device rand;
      std::mt19937 gen(rand());

      std::uniform_real_distribution<float> time_mod(MAX_LIGHTNING_DELAY_DIFFERENCE.x, MAX_LIGHTNING_DELAY_DIFFERENCE.y);
      spellState.timer = LIGHTNING_CASTING_LIFETIME + time_mod(gen);
      damage.value = MAX_LIGHTNING_DAMAGE;
      spellState.isChild = true;
      spell.level = MAX_SPELL_LEVEL;
    }
    else
    {
        spell.level = level;
        spellState.isChild = false;
        spellState.timer = LIGHTNING_CASTING_LIFETIME;
    }

    request.texture = "lightning1";
    request.type = PROJECTILE;
  }

  void configureWindSpell(ECSRegistry& registry, Entity& spell_ent, int level) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Decay& decay = registry.decays.emplace(spell_ent);

    decay.timer = WIND_PLACEMENT_LIFETIME;
    spell_motion.scale = WIND_SCALE;
    spell_motion.collider = WIND_COLLIDER;

    projectile.type = DamageType::wind;
    projectile.range = WIND_RANGE;
    projectile.isActive = true;

    spell.type = SpellType::WIND;
    spell.level = level;

    Animation& animation = registry.animations.emplace(spell_ent);
    animation.frameTime = 80.f;

    deadly.to_enemy = true;
    damage.type = DamageType::wind;
    damage.value = WIND_DAMAGE;

    request.texture = "wind";
    request.shader = "animatedsprite";
    request.type = PROJECTILE;

    if (level >= MAX_SPELL_LEVEL) {
      request.texture = "wind-max";
      spell_motion.scale *= 1.5;
      spell_motion.collider *= 1.5;
    }
  }

  void configureIceSpell(ECSRegistry& registry, Entity& spell_ent, int level) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    spell_motion.scale = ICE_SCALE;
    spell_motion.collider = ICE_COLLIDER;

    projectile.type = DamageType::ice;
    projectile.range = ICE_RANGE;
    projectile.isActive = true;

    spell.type = SpellType::ICE;
    spell.level = level;

    deadly.to_enemy = true;

    damage.value = ICE_DAMAGE;

    if (level >= MAX_SPELL_LEVEL)
    {
      spell_motion.collider = MAX_ICE_COLLIDER;
      spell_motion.scale = MAX_ICE_SCALE;
      projectile.range = MAX_ICE_RANGE;
      damage.value = MAX_ICE_DAMAGE;
    }

    request.texture = "ice";
    request.type = PROJECTILE;
  }

  void configurePlasmaSpell(ECSRegistry& registry, Entity& spell_ent, int level) {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);

    spell_motion.scale = PLASMA_SCALE;
    spell_motion.collider = PLASMA_COLLIDER;

    projectile.type = DamageType::plasma;
    projectile.range = PLASMA_RANGE;
    projectile.isActive = true;

    spell.type = SpellType::PLASMA;

    deadly.to_enemy = true;

    damage.value = PLASMA_DAMAGE;
    damage.type = DamageType::plasma;

    request.texture = "plasma";
    request.type = PROJECTILE;
  }

  void configureMaxFireProjectile(ECSRegistry& registry, Entity& spell_ent)
  {
    Motion& spell_motion = registry.motions.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);
    Decay& decay = registry.decays.emplace(spell_ent);

    spell_motion.collider = MAX_FIRE_SPLASH_COLLIDER;
    spell_motion.scale = MAX_FIRE_SPLASH_SCALE;
    damage.value = MAX_FIRE_DAMAGE_SPLASH;
    spell.type = SpellType::FIRE;
    spell.isPostAttack = true;
    projectile.isActive = true;
    projectile.range = MAX_FIRE_SPLASH_RANGE;
    spell.level = MAX_SPELL_LEVEL;
    deadly.to_enemy = true;
    decay.timer = FIRE_SPLASH_LIFETIME;

    request.texture = "fire-post";
    request.type = PROJECTILE;
  }
  void configureMaxWaterExplosion(ECSRegistry& registry, Entity& spell_ent, int barrier_level)
  {

    Motion& spell_motion = registry.motions.get(spell_ent);
    Damage& damage = registry.damages.get(spell_ent);
    Deadly& deadly = registry.deadlies.get(spell_ent);
    SpellProjectile& spell = registry.spellProjectiles.get(spell_ent);
    Projectile& projectile = registry.projectiles.get(spell_ent);
    RenderRequest& request = registry.render_requests.get(spell_ent);
    Decay& decay = registry.decays.emplace(spell_ent);

    spell.type = SpellType::WATER;
    spell.isPostAttack = true;

    spell_motion.collider = WATER_EXPLOSION_COLLIDER;
    spell_motion.scale = WATER_EXPLOSION_SCALE;
    damage.value = WATER_DAMAGE * WATER_ABSORB_DMG_BOOST[barrier_level - 1];
    projectile.isActive = true;
    projectile.range = WATER_SPLASH_RANGE;
    decay.timer = WATER_SPLASH_LIFETIME;
    deadly.to_enemy = true;

    request.texture = "water-post";
    request.type = PROJECTILE;
  }
}
