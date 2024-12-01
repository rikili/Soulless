#pragma once
#include "entities/ecs_registry.hpp"
#include "utils/angle_functions.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace SpellFactory {
  void createSpellProjectile(ECSRegistry& registry, Entity& player, SpellType spell, int spell_level, double x, double y, bool skip_animation = false);
  void createSpellResolution(ECSRegistry& registry, vec2& position, PostResolution resolution, Entity& source_ent);

  Entity initSpellEntity(ECSRegistry& registry, vec2 position = { 0.f, 0.f }, float angle = 0.f, vec2 velocity = { 0.f, 0.f }, int level = 1);

  void configureFireSpell(ECSRegistry& registry, Entity& spell_ent, int level);
  void activateMaxLevelWater(vec2& angle, Entity& source_ent);
  void configureWaterSpell(ECSRegistry& registry, Entity& spell_ent, int level);
  void configureLightningSpell(ECSRegistry& registry, Entity& spell_ent, int level, vec2 direction, bool is_chain);
  void configureWindSpell(ECSRegistry& registry, Entity& spell_ent, int level);
  void configureIceSpell(ECSRegistry& registry, Entity& spell_ent, int level);
  void configurePlasmaSpell(ECSRegistry& registry, Entity& spell_ent, int level);
  void configureMaxFireProjectile(ECSRegistry& registry, Entity& spell_ent);
  void configureMaxWaterExplosion(ECSRegistry& registry, Entity& spell_ent, int barrier_level);
}
