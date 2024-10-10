#pragma once

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <map>
#include "core/common.hpp"

enum class SoundEffect {
		FIRE,
		VILLAGER_DAMAGE
};

class SoundManager {
public:
    static SoundManager* getSoundManager();

    bool initialize();

    void playSound(SoundEffect effect);

    void playMusic();

    void removeSoundManager();

private:
    SoundManager();

    static SoundManager* instance;

    std::map<SoundEffect, Mix_Chunk*> soundEffects;

    Mix_Music* backgroundMusic;

    void registerSound(SoundEffect effect, const char* filePath);
};
