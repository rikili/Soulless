#pragma once
#include "render_system.hpp"


class CollisionSystem
{

public:
    explicit CollisionSystem(RenderSystem* renderer);
    void init();
    void handle_collisions();

private:
    RenderSystem* renderer;
};
