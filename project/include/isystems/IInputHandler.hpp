// IInputHandler.hpp
#pragma once
#include "forward_types.hpp"

class IInputHandler {
public:
    virtual ~IInputHandler() = default;

    // Input events
    virtual void onKey(
        int key,
        int scancode,
        int action,
        int mods) = 0;

    virtual void onMouseMove(vec2 mouse_position) = 0;

    virtual void onMouseKey(
        GLFWwindow* window,
        int button,
        int action,
        int mods) = 0;

    virtual void reset() = 0;
};