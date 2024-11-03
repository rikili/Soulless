#include <random>
#include <deque>
#include <unordered_map>
#include "utils/constants.hpp"
#include "utils/spell_queue.hpp"


SpellQueue::SpellQueue() {
  // init random number generator
  std::random_device rd;
  rng = std::mt19937(rd());

  // init have collected zero spells
  for (int i = 0; i < static_cast<int>(SpellType::COUNT); i++) {
    SpellType spell = static_cast<SpellType>(i);
    collectedSpells[spell] = 0;
  }

  // player starts with 1 fire spell + 1 water spell
  collectedSpells[SpellType::FIRE] = 1;
  collectedSpells[SpellType::WATER] = 1;

  for (int i = 0; i < QUEUE_SIZE; i++) {
    addSpell();
  }
}

SpellQueue::~SpellQueue() {}

/**
 * Increments count of collected spell. Does not add the spell to the queue.
 *
 * @param spell Type of spell to be collected
 */
void SpellQueue::collectSpell(SpellType spell) {
  collectedSpells[spell]++;
};


/**
 * Uses the first or second spell in the queue and removes it from the queue.
 * Adds a random spell to the queue.
 *
 * @param is_first Whether to use the first (left click) or second (right click) spell from the queue
 * @return Type of spell used
 */
SpellType SpellQueue::useSpell(bool is_first) {
  SpellType spell;
  if (is_first) {
    // use first spell
    spell = queue.front();
    queue.pop_front();
  }
  else {
    std::deque<SpellType>::iterator it = queue.begin();
    it++; // move iterator to second item
    spell = *it;
    queue.erase(it);
  }
  addSpell();
  return spell;
}

/**
 * Discards the first or second spell in the queue.
 * Adds a random spell to the queue.
 *
 * @param is_first Whether to discard the first or second spell from the queue
 */
void SpellQueue::discardSpell(bool is_first) {
  if (is_first) {
    queue.pop_front();
  }
  else {
    std::deque<SpellType>::iterator it = queue.begin();
    it++; // move iterator to second item
    queue.erase(it);
  }
  addSpell();
}

/**
 * Get the current queue of spells.
 *
 * @return Queue of spells
 */
const std::deque<SpellType>& SpellQueue::getQueue() const {
  return this->queue;
};

/**
 * Add a random spell to the queue.
 */
void SpellQueue::addSpell() {
  SpellType spell = getRandomSpell();
  this->queue.push_back(spell);
}

/**
 * Get a random spell from the collected spells.
 * It uses a discrete distribution where each spell's weight is the number of times it has been collected divided by total number of collect spells.
 *
 * @return Random type of spell from the collected spells
 */
SpellType SpellQueue::getRandomSpell() {
  // create discrete distribution for random spell selection
  std::vector<SpellType> spells;
  std::vector<int> weights;
  for (const auto& pair : collectedSpells) {
    spells.push_back(pair.first);
    weights.push_back(pair.second);
    // printf("Spell: %d, Weight: %d\n", pair.first, pair.second);
  }

  std::discrete_distribution<> dist(weights.begin(), weights.end());

  // generate random spell index, return which spell is selected
  int index = dist(rng);
  return spells[index];
}
