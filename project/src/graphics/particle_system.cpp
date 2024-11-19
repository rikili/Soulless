#include "graphics/particle_system.hpp"

ParticleSystem* ParticleSystem::instance = nullptr;
ParticleSystem::ParticleSystem() {}

ParticleSystem* ParticleSystem::getParticleSystem() {
	if (instance == nullptr) {
		instance = new ParticleSystem();
	}
	return instance;
}

void ParticleSystem::emitParticle(const vec2 position, const vec2 velocity, float lifetime, float size) {

	if (registry.particles.size() < MAX_PARTICLES) {
		auto ent = Entity();
		Particle& particle = registry.particles.emplace(ent);
		particle.position = position;
		particle.velocity = velocity;
		particle.lifetime = lifetime;
		particle.size = size;
	}
	else {
		printd("Maximum number of particles reached!\n");
	}
}

void ParticleSystem::updateParticles(float elapsed_ms) {
	for (Entity e : registry.particles.entities) {
		Particle& particle = registry.particles.get(e);

		particle.position += particle.velocity * elapsed_ms;
		particle.lifetime -= elapsed_ms;

		if (particle.lifetime <= 0.0f) {
			registry.remove_all_components_of(e);
		}
	}
}

float ParticleSystem::randomFloat(float min, float max) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distr(min, max);

	return distr(gen);
}

void ParticleSystem::particleBurst(vec2 position) {
	for (int i = 0; i < 30; i++) {
		vec2 velocity = vec2(randomFloat(-0.1, 0.1), randomFloat(-0.1, 0.1));

		emitParticle(position, velocity, 400, 5);
	}
}