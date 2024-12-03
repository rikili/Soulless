# Team10



Team 10 - Soulless

### **Instructions to Start the Game:**

1. In the project folder, run:
   - cmake .
   - make
2. After building, run ./soulless
3. Ensure that your machine's audio is on.



------



### **Game Mechanics and Controls:**

#### **Player Movement:**

- **Movement Keys**: Player can move in 8 directions (N, S, E, W, NW, NE, SW, SE) using W, A, S, D.
  - Holding two keys, such as WA, moves diagonally (e.g., NW).
  - Movement is independent of mouse position.

#### **Shooting Mechanics:**

- **Mouse Direction**: Shooting direction is based on mouse position, allowing for any direction.
- **Shooting Action**: Left-click to shoot first spell in queue; Right-click to shoot second spell in queue; cooldown prevents rapid firing / rapid healing
  - Player character should turn to face closest cardinal or intermediate direction towards the mouse when shooting a spell
- **Drop Spell and Heal:** Press q to drop the left spell. Press e to drop the right spell. Heals the player by a small amount.
- **Projectile**: See Spell Mechanics below.
- **Enemy Interaction**: Damaging spells reduce enemy health on impact; enemies disappear only when their health reaches 0.
- **Audio Cue**: Ensure sound effects for fireball, lightning, water, and enemy hits trigger at appropriate times.
- **Cooldown**: casting a spell causes a cooldown of 1s. Dropping a spell causes a cooldown of 1.5s. There is a separate timer / cooldown for the left and right spell.

#### **Health Mechanics and Invincibility:**

- **Damage and Cooldowns**: Player takes damage upon contact with enemies or their projectiles, indicated by an impact sound.
  - **Damage Lock**: Player is invincible for a short time after taking damage.
  - **Visual Indicator**: Player flashes red when damaged, and then becomes slightly transparent to indicate the damage lock.
- **Health Bar**: Health bar is shown above player and enemies, and corresponds to their current health. Player begins with 100 HP (which is also the maximum). Enemies have varying HP.
- **Health Recovery**: 
  - Player can regain a small amount of health by dropping a spell.

#### **Additional Controls:**

- **FPS Toggle**: Press Shift + F to toggle FPS display in the top-right corner.
- **Game Over Shortcut**: Press "J" to force a game over.
- **Debug Mode:** Press “K” to enter debug mode, visualizing bounding boxes, collisions, and debug logs
- **Force Spawn Powerup Collectible:** (requires debug mode) “P” to initiate spell spawn event
- **Tutorial mode**: Explained later in the document

#### **Spell Mechanics:**

A spell will level up from 1 - 5, by progressively doing damage with it or picking it up off the ground. Each of these contributes some XP to leveling it up. From levels 1 - 4 a spell’s damage increases. Upon reaching its max level (5), the spell evolves and gains a new effect. Spells will be randomly dropped across the map every 25 seconds. 

At 2.5 minutes remaining, the necromancer appears towards the top middle of the map. The player can interact with the necromancer to obtain plasma by sacrificing 5 random spell levels (player needs 7 spell levels minimum. Ex. Fire 5, Wind 2 -> sacrifice -> Fire 1, Wind 1).

**Spell Types**:

- **Fire Projectile**: Shoots in a straight line, damaging the first enemy hit or disappearing after a certain distance.
  - Max level: explodes upon impact dealing damage to all enemies nearby
- **Lightning Strike**: Hits a large area after a short delay, damaging multiple enemies. (large radius).
  - Max level: causes a chain of lightning strikes after the initial one explodes
- **Water Barrier**: Orb around the player that blocks up to three unique attacks and explodes after a few seconds. For each attack it absorbs, the explosion damage increases (damage increases per attack up to 3). If the player already has a water barrier and casts another one, it just resets the barrier timer.
  - Max level: Teleports the player and does an initial explosion as well.
- **Wind Pillar**: A tornado that can be placed that does continuous damage over its lifetime to nearby enemies
  - Max level: pulls enemies towards it.
- **Plasma**: Shoots in a straight line, flies slow then fast and damages first enemy hit.
  - Not upgradable

- **Spell Queue**: Spell queue holds up to 8 spells, adding a random spell when one is used
  - **Spell Display**: Left and right hand spell box shown in HUD. Also shows the next 6 spells in the queue below.
  - **Cooldown**: upon casting or dropping a spell, there’s a cooldown indicated by a shrinking grey filter within the left or right hand spell box.

#### **Camera and Visuals:**

- **Fixed Camera**: Camera follows the player as they move, stops at world boundaries.
- **Collision Visualization**: In debug mode, collision boxes are shown for entities with motion.
  - **Mesh Collisions**: Player collides with entities only when the bounding box collides with the player’s mesh.
  - **Map Boundaries**: Ensure the player remains within map boundaries.
- **Rain:** During the duration of the game a rain effect should consistently be playing
- **Background Generation**: A randomly generated, grassy/muddy background with shrubs should be visible throughout the game’s playtime
- **Cursor:** Bow and arrow style cursor in place of computer cursor

#### **Animation:**

- **Player Animations**: Includes idle, walking and spell-casting animations. Animations align with the 8 directions in the game. 
- **Spell Animations:** All spells are animated during their cast



------



### **Game Start, Tutorial, and Pausing:**

- **Tutorial Screen**: Game begins with a tutorial showing controls. Press space or mouse click to exit the tutorial or pause screen.
  - **Pause / Resume**: Press shift + T to pause or open the tutorial during gameplay. Game loop should also pause/resume.
  - **Navigation:** There are three pages in the tutorial screen, navigate with keys “1”, “2”, and “3”
  - **Gameplay:** Description of the objectives, story, and events to look for in the game
  - **Controls:** Describes the player controls for the game
  - **Settings:** Additional controls for debugging and testing



------



### **Cutscenes:**

Can press space to skip a cutscene.

- **Cutscene 1:** Plays at the start of the game. Camera pans over a peaceful village and slowly transitions to chaos and then the start of the game.
- **Cutscene 2:** Plays when you unlock all 6 spells.
- **Cutscene 3:** Plays when you defeat the dark lord. Game will exit after.



------



### **Enemy Behaviour:**

- **Enemies Overview**

Enemies spawn incrementally, and will move towards the player and try to attack them.

- **Knights**: Spawn from the start, new one will spawn every 8 seconds.
  - Ranged attack: periodically throw pitchforks which deal 10 damage.
  - Colliding with the player will also deal 5 damage.
  - On low health, they will retreat, periodically blocking which will protect them against some of the player’s spells. Once a certain distance is reached they will move towards the player again, continuing to block.
- **Archers**: Spawn after 1 minute, new one will spawn every 16 seconds
  - Ranged attack: periodically fire arrows which deal 20 damage.
  - On low health, they will run away until a certain distance is reached, at which point they will return to attack the player.
- **Paladin:** Spawn after 2 minutes, new one will spawn every 20 seconds
  - “Tank” enemy (has a lot of HP)
  - Melee attack: sword swing which deals 35 damage
  - On low health, they will charge at the player and deal 25 damage on collision
- **Slasher**: Spawn after 3 minutes, new one will spawn every 20 seconds
  - Fastest enemy, runs at player
  - Melee attack: forward dash/slash which deals 20 damage
- **Dark Lord (Boss):** Player manually spawns it by interacting with the fountain in the middle of the map or will automatically spawn after 10 minutes of play.
  - On low health, increased velocity and decreased cooldown 

- **Enemy AI**:
  - **Low Health:** Enemies (except for slasher) have altered behaviour on low health (covered previously). Low health for non-boss enemies is <35%, for the boss it is <50%.
  - **Move closer to player**: If the player is outside of the enemy’s range, they will move closer to the player.
  - **Attack and Range**: Enemies approach and attack if within range. Ranged enemies stop upon reaching their attack range and will not move closer to the player.

- **Boss Features**:
  - Dark Lord periodically casts its plasma and portal attack.
  - It has a secondary attack that spawns a portal shortly in front of the player (which depends on their current travelling direction). If the player steps on it, they are teleported to the dark lord and given a slow effect that lasts a few seconds.
  - The plasma attack’s speed follows a sigmoid curve: plateaus followed by steep increase until it plateaus again



------



### **Death and Loss Condition:**

- **Player Death**:
  - **Background Music Change**: Changes upon death and reverts after the reset.
  - **Rotation**: Player rotates 180 degrees on death.
  - **Disable Input**: Player cannot move, cast spells, or take damage after death.
  - **Defeat Announcement**: Mage announces defeat with a unique audio cue.
  - **Game Reset**: After ~6 seconds, game resets with the player in the center, and all enemies and projectiles removed.



------



### **Audio Testing:**

#### **Background Music:**

Music should pause when the game pauses.

- **Start**: Background music plays at the beginning.
- **Death**: Music changes on player death and resets after respawn.
- **Boss:** Boss background music when player enters boss battle.

#### **Sound Effects:**

   Verify sound effects for:

- Fireball, water, lightning, wind, ice, and plasma spells. The following spells should have different sounds for their max level.
  - Fireball: New deeper sound with an explosion sound on impact
  - Lightning: Same sound but repeats for secondary lightning strikes after the primary one explodes
  - Water: Cast sound as well as explosion sound
  - Ice: Sharper sound (should only sound like one ice shard)
  - Wind: Different wind sound with paper being blown
- Dark lord portal attack:
  - Audio cure: “come here” right before placing the portal
  - If player steps on the portal, a “zrrrrp” like sound should play
- Spell collectible spawn
- Spell collectible pickup
- Enemy hit
- Player damage
- Player death announcement



------



### **Debug and Testing:**

#### **Debug Controls:**

- **Visualise Collisions**: "K" to show debug collision boxes.
- **Force Spawn Powerup Collectible:** (requires debug) “P” to randomly spawn missing spell collectible
- **Force Game Over**: "J" to immediately trigger game over.