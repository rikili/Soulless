#pragma once

#include <random>
#include <deque>
#include <unordered_map>
#include <utils/constants.hpp>

// hash function for unordered map of SpellType
struct SpellTypeHash {
	std::size_t operator()(const SpellType& spell) const {
		std::hash<int> hasher; 	// create hash function 
		int spellInt = static_cast<int>(spell); // convert enum to int
		return hasher(spellInt);	// return hash of int
	}
};

class SpellQueue
{
public:
	SpellQueue();
	~SpellQueue();
	void collectSpell(SpellType spell);
	SpellType useSpell(bool is_first);
	void discardSpell(bool is_first);
	const std::deque<SpellType>& getQueue() const;

private:
	void addSpell();
	SpellType getRandomSpell();

	std::deque<SpellType> queue;
	std::unordered_map<SpellType, int, SpellTypeHash> collectedSpells;
	std::mt19937 rng;
};