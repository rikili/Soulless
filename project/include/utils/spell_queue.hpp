#pragma once

#include <random>
#include <utils/constants.hpp>

class SpellQueue
{
private:
	unsigned int index = 0;
	std::array<DamageType, QUEUE_SIZE> queue;

public:
	SpellQueue()
	{
		// TODO
	}

	~SpellQueue()
	{
		// TODO
	}

	DamageType dequeue()
	{
		// TODO: Implement + remove placeholder

		// Placeholder return
		return DamageType::fire;
	}

	const std::array<DamageType, QUEUE_SIZE>& getQueue() const
	{
		// TODO: Implement + remove placeholder

		// Placeholder return
		return queue;
	}
};