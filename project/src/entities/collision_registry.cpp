#include "entities/collision_registry.hpp"

void CollisionRegistry::print_collision_map()
{
	for (auto pairing : this->collision_mapping)
	{
		printf("key: %u\n   values:", (unsigned int)pairing.first);
		for (auto value : pairing.second)
		{
			printf("%u ", (unsigned int)value);
		}
		printf("\n");
	}
}

/*
	Build entity pairing based off first being the entity with ID lower than the second
*/
std::pair<Entity, Entity> build_pairing(const Entity& entity, const Entity& other_entity)
{
	const Entity& less_entity = entity < other_entity ? entity : other_entity;
	const Entity& more_entity = entity == less_entity ? other_entity : entity;
	return { less_entity, more_entity };
}

bool CollisionRegistry::exists_in_both(const std::pair<Entity, Entity>& pairing)
{
	return this->collision_mapping[pairing.first].find(pairing.second) != this->collision_mapping[pairing.first].end()
		&& this->collision_mapping[pairing.second].find(pairing.first) != this->collision_mapping[pairing.second].end();
}

void CollisionRegistry::register_collision(const Entity& entity, const Entity& other_entity)
{
	const auto pairing = build_pairing(entity, other_entity);
	if (exists_in_both(pairing)) return;

	this->collision_mapping[pairing.first].insert(pairing.second);
	this->collision_mapping[pairing.second].insert(pairing.first);
}

bool CollisionRegistry::check_collision(const Entity& entity, const Entity& other_entity)
{
	return exists_in_both(build_pairing(entity, other_entity));
}

void CollisionRegistry::remove_collision(const Entity& entity, const Entity& other_entity)
{
	const auto pairing = build_pairing(entity, other_entity);
	if (!exists_in_both(pairing)) return;

	this->collision_mapping[pairing.first].erase(this->collision_mapping[pairing.first].find(pairing.second));
	this->collision_mapping[pairing.second].erase(this->collision_mapping[pairing.second].find(pairing.first));
}

std::unordered_set<Entity> CollisionRegistry::get_collision_by_ent(const Entity& entity)
{
	if (this->collision_mapping.find(entity) == this->collision_mapping.end())
	{
		return std::unordered_set<Entity>();
	}
	return this->collision_mapping[entity];
}

void CollisionRegistry::clear_collisions()
{
	this->collision_mapping.clear();
}
