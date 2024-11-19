#pragma once

#include "entities/general_components.hpp"
#include "entities/ecs_registry.hpp"
#include <array>
#include <random>

class ParticleSystem {
public:
	ParticleSystem();

	static ParticleSystem* getParticleSystem();
	void emitParticle(const vec2 position, const vec2 velocity, float lifetime, float size);
	void updateParticles(float elapsed_ms);
	void particleBurst(vec2 position);

private:
	float randomFloat(float min, float max);
	static ParticleSystem* instance;
};