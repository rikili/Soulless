
#define GL3W_IMPLEMENTATION
#include <gl3w.h>
#include <chrono>
#include "core/render_system.hpp"
#include "core/world_system.hpp"
#include "input/glrender.hpp"

#define ERROR_SUCCESS 0

int main(int argc, char* argv[])
{
	WorldSystem world;
	RenderSystem renderer;

	GLFWwindow* window = world.create_window();

	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	renderer.init(window);
	// create the window and size it
	//GLWindow glWindow(1024, 768);

	//glfwMakeContextCurrent(glWindow.window);
	////glfwSwapInterval(1); // vsync

	//// Load OpenGL function pointers... before we render or you will segfault below...
	//const int is_fine = gl3w_init();
	//assert(is_fine == 0);

	//// viewport - necessary if we allow re-sizing of window, which we do not
	//glViewport(0, 0, glWindow.window_width_px(), glWindow.window_width_px());

	//GLuint frame_buffer;
	//frame_buffer = 0;
	//glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	//// check if the pixel buffer is as expected... apparently not the case for some high DPI displays (ex. Retina Display on Macbooks)
	//// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	//int frame_buffer_width_px, frame_buffer_height_px;
	//glfwGetFramebufferSize(glWindow.window, &frame_buffer_width_px, &frame_buffer_height_px);  // Note, this will be 2x the resolution given to glfwCreateWindow on Mac Retina displays
	//if (frame_buffer_width_px != glWindow.window_width_px())
	//{
	//	std::cerr << "WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value" << std::endl;
	//	std::cerr << "glfwGetFramebufferSize = " << frame_buffer_width_px << ", " << frame_buffer_height_px << std::endl;
	//	std::cerr << "window width_height = " << glWindow.window_width_px() << ", " << glWindow.window_height_px() << std::endl;
	//}

	//// create our renderer
	//GLRender render;
	//render.init(glWindow);

	// game loop
	while (!world.is_over()) {

		// we must poll for event or the window will stop responding
		glfwPollEvents();

		// clear the screen (RGB)
		//glClearColor((0.376), (0.78), (0.376), 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
		//// render
		//render.render(glWindow);

		//glfwSwapBuffers(glWindow.window);
		renderer.draw();
	}

	// clean up
	/*glDeleteFramebuffers(1, &frame_buffer);*/

	return ERROR_SUCCESS;
}
