#define GL3W_IMPLEMENTATION

#include "core/render_system.hpp"
#include "core/world_system.hpp"
#include "utils/game_assets.hpp"

#define ERROR_SUCCESS 0 // For Mac OS

int main(int argc, char* argv[])
{
	WorldSystem world;
	AssetManager asset_manager;
	RenderSystem renderer;

	renderer.initialize(1200, 800, "Soulless"); // must be called at the beginning of the program
	GLFWwindow* window = renderer.getGLWindow();

	GameAssets gameAssets = initializeGameAssets(asset_manager); // Initialize the asset manager
	renderer.setAssetManager(&asset_manager);
	world.set_renderer(&renderer);
	world.initialize(); // Initialize the game world

	while (!glfwWindowShouldClose(window)) { // Game loop
		// IMPORTANT: The following lines order are CRUCIAL to the rendering process
		renderer.setUpView(); // clear the screen

		world.step(0.01f); // Update the game state

		renderer.drawFrame(); // Re-render the scene (where the magic happens)
		glfwSwapBuffers(window); // swap front and back buffers
		glfwPollEvents(); // poll for and process events
	}
	// TODO: Add cleanup code here
	return ERROR_SUCCESS;
}
