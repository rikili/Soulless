#pragma once

#include "entities/ecs.hpp"
#include "entities/ecs_registry.hpp"

// Sorts draw order of render requests
struct
{
	bool operator()(Entity a, Entity b) const {
		return registry.render_requests.get(a).type < registry.render_requests.get(b).type;
	}
}
typeAscending;