#define GL3W_IMPLEMENTATION
#define SDL_MAIN_HANDLED

#include "core/render_system.hpp"
#include "core/world_system.hpp"
#include "utils/game_assets.hpp"
#include "sound/sound_manager.hpp"
#include "core/common.hpp"
#include "entities/general_components.hpp"

#define ERROR_SUCCESS 0 // For Mac OS

int main(int argc, char* argv[])
{
	AssetManager asset_manager;
	RenderSystem renderer;
	InputHandler input_handler;
	WorldSystem world = WorldSystem(&renderer);

	renderer.initialize(input_handler, window_width_px, window_height_px, "Soulless"); // must be called at the beginning of the program
	GLFWwindow* window = renderer.getGLWindow();

	SoundManager* soundManager = SoundManager::getSoundManager();
	if (!soundManager->initialize()) {
		printd("Error initializing sound manager\n");
	}

	GameAssets gameAssets = initializeGameAssets(asset_manager); // Initialize the asset manager
	renderer.setAssetManager(&asset_manager);
	world.set_renderer(&renderer);
	world.initialize(); // Initialize the game world

	auto t = std::chrono::high_resolution_clock::now();

	unsigned int frames = 0;
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (!glfwWindowShouldClose(window)) { // Game loop
		// IMPORTANT: The following lines order are CRUCIAL to the rendering process
		renderer.setUpView(); // (1) clear the screen

		auto now = std::chrono::high_resolution_clock::now();
		const float elapsed_ms = static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count()) / 1000.0f;
		t = now;

		if (!globalOptions.tutorial) {
			world.step(elapsed_ms); // (2) Update the game state
			if (registry.game_over)
			{
				continue;
			}
		}

		renderer.drawFrame(elapsed_ms); // (3) Re-render the scene (where the magic happens)

		frames++;

		auto currentTime = std::chrono::high_resolution_clock::now();
		float timeDifference = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count());

		if (timeDifference >= 1000.0f) {
			globalOptions.fps = frames / (timeDifference / 1000.0f);
			frames = 0;
			lastTime = currentTime;
		}

		glfwSwapBuffers(window); // (4) swap front and back buffers
		glfwPollEvents(); // (5) poll for and process events

		registry.debug_requests.clear();
	}

	// TODO: Add cleanup code here
	soundManager->removeSoundManager();
	return ERROR_SUCCESS;
}
