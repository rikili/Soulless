#define GL3W_IMPLEMENTATION
#define SDL_MAIN_HANDLED

#include "core/render_system.hpp"
#include "core/world_system.hpp"
#include "utils/game_assets.hpp"
#include "sound/sound_manager.hpp"
#include "core/common.hpp"

#define ERROR_SUCCESS 0 // For Mac OS

int main(int argc, char* argv[])
{
	AssetManager asset_manager;
	RenderSystem renderer;
	InputHandler input_handler;
	Camera camera(400, 400);
	WorldSystem world = WorldSystem(&renderer);

	renderer.initialize(input_handler, window_width_px, window_height_px, "Soulless"); // must be called at the beginning of the program
	GLFWwindow* window = renderer.getGLWindow();

	SoundManager *soundManager = SoundManager::getSoundManager();
	if (!soundManager->initialize()) {
		printd("Error initializing sound manager\n");
	}
	soundManager->playMusic();

	GameAssets gameAssets = initializeGameAssets(asset_manager); // Initialize the asset manager
	renderer.setAssetManager(&asset_manager);
	world.set_renderer(&renderer);
	world.initialize(); // Initialize the game world

	auto t = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window)) { // Game loop
		// IMPORTANT: The following lines order are CRUCIAL to the rendering process
		renderer.setUpView(); // (1) clear the screen

		auto now = std::chrono::high_resolution_clock::now();
		const float elapsed_ms = static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count()) / 1000.0f;
		t = now;

		world.step(elapsed_ms); // Update the game state

		renderer.drawFrame(camera); // Re-render the scene (where the magic happens)
		glfwSwapBuffers(window); // swap front and back buffers
		glfwPollEvents(); // poll for and process events
	}

	// TODO: Add cleanup code here
	soundManager->removeSoundManager();
	return ERROR_SUCCESS;
}
