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

SoundManager::SoundManager() : backgroundMusic(nullptr) {}

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

		backgroundMusic = Mix_LoadMUS(audio_path("background_music.mp3").c_str());

    if (backgroundMusic == nullptr) {
				printd("Failed to load background music: Mix_Error: %s\n", Mix_GetError());
				return false;
    }

    registerSound(SoundEffect::FIRE, audio_path("fireball.mp3").c_str());
    registerSound(SoundEffect::VILLAGER_DAMAGE, audio_path("villager_damage.mp3").c_str());

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

void SoundManager::playMusic() {
    if (backgroundMusic != nullptr) {
        Mix_PlayMusic(backgroundMusic, -1);
    }
}

void SoundManager::removeSoundManager() {
    for (auto& pair : soundEffects) {
        Mix_FreeChunk(pair.second);
    }

    if (backgroundMusic != nullptr) {
        Mix_FreeMusic(backgroundMusic);
				backgroundMusic = nullptr;
    }

    soundEffects.clear();

    Mix_CloseAudio();
}

void SoundManager::registerSound(SoundEffect effect, const char *filePath) {
    Mix_Chunk *chunk = Mix_LoadWAV(filePath);
    if (chunk == nullptr) {
				printd("Failed to load sound %s with error %s\n", filePath, Mix_GetError());
    } else {
        soundEffects[effect] = chunk;
    }
}
