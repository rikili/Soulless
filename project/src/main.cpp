
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
// #include "physics_system.hpp"
// #include "render_system.hpp"
// #include "world_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
    // TODO: Implement the main function
    // Global systems
    // WorldSystem world;
    // RenderSystem renderer;
    // PhysicsSystem physics;
    
    // // Initializing window
    // GLFWwindow* window = world.create_window();
    // if (!window) {
    //     // Time to read the error message
    //     printf("Press any key to exit");
    //     getchar();
    //     return EXIT_FAILURE;
    // }
    
    // // initialize the main systems
    // renderer.init(window);
    // world.init(&renderer);


    // auto t = Clock::now();
    
    // Countdown timer
    auto start = std::chrono::steady_clock::now();
    auto countdown_duration = std::chrono::minutes(5);

    // while (!world.is_over()) {
    while (true) {
    //     // Processes system messages, if this wasn't present the window would become unresponsive
    //     glfwPollEvents();
    //

        auto now = std::chrono::steady_clock::now();
        auto remaining = countdown_duration - (now - start);

        if (remaining <= std::chrono::seconds(0)) {
            // TODO: Trigger boss when countdown is over?
        }

        auto remaining_seconds = std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
        // Example print statement -- Can use remaining_seconds to display time remaining until boss otherwise
        // printf("Time remaining: %d:%d\n", remaining_seconds / 60, remaining_seconds % 60);

        // Calculating elapsed times in milliseconds from the previous iteration
		// auto now = Clock::now();
		// float elapsed_ms =
		// 	(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		// t = now;
    
    //     world.step(elapsed_ms);
    //     physics.step(elapsed_ms);
    //     world.handle_collisions();
    //
    //     renderer.draw();
        
    }

    return 0;
}
