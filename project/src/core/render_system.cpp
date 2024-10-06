#include "core/render_system.hpp"
#include <SDL.h>
#include "entities/ecs_registry.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/matrix_transform.hpp>

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
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->inputHandler.onKey(_0, _1, _2, _3);
		};

	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->inputHandler.onMouseMove({ _0, _1 });
		};

	auto mouse_button_redirect = [](GLFWwindow* wnd, int _1, int _2, int _3) {
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->inputHandler.onMouseKey(_1, _2, _3);
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
 * @brief Add entity to be rendered
 * Pushes to render requests, which are iterated through in each draw call
 */
void RenderSystem::addRenderRequest(Entity entity, AssetId mesh, AssetId texture, AssetId shader)
{
	render_requests.push_back({ entity, std::move(mesh), std::move(texture), std::move(shader) });
}

/**
 * @brief Draw the frame
 * This function is called every frame to draw the frame
 * @attention Skips rendering if the shader is not found
 */
void RenderSystem::drawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front

	// Draw the background
	const Shader* bgShader = this->asset_manager.getShader("background");
	const Mesh* bgMesh = this->asset_manager.getMesh("background");
	const Texture* bgTexture = this->asset_manager.getTexture("grass");

	if (bgShader && bgMesh && bgTexture) {
		glUseProgram(bgShader->program);

		// Set the texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bgTexture->handle);  // Assuming Texture struct has an 'id' field
		glUniform1i(glGetUniformLocation(bgShader->program, "backgroundTexture"), 0);

		// Set the repeat factor (adjust these values to control the number of repetitions)
		glUniform2f(glGetUniformLocation(bgShader->program, "repeatFactor"), 10.0f, 10.0f);

		glBindVertexArray(bgMesh->vao);
		glDrawElements(GL_TRIANGLES, bgMesh->indexCount, GL_UNSIGNED_INT, 0);
	}


	// Draw all entities
	for (const RenderRequest& render_request : this->render_requests)
	{
		Entity entity = render_request.entity;
		Motion& motion = registry.motions.get(entity);

		if (!registry.motions.has(entity))
		{
			std::cerr << "Entity " << entity << " does not have a motion component" << std::endl;
			std::cerr << "Skipping rendering of this entity" << std::endl;
			continue;
		}

		if (render_request.shader != "") {
			const Shader* shader = this->asset_manager.getShader(render_request.shader);
			if (!shader)
			{
				printf("Could not find shader with id %s\n", render_request.shader.c_str());
				printf("Skipping rendering of this shader\n");
				continue;
			}
			const GLuint shaderProgram = shader->program;
			glUseProgram(shaderProgram);

			mat4 transform = mat4(1.0f);
			transform = translate(transform, glm::vec3(motion.position, 0.0f));
			transform = scale(transform, vec3(motion.scale * 100.f, 1.0f));


			if (render_request.shader == "sprite") {

				const Texture* texture = this->asset_manager.getTexture(render_request.texture);
				if (!texture)
				{
					std::cerr << "Texture with id " << render_request.texture << " not found!" << std::endl;
					continue;
				}

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture->handle);
				glUniform1i(glGetUniformLocation(shader->program, "image"), 0);
			}

		}

			mat4 projection = glm::ortho(0.f, (float)window_width_px, (float)window_height_px, 0.f, -1.f, 1.f);

			const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			const GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			gl_has_errors();
		}


		const Mesh* mesh = this->asset_manager.getMesh(render_request.mesh);

		if (!mesh)
		{
			std::cerr << "Mesh with id " << render_request.mesh << " not found!" << std::endl;
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


void RenderSystem::removeRenderRequest(Entity entity)
{
	auto it = std::remove_if(this->render_requests.begin(), this->render_requests.end(),
		[entity](const RenderRequest& render_request) {
			return render_request.entity == entity;
		});
	this->render_requests.erase(it, this->render_requests.end());
}
