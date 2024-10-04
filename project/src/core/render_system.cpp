#include "core/render_system.hpp"
#include <SDL.h>
#include "entities/ecs_registry.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <glm/gtc/type_ptr.inl>

/**
 * @brief Initialize the render system
 *
 * @param width: the width of the window in pixels
 * @param height: the height of the window in pixels
 * @param title: the title of the window
 * @return true: if the render system is initialized successfully
 * @return false: if the render system is not initialized successfully
 */
bool RenderSystem::initialize(const int width, const int height, const char* title)
{
	// Most of the code below is just boilerplate code to create a window
	if (!glfwInit()) { 	// Initialize the window
		exit(EXIT_FAILURE);
	}

	// Create a windowed mode window and its OpenGL context -------------------------
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);
	// ------------------------------------------------------------------------------

	this->window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!this->window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetWindowUserPointer(this->window, this);

	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) {
		((RenderSystem *)glfwGetWindowUserPointer(wnd))->inputHandler.onKey(_0, _1, _2, _3);
	};

	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
		((RenderSystem *)glfwGetWindowUserPointer(wnd))->inputHandler.onMouseMove({_0, _1});
	};

	auto mouse_button_redirect = [](GLFWwindow* wnd, int _1, int _2, int _3) {
			((RenderSystem *)glfwGetWindowUserPointer(wnd))->inputHandler.onMouseKey(_1, _2, _3);
	};

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	glfwMakeContextCurrent(this->window);
	glfwSwapInterval(1);

	const int is_fine = gl3w_init();
	assert(is_fine == 0);

	// Set initial window colour
	glClearColor(0.376f, 0.78f, 0.376f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(this->window);
	return true;
}

/**
 * @brief Set up the view for rendering
 * For each frame, we need to clear the screen and set up the view
 * Should be called at the beginning of each frame in the render loop
 */
void RenderSystem::setUpView() const
{
	int width, height;
	glfwGetFramebufferSize(this->window, &width, &height);
	glViewport(0, 0, width, height);
	glClearColor((0.376), (0.78), (0.376), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

/**
 * @brief Get the GL window
 * Other systems may need to access the GL window; for example, the input system
 * @return GLWindow*: the GL window
 */
GLFWwindow* RenderSystem::getGLWindow() const
{
	return this->window;
}

/**
 * @brief Draw the frame
 * This function is called every frame to draw the frame
 * @attention Skips rendering if the shader is not found
 */
void RenderSystem::drawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	for (const RenderRequest& render_request : this->render_requests)
	{
		Entity entity = render_request.entity;
		Motion &motion = registry.motions.get(entity);

		if (!registry.motions.has(entity))
		{
			std::cerr << "Entity " << entity << " does not have a motion component" << std::endl;
			std::cerr << "Skipping rendering of this entity" << std::endl;
			continue;
		}

		const Shader* shader = this->asset_manager.getShader(render_request.asset_id);
		if (!shader)
		{
			printf("Could not find shader with id %s\n", render_request.asset_id.c_str());
			printf("Skipping rendering of this shader\n");
			continue;
		}
		const GLuint shaderProgram = shader->program;
		glUseProgram(shaderProgram);

		mat4 transform = mat4(1.0f); // Start with an identity matrix
		transform = translate(transform, glm::vec3(motion.position, 0.0f)); // Apply translation
		transform = scale(transform, glm::vec3(motion.scale, 1.0f)); // Apply scaling


		const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		const Mesh* mesh = this->asset_manager.getMesh(render_request.asset_id);

		if (!mesh)
		{
			std::cerr << "Mesh with id " << render_request.asset_id << " not found!" << std::endl;
			std::cerr << "Skipping rendering of this mesh" << std::endl;
			continue;
		}
		glBindVertexArray(mesh->vao);
		glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << error << std::endl;

		}

	// 	std::cout << "Drawing entity " << entity << " at position ("
	// 	  << motion.position.x << ", " << motion.position.y
	// 	  << ") with scale (" << motion.scale.x << ", " << motion.scale.y << ")" << std::endl;
	// 	std::cout << "Mesh vertex count: " << mesh->vertexCount
	// 	  << ", index count: " << mesh->indexCount << std::endl;
	}
}

