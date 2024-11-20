#pragma once
#include "entities/ecs_registry.hpp"
#include "utils/angle_functions.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace SpellFactory {
  void createSpellProjectile(ECSRegistry& registry, Entity& player, SpellType spell, double x, double y);

  Entity initSpellEntity(ECSRegistry& registry, vec2 position, float angle, vec2 velocity);

  void configureFireSpell(ECSRegistry& registry, Entity& spell_ent);
  void configureWaterSpell(ECSRegistry& registry, Entity& spell_ent);
  void configureLightningSpell(ECSRegistry& registry, Entity& spell_ent);
  void configureWindSpell(ECSRegistry& registry, Entity& spell_ent);
  void configureIceSpell(ECSRegistry& registry, Entity& spell_ent);
  void configurePlasmaSpell(ECSRegistry& registry, Entity& spell_ent);
}
