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
    upgradeTracker[spell] = 0;
  }

  collectedSpells[SpellType::FIRE] = 1;

  // Uncomment to add all spells to start
  // collectedSpells[SpellType::WIND] = 1;
  // collectedSpells[SpellType::WATER] = MAX_SPELL_LEVEL;
  // collectedSpells[SpellType::LIGHTNING] = 1;
   //collectedSpells[SpellType::ICE] = 2;
  // collectedSpells[SpellType::PLASMA] = 1;

  for (int i = 0; i < QUEUE_SIZE; i++) {
    addSpell();
  }

  firstSpell = getRandomSpell();
  secondSpell = getRandomSpell();
}

SpellQueue::~SpellQueue() {
    this->upgradeTracker.clear();
}

/**
 * Increments count of collected spell. Does not add the spell to the queue.
 *
 * @param spell Type of spell to be collected
 */
void SpellQueue::collectSpell(SpellType spell) {
    levelSpell(spell);
};


/**
 * Uses the first or second spell in the queue and removes it from the queue.
 * Adds a random spell to the queue.
 *
 * @param is_first Whether to use the first (left click) or second (right click) spell from the queue
 * @return Type of spell used
 */
std::pair<SpellType, int> SpellQueue::useSpell(bool is_first) {
  SpellType new_spell = queue.front();
  queue.pop_front();
  addSpell(); // update queue

  SpellType used_spell;
  if (is_first) {
    used_spell = firstSpell;
    firstSpell = new_spell;
  }
  else {
    used_spell = secondSpell;
    secondSpell = new_spell;
  }


  return std::pair<SpellType, int>(used_spell, collectedSpells[used_spell]);
}

/**
 * Replaces players first (left) or second (right) spell.
 * Adds a random spell to the queue.
 *
 * @param is_first Whether to discard the first or second spell from the queue
 */
void SpellQueue::discardSpell(bool is_first) {
  SpellType new_spell = queue.front();
  queue.pop_front();
  addSpell(); // update queue

  if (is_first) {
    firstSpell = new_spell;
  }
  else {
    secondSpell = new_spell;
  }
}

/**
 * Get the current queue of spells.
 *
 * @return Queue of spells
 */
const std::deque<SpellType>& SpellQueue::getQueue() const {
  return this->queue;
};

void SpellQueue::unlockSpell(SpellType type) {
    levelSpell(type);
}

const std::vector<SpellType> SpellQueue::getMissingSpells() {
  std::vector<SpellType> ret;
  for (auto& pair : collectedSpells)
  {
    if (!pair.second) ret.push_back(pair.first);
  }
  return ret;
}

const std::vector<std::pair<SpellType, int>> SpellQueue::getCollectedSpells()
{
    std::vector<std::pair<SpellType, int>> ret;
    for (auto& pair : collectedSpells)
    {
        if (pair.second) ret.push_back({pair.first, pair.second});
    }
    return ret;
}

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

/**
 * Replace a specified spell in the queue
 */
void SpellQueue::replaceSpell(int position, SpellType spell) {
  if (position >= 0 && position < static_cast<int>(queue.size())) {
    queue[position] = spell;
  }
}

void SpellQueue::levelSpell(SpellType spell)
{
    if (spell == SpellType::PLASMA) return;
    this->collectedSpells[spell]++;
}

void SpellQueue::addProgressSpell(SpellType spell, int count)
{
    if (collectedSpells[spell] >= MAX_SPELL_LEVEL) return;

    if (UPGRADE_KILL_COUNT[collectedSpells[spell] - 1] <= upgradeTracker[spell] + count)
    {
        levelSpell(spell);
        if (collectedSpells[spell] >= MAX_SPELL_LEVEL) upgradeTracker[spell] = UPGRADE_KILL_COUNT[MAX_SPELL_LEVEL - 1];
    }
    else
    {
        upgradeTracker[spell] += count;
    }
}

bool SpellQueue::isAbleToSacrifice()
{
    int total_levels = 0;
    for (int x = 0; x <= static_cast<int>(SpellType::COUNT) - 1 - NOT_DROPPED_SPELL_COUNT; x++)
    {
        SpellType type = static_cast<SpellType>(x);
        if (collectedSpells[type] > 1)
        {
            total_levels += collectedSpells[type] - 1;
        }
    }

    return total_levels >= PLASMA_SACRIFICE_COST;
}

void SpellQueue::doPlasmaSacrifice()
{
    std::random_device rd;

    std::uniform_int_distribution<int> spell_choice(0, (static_cast<int>(SpellType::COUNT) - 1 - NOT_DROPPED_SPELL_COUNT));
    std::mt19937 gen(rd());

    if (!isAbleToSacrifice()) return;

    for (int x = 0; x < PLASMA_SACRIFICE_COST; x++)
    {
        SpellType choice = static_cast<SpellType>(spell_choice(gen));
        while (collectedSpells[choice] <= 1)
        {
            choice = static_cast<SpellType>(spell_choice(gen));
        }
        collectedSpells[choice]--;
    }
}

bool SpellQueue::hasSpell(SpellType spell)
{
    return collectedSpells[spell] > 0;
}

const int SpellQueue::getSpellUpgradeTrack(SpellType spell)
{
    return upgradeTracker[spell];
}

const int SpellQueue::getSpellLevel(SpellType spell)
{
    return collectedSpells[spell];
}
