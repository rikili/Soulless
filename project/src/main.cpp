#define GL3W_IMPLEMENTATION

#include "core/render_system.hpp"
#include "core/world_system.hpp"
#include "utils/game_assets.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "entities/ecs_registry.hpp"

#define ERROR_SUCCESS 0 // For Mac OS

int main(int argc, char* argv[])
{
	// const WorldSystem world; // TODO: world is not currently used
	AssetManager asset_manager;
	RenderSystem renderer;

	// IMPORTANT: This function must be called before any OpenGL function otherwise it won't work properly
	renderer.initialize(1200, 800, "Soulless");
	GLFWwindow* window = renderer.getGLWindow();

	GameAssets gameAssets = initializeGameAssets(asset_manager); // Initialize the asset manager
	renderer.setAssetManager(&asset_manager);

	// TODO: Delete this code - only for testing purposes
	Entity player;
    registry.players.emplace(player);
	Motion& motion = registry.motions.emplace(player);
	motion.position = { 10.0f, 10.0f };
	motion.velocity = { 0.0f, 0.0f };
	motion.scale = { 100.0f, 100.0f };


	renderer.addRenderRequest(player, "basic");
	// End of testing code


	// Game loop - basically updating the game state and rendering the scene
	while (!glfwWindowShouldClose(window)) {
		// IMPORTANT: The following lines order are CRUCIAL to the rendering process
		renderer.setUpView(); // clear the screen
		renderer.drawFrame(); // Re-render the scene (where the magic happens)
		glfwSwapBuffers(window); // swap front and back buffers
		glfwPollEvents(); // poll for and process events
	}
	// TODO: Add cleanup code here
	return ERROR_SUCCESS;
}
