#pragma once

#include <unordered_set>
#include "core/common.hpp"
#include "entities/ecs_registry.hpp"
#include "utils/spell_factory.hpp"

#include "isystems/IInputHandler.hpp"

class InputHandler : public IInputHandler {
public:
	InputHandler();

	void onKey(int key, int scancode, int action, int mods);

	void onMouseMove(vec2 mouse_position);

	void onMouseKey(GLFWwindow* window, int button, int action, int mods);

	void reset();

	void setRenderer(IRenderSystem* renderer) override;

private:
	// keeps track of pressed movement keys (up, right, left, down) 
	std::unordered_set<int> activeMoveKeys;
	vec2 worldMousePosition;

	IRenderSystem* renderer;

	void updateVelocity();
	void cast_player_spell(double x, double y, bool is_left);
	void drop_player_spell(bool is_left);
	void invoke_player_cooldown(Player& player, bool is_left, bool is_heal);
};
