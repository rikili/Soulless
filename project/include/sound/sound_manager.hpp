#pragma once

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <map>
#include "core/common.hpp"

enum class SoundEffect {
    FIRE,
    VILLAGER_DAMAGE,
    PITCHFORK_DAMAGE,
    PLAYER_DEFEATED
};

enum class Song {
    MAIN,
    DEFEAT
};

class SoundManager {
public:
    static SoundManager* getSoundManager();

    bool initialize();

    void playSound(SoundEffect effect);

    void playMusic(Song song);

    void removeSoundManager();

private:
    SoundManager();

    static SoundManager* instance;

    std::map<SoundEffect, Mix_Chunk*> soundEffects;
    std::map<Song, Mix_Music*> music;

    void registerSound(SoundEffect effect, const char* filePath);
    void registerMusic(Song song, const char* filePath);
};
