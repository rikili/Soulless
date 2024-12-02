#pragma once

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <map>
#include "core/common.hpp"
#include "utils/constants.hpp"

enum class SoundEffect {
    FIRE,
    FIRE_MAX,
    FIRE_MAX_EXPLODE,
    VILLAGER_DAMAGE,
    PITCHFORK_DAMAGE,
    LIGHTNING,
    WATER,
    WATER_EXPLODE,
    PLAYER_DEFEATED,
    DISCARD_SPELL,
    POWERUP_PICKUP,
    POWERUP_SPAWN,
    ICE,
    ICE_MAX,
    WIND,
    PLASMA,
    BOSS_DEATH_BELL,
    CHOIR,
    SHIELD_BLOCK,
    WIND_MAX,
    COMEHERE,
    PORTAL_DAMAGE,
};

enum class Song {
    MAIN,
    DEFEAT,
    BOSS
};

class SoundManager {
public:
    static SoundManager* getSoundManager();

    bool initialize();

    void playSound(SoundEffect effect);

    void playMusic(Song song);

    void fadeInMusic(Song song);

    void toggleMusic();

    void removeSoundManager();

    bool isMusicPlaying();

    static SoundEffect convertSpellToSoundEffect(SpellType spellType, int level);

private:
    SoundManager();

    bool musicPlaying;

    static SoundManager* instance;

    std::map<SoundEffect, Mix_Chunk*> soundEffects;
    std::map<Song, Mix_Music*> music;

    void registerSound(SoundEffect effect, const char* filePath);
    void registerMusic(Song song, const char* filePath);
};
