#include "core/render_system.hpp"
#include <SDL.h>
#include "entities/ecs_registry.hpp"
#include "utils/sorting_functions.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/matrix_transform.hpp>
#include "entities/general_components.hpp"

/**
 * @brief Initialize the render system
 *
 * @param width: the width of the window in pixels
 * @param height: the height of the window in pixels
 * @param title: the title of the window
 * @return true: if the render system is initialized successfully
 * @return false: if the render system is not initialized successfully
 */
bool RenderSystem::initialize(InputHandler& input_handler, const int width, const int height, const char* title)
{
	projectionMatrix = glm::ortho(0.f, (float)window_width_px * zoomFactor, (float)window_height_px * zoomFactor, 0.f, -1.f, 1.f);
	registry.projectionMatrix = projectionMatrix;

	initializeCamera();

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
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->input_handler.onKey(_0, _1, _2, _3);
		};

	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->input_handler.onMouseMove({ _0, _1 });
		};

	auto mouse_button_redirect = [](GLFWwindow* wnd, int _1, int _2, int _3) {
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->input_handler.onMouseKey(wnd, _1, _2, _3);
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

void RenderSystem::initializeCamera() {
	camera = Entity();
	registry.cameras.emplace(camera);
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
void RenderSystem::drawFrame(float elapsed_ms)
{
	if (globalOptions.tutorial) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

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

	if (globalOptions.tutorial) {
		float titleFontSize = this->asset_manager.getFont("king")->size;
		float tutFontSize = this->asset_manager.getFont("deutsch")->size;
		float currentY = window_height_px - titleFontSize;
		drawText("Soulless", "king", window_width_px / 2.0f, currentY, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		currentY -= titleFontSize * 1.2;

		vec3 color = glm::vec3(0.83f, 0.83f, 0.83f);
		
		drawText("Move using: W, A, S, D", "deutsch", window_width_px / 2.0f, currentY, 1.0f, color);
		currentY -= tutFontSize * 1.5;

		drawText("Aim with your mouse.", "deutsch", window_width_px / 2.0f, currentY, 1.0f, color);
		currentY -= tutFontSize * 1.5;

		drawText("Left click to shoot.", "deutsch", window_width_px / 2.0f, currentY, 1.0f, color);
		currentY -= tutFontSize * 1.5;

		drawText("Press t to pause/show tutorial.", "deutsch", window_width_px / 2.0f, currentY, 1.0f, color);
		currentY -= tutFontSize * 1.5;

		drawText("Survive as long as you can!", "deutsch", window_width_px / 2.0f, currentY, 1.0f, color);

		std::string message = "Press SPACE key to ";
		std::string start = globalOptions.pause ? "resume." : "start.";
		drawText(message + start, "deutsch", window_width_px / 2.0f, tutFontSize * 1.5, 1.0f, color);
		return;
	}

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

	Entity player = registry.players.entities[0];
	float playerX = registry.motions.get(player).position.x - window_width_px / 2.0 * zoomFactor;
	float playerY = registry.motions.get(player).position.y - window_height_px / 2.0 * zoomFactor;

	updateCameraPosition(clamp(playerX, 0.f, (float)(window_width_px / 2.0)),
						 clamp(playerY, 0.f, (float)(window_height_px / 2.0)));
	
	// Draw all entities
	// registry.render_requests.sort(typeAscending);
	this->updateRenderOrder(registry.render_requests);

	for (const auto& render_index : sorted_indices)
	{
		Entity& entity = registry.render_requests.entities.at(render_index.index);
		RenderRequest& render_request = registry.render_requests.get(entity);

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

			vec2 position;
			if (registry.motions.has(entity)) {
				position = registry.motions.get(entity).position;
			} else {
				// Fallback to render_y if no Motion component
				position = vec2(0, render_request.smooth_position.render_y);
			}

			mat4 transform = mat4(1.0f);
			transform = translate(transform, glm::vec3(position, 0.0f));

			// Rotate the sprite for all eight directions if request type is a PROJECTILE
			if (render_request.type == PROJECTILE) {
				// printd("Fireball angle: %f\n", motion.angle);
				transform = rotate(transform, motion.angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Apply rotation
			}
			transform = scale(transform, vec3(motion.scale * 100.f * zoomFactor, 1.0f));


			if (render_request.shader == "sprite" || render_request.shader == "animatedsprite") {
				if (registry.players.has(entity) && registry.deaths.has(entity)) {
					transform = rotate(transform, (float) M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
				}

				const Texture* texture = this->asset_manager.getTexture(render_request.texture);
				if (!texture)
				{
					std::cerr << "Texture with id " << render_request.texture << " not found!" << std::endl;
					continue;
				}

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture->handle);
				glUniform1i(glGetUniformLocation(shader->program, "image"), 0);

				if (render_request.shader == "animatedsprite") {
					Animation& animation = registry.animations.get(entity);
					animation.elapsedTime += elapsed_ms;
					
					if (animation.elapsedTime > animation.frameTime) {
						animation.elapsedTime = 0;
						animation.currentFrame++;
						/*if (registry.players.has(entity)) {
							printd("Frame: %f\n", animation.currentFrame);
						}*/
						if (animation.currentFrame - animation.startFrame >= animation.frameCount) {
							animation.currentFrame = animation.startFrame;
						}
					}

					if (registry.players.has(entity) && registry.onHits.has(entity)) {
						if (registry.onHits.get(entity).invicibilityShader) {
							glUniform1i(glGetUniformLocation(shaderProgram, "state"), 2);
						}
						else {
							glUniform1i(glGetUniformLocation(shaderProgram, "state"), 1);
						}		
					}
					else {
						glUniform1i(glGetUniformLocation(shaderProgram, "state"), 0);
					}
					glUniform1f(glGetUniformLocation(shaderProgram, "frame"), animation.currentFrame);
					glUniform1i(glGetUniformLocation(shaderProgram, "SPRITE_COLS"), animation.spriteCols);
					glUniform1i(glGetUniformLocation(shaderProgram, "SPRITE_ROWS"), animation.spriteRows);
					glUniform1i(glGetUniformLocation(shaderProgram, "NUM_SPRITES"), animation.spriteCount);	
				}
			}
			mat4 projection = projectionMatrix;
			mat4 view = viewMatrix;

			const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			const GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			const GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
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

	// TODO: does this work with the camera????
	if (globalOptions.showFps) {
		drawText(std::to_string(globalOptions.fps), "deutsch", window_width_px - 100.0f, window_height_px - 50.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	// makes it kinda slow
	for (Entity entity : registry.enemies.entities) {
		if (registry.healths.has(entity) && registry.motions.has(entity)) {
			Motion &motion = registry.motions.get(entity);
			Health &health = registry.healths.get(entity);

			int percentage = static_cast<int>((health.health / health.maxHealth) * 100);

			drawText(std::to_string(percentage) + "%", "healthFont", motion.position.x, motion.position.y - 55.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}
}

// source: inclass SimpleGL-3
// the x,y values should be the position we want the center of our text to be
void RenderSystem::drawText(const std::string& text, const std::string& fontName, float x, float y, float scale, const glm::vec3& color) {
	Font* font = this->asset_manager.getFont(fontName);
	float textWidth = getTextWidth(text, fontName, scale);

	x = x - textWidth / 2;
	y = y - font->size / 2;

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, glm::vec3(0.0, 0, 0.0));
	trans = glm::rotate(trans, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(1.0f, 1.0f, 1.0f));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const Shader* fontShader = this->asset_manager.getShader("font");
	GLuint m_font_shaderProgram = fontShader->program;
	glUseProgram(m_font_shaderProgram);
	
	mat4 view;
	mat4 projection;
	
	GLint flipLoc = glGetUniformLocation(m_font_shaderProgram, "flip");
	
	if (fontName == "healthFont") {
		view = viewMatrix;
		projection = registry.projectionMatrix;
		glUniform1f(flipLoc, true);
	}
	else {
		view = mat4(1.0f);
		projection = glm::ortho(0.f, (float)window_width_px, 0.0f, (float)window_height_px);
		glUniform1f(flipLoc, false);
	}
	
	GLint view_location = glGetUniformLocation(m_font_shaderProgram, "view");
	glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
	
	GLint projection_location = glGetUniformLocation(m_font_shaderProgram, "projection");
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

	GLint textColor_location =
		glGetUniformLocation(m_font_shaderProgram, "textColor");
	glUniform3f(textColor_location, color.x, color.y, color.z);

	GLint transformLoc =
		glGetUniformLocation(m_font_shaderProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	

	glBindVertexArray(font->vao);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = font->m_ftCharacters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	gl_has_errors();
}

float RenderSystem::getTextWidth(const std::string& text, const std::string& fontName, float scale) {
    const Font* font = this->asset_manager.getFont(fontName);
    float width = 0.0f;
    for (char c : text) {
        auto it = font->m_ftCharacters.find(c);
        if (it != font->m_ftCharacters.end()) {
            Character ch = it->second;
            width += (ch.Advance >> 6) * scale;
        }
    }
    return width;
}

void RenderSystem::updateCameraPosition(float x, float y) {
	Camera& cameraEntity = registry.cameras.get(camera);
	cameraEntity.position.x = x;
	cameraEntity.position.y = y;

	viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraEntity.position, 0.0f));
	registry.viewMatrix = viewMatrix;
}
