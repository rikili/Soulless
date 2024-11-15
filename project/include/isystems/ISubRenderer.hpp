#pragma once
#include "forward_types.hpp"

class ISubRenderer {
    public:
        virtual ~ISubRenderer() = default;
        virtual void render(IRenderSystem* render_system) = 0;

};