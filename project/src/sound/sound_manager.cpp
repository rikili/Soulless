// request to play sound
// initializes / loads all music / sounds
//
// do one for spells that follows the enum
// playSpellSound
//
// for enemies: register it... and maybe don't allow other hit sounds for 0.5 seconds... so that it doesnt play a bunch of enemy death sounds
// different sounds for getting hit by a certain spell?

// enemy getting hit sound

// dying sound

#include "sound/sound_manager.hpp"

SoundManager* SoundManager::instance = nullptr;

SoundManager::SoundManager() {}

SoundManager* SoundManager::getSoundManager() {
    if (instance == nullptr) {
        instance = new SoundManager();
    }
    return instance;
}

bool SoundManager::initialize() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printd("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printd("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        return false;
    }

    registerMusic(Song::MAIN, "background_music.mp3");
    registerMusic(Song::DEFEAT, "player_defeated_music.mp3");

    registerSound(SoundEffect::FIRE, "fireball.mp3");
    registerSound(SoundEffect::VILLAGER_DAMAGE, "villager_damage.mp3");
    registerSound(SoundEffect::PITCHFORK_DAMAGE, "pitchfork_damage.mp3");
    registerSound(SoundEffect::PLAYER_DEFEATED, "player_defeated.mp3");

    return true;
}

void SoundManager::playSound(SoundEffect effect) {
    auto soundEffect = soundEffects.find(effect);
    if (soundEffect != soundEffects.end()) {
        Mix_PlayChannel(-1, soundEffect->second, 0);
    } else {
        printd("Sound effect not found!\n");
    }
}

void SoundManager::playMusic(Song song) {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }

    if (music.find(song) != music.end()) {
        Mix_PlayMusic(music[song], -1);
    } else {
        std::cerr << "Song not found!" << std::endl;
    }
}

void SoundManager::removeSoundManager() {
    for (auto& pair : soundEffects) {
        Mix_FreeChunk(pair.second);
    }

    soundEffects.clear();

    for (auto& pair : music) {
        Mix_FreeMusic(pair.second);
    }

    music.clear();

    Mix_CloseAudio();
}

void SoundManager::registerSound(SoundEffect effect, const char *filePath) {
    Mix_Chunk *chunk = Mix_LoadWAV(audio_path(filePath).c_str());

    if (chunk == nullptr) {
        printd("Failed to load sound %s with error %s\n", filePath, Mix_GetError());
    } else {
        soundEffects[effect] = chunk;
    }
}

void SoundManager::registerMusic(Song song, const char *filePath) {
    Mix_Music *backgroundMusic = Mix_LoadMUS(audio_path(filePath).c_str());

    if (backgroundMusic == nullptr) {
        printd("Failed to load sound %s with error %s\n", filePath, Mix_GetError());
    } else {
        music[song] = backgroundMusic;
    }
}
