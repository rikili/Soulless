#pragma once
#include <vector>

#include "ecs.hpp"
#include "general_components.hpp"
#include "collision_registry.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<Motion> motions;
	ComponentContainer<Health> healths;
	ComponentContainer<HealthBar> healthBars;
	ComponentContainer<ResistanceModifier> resistanceModifiers;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Timed> timeds;
	ComponentContainer<Player> players;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Deadly> deadlies;
	ComponentContainer<Death> deaths;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Interactable> interactables;
	ComponentContainer<Damage> damages;
	ComponentContainer<OnHit> onHits;
	ComponentContainer<OnHeal> onHeals;
	ComponentContainer<RenderRequest> render_requests;
	ComponentContainer<RenderRequest> static_render_requests;
	ComponentContainer<Animation> animations;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Tile> tiles;
	ComponentContainer<DebugRequest> debug_requests;
	ComponentContainer<MeshCollider> mesh_colliders;
	CollisionRegistry collision_registry;
  
	mat4 viewMatrix;
	mat4 projectionMatrix;

	bool game_over = false;
	bool debug = false;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&motions);
		registry_list.push_back(&healths);
		registry_list.push_back(&healthBars);
		registry_list.push_back(&resistanceModifiers);
		registry_list.push_back(&collisions);
		registry_list.push_back(&timeds);
		registry_list.push_back(&players);
		registry_list.push_back(&enemies);
		registry_list.push_back(&deadlies);
		registry_list.push_back(&deaths);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&interactables);
		registry_list.push_back(&damages);
		registry_list.push_back(&onHits);
		registry_list.push_back(&onHeals);
		registry_list.push_back(&render_requests);
		registry_list.push_back(&animations);
		registry_list.push_back(&cameras);
		registry_list.push_back(&static_render_requests);
		registry_list.push_back(&tiles);
	}

	void clear_all_components() const
	{
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() const
	{
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) const
	{
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) const
	{
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;
