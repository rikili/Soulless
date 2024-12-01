#pragma once

#include "ecs.hpp"
#include <unordered_set>

class CollisionRegistry
{
private:
	std::unordered_map<Entity, std::unordered_set<Entity>> collision_mapping;
	bool exists_in_both(const std::pair<Entity, Entity>& pairing);

	void print_collision_map();

public:
	CollisionRegistry() {}
	~CollisionRegistry() {
		for (auto& collision : collision_mapping)
		{
			collision.second.clear();
		}
		collision_mapping.clear();
	};
	void register_collision(const Entity& entity, const Entity& other_entity);
	std::unordered_set<Entity> get_collision_by_ent(const Entity& entity);
	bool check_collision(const Entity& entity, const Entity& other_entity);
	void remove_collision(const Entity& entity, const Entity& other_entity);
	void clear_collisions();
};