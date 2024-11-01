#pragma once

#include <unordered_set>
#include "core/common.hpp"
#include "entities/ecs_registry.hpp"

class InputHandler {
public:
	InputHandler();

	void onKey(int key, int scancode, int action, int mods);

	void onMouseMove(vec2 mouse_position);

	void onMouseKey(GLFWwindow* window, int button, int action, int mods);

private:
	// keeps track of pressed movement keys (up, right, left, down) 
	std::unordered_set<int> activeMoveKeys;

	void updateVelocity();

	void cast_player_spell(double x, double y, bool is_left);
	void create_player_projectile(Entity& player_ent, double x, double y, SpellType spell);
	void invoke_player_cooldown(Player& player, bool is_left);
};
