#include "core/render_system.hpp"
#include <SDL.h>
#include "entities/ecs_registry.hpp"
#include "utils/sorting_functions.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <glm/gtc/type_ptr.inl>
#include "stb_image.h"
#include "core/common.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <libavformat/avformat.h>

#include "entities/general_components.hpp"
#include "graphics/video_player.hpp"
#include "utils/spell_queue.hpp"
#include "utils/isometric_helper.hpp"

std::string spellTypeToCollect(SpellType spell) {
	switch (spell) {
	case SpellType::FIRE: return "fire-collect";
	case SpellType::WATER: return "water-collect";
	case SpellType::LIGHTNING: return "lightning-collect";
	case SpellType::ICE: return "ice-collect";
	case SpellType::WIND: return "wind-collect";
	case SpellType::PLASMA: return "plasma-collect";
	default: return "Unknown";
	}
}

glm::vec3 spellTypeToColor(SpellType spell) {
	switch (spell) {
	case SpellType::FIRE: return { 0.984313725490196f, 0.5215686274509804f, 0.20784313725490197f };
	case SpellType::WATER: return { 0.09411764705882353f, 0.7568627450980392f, 0.8980392156862745f };
	case SpellType::LIGHTNING: return { 0.984313725490196f, 0.9490196078431372f, 0.21176470588235294f };
	case SpellType::ICE: return { 0, 1, 1 }; // TODO: Change this to a different color
	case SpellType::WIND: return { 0.7176470588235294f, 0.8705882352941177f, 0.7176470588235294f };
	case SpellType::PLASMA: return { 0.949f, 0.082f, 0.957f };
	default: return { 0.f, 0.f, 0.f };
	}
}

/**
 * @brief Initialize the render system
 *
 * @param width: the width of the window in pixels
 * @param height: the height of the window in pixels
 * @param title: the title of the window
 * @return true: if the render system is initialized successfully
 * @return false: if the render system is not initialized successfully
 */
bool RenderSystem::initialize(IInputHandler& input_handler,
	const int width, const int height, const char* title)
{
	glm::mat4 iso = glm::mat4(1.0f);
	iso = glm::rotate(iso, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // X rotation
	iso = glm::rotate(iso, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // Y rotation

	projectionMatrix = glm::ortho(0.f, (float)window_width_px * zoomFactor,
		(float)window_height_px * zoomFactor, 0.f, -1.f, 1.f);
	registry.projectionMatrix = projectionMatrix;

	initializeCamera();

	this->sound_manager = sound_manager;

	if (!glfwInit()) { // Initialize the window
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	this->window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!this->window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	setCustomCursor();

	glfwSetWindowUserPointer(this->window, this);

	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) {
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->input_handler->onKey(_0, _1, _2, _3);
		};

	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->input_handler->onMouseMove({ _0, _1 });
		};

	auto mouse_button_redirect = [](GLFWwindow* wnd, int _1, int _2, int _3) {
		((RenderSystem*)glfwGetWindowUserPointer(wnd))->input_handler->onMouseKey(wnd, _1, _2, _3);
		};

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	glfwMakeContextCurrent(this->window);
	glfwSwapInterval(1);

	const int is_fine = gl3w_init();
	if (is_fine) {
		fprintf(stderr, "failed to initialize OpenGL\n");
		return false;
	}
	assert(is_fine == 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(this->window);

	this->input_handler = &input_handler;
	this->input_handler->setRenderer(this);
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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


	if (globalOptions.tutorial && !this->isPlayingVideo()) {
		float titleFontSize = this->asset_manager->getFont("king")->size;
		float tutFontSize = this->asset_manager->getFont("deutsch")->size;
		float currentY = window_height_px - titleFontSize;

		drawText("Soulless", "king", window_width_px / 2.0f, currentY, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		currentY -= titleFontSize * 0.9f;

		vec3 color = glm::vec3(0.83f, 0.83f, 0.83f);

		vec3 selectedColor = glm::vec3(1.0f, 1.0f, 0.0f);
    
		drawText("Gameplay: 1", "deutsch", window_width_px / 2.0f - 300, currentY, 1.0f, globalOptions.showingTab == 1 ? selectedColor : color, true);
		drawText("Controls: 2", "deutsch", window_width_px / 2.0f - 100, currentY, 1.0f, globalOptions.showingTab == 2 ? selectedColor : color, true);
		drawText("Advanced: 3", "deutsch", window_width_px / 2.0f + 100, currentY, 1.0f, globalOptions.showingTab == 3 ? selectedColor : color, true);
		drawText("Spells: 4", "deutsch", window_width_px / 2.0f + 300, currentY, 1.0f, globalOptions.showingTab == 4 ? selectedColor : color, true);
		currentY -= tutFontSize * 1.5 + 20;


		switch (globalOptions.showingTab)
		{
		case 0:
			break;
		case 1:
			drawText("As a dark mage you must survive against an army of knights and archers.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("As time goes on new enemies will spawn with new strength and weaknesses.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Consuming campfires will heal you, but be careful, they don't spawn fast.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("You combat the enemies by casting spells. At the start you have one spell- Fire.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("You will find new spell collectibles on the map or dropped by enemies.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("You can cast two spells at a time, so choose wisely to drop or cast it.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Spells are leveled up by killing enemies with the spell or by picking it up off the ground.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Once a spell reaches its max level, it will evolve, gaining unique effects.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Progress can be seen at the bottom left corner of the screen.", "deutsch", 20, currentY, 1.0f, color, false);

			break;

		case 2:
			drawText("Move using: W, A, S, D.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;

			drawText("Aim with your mouse.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;

			drawText("Left click to shoot first spell. Right click for second.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;

			drawText("Press q to drop first spell. Press e to drop second.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Press shift + T to pause/show tutorial.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("While paused, press s to save or l to load game.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			break;
		case 3:
			drawText("Press shift + f to show FPS.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Press k to display collision boxes.", "deutsch", 20, currentY, 1.0f, color, false);
			break;
		case 4:
			drawText("Fire: shoots a fireball, damaging the first enemy hit; Max level: explodes on impact.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Water: shields the player and explodes; Max level: teleports the player and explodes twice.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Wind: damages enemies over time; Max level: pulls enemies towards it.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Lightning: after a short delay, damages enemies; Max level: spawns a chain of strikes.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Ice: shoots ice shards; Max level: shoots only one ice shard that pierces enemies.", "deutsch", 20, currentY, 1.0f, color, false);
			currentY -= tutFontSize * 1.5;
			drawText("Plasma: slowly accelerating arc that damages enemies it passes through. Not upgradable.", "deutsch", 20, currentY, 1.0f, color, false);
			break;
		}

		std::string message = "Press SPACE or click to ";
		std::string start = globalOptions.pause ? "resume." : "start.";
		drawText(message + start, "deutsch", window_width_px / 2.0f, tutFontSize * 1.5, 1.0f, selectedColor);
		return;
	}

	if (is_playing_video && video_player) {
		updateVideo();

		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		// Use simple orthographic projection for testing
		glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(w),
								   static_cast<float>(h), 0.0f, -1.0f, 1.0f);
		glm::mat4 view = glm::mat4(1.0f);  // Identity matrix

		video_player->draw(proj, view);
		return;
	}

	if (globalOptions.pause) {
		drawText("Paused", "king", window_width_px / 2.0f, window_height_px / 2.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		return;
	}

	Entity player = registry.players.entities[0];
	float playerX = registry.motions.get(player).position.x - window_width_px / 6.0;
	float playerY = registry.motions.get(player).position.y - window_height_px / 6.0;

	updateCameraPosition(clamp(playerX, 0.f, (float)(2.0 * window_width_px / 3.0)),
		clamp(playerY, 0.f, (float)(2.0 * window_height_px / 3.0)));

	//printd("Camera: X:%f Y: %f \n", registry.cameras.get(camera).position.x, registry.cameras.get(camera).position.y);
	//printd("Player: X:%f Y: %f \n", registry.motions.get(player).position.x, registry.motions.get(player).position.y);

	drawBackgroundObjects();

	// Draw all entities
	// registry.render_requests.sort(typeAscending);
	this->updateRenderOrder(registry.render_requests);

	for (const auto& render_index : sorted_indices)
	{
		Entity& entity = registry.render_requests.entities.at(render_index.index);
		RenderRequest& render_request = registry.render_requests.get(entity);

		if (!registry.motions.has(entity))
		{
			std::cerr << "Entity " << entity << " does not have a motion component" << std::endl;
			std::cerr << "Skipping rendering of this entity" << std::endl;
			continue;
		}

		Motion& motion = registry.motions.get(entity);

		Camera& gameCamera = registry.cameras.get(camera);
		if (motion.position.x < gameCamera.position.x - RENDER_PAST_SCREEN_OFFSET
			|| motion.position.x > gameCamera.position.x + 3.0 * window_width_px / 4.0 + RENDER_PAST_SCREEN_OFFSET
			|| motion.position.y < gameCamera.position.y - RENDER_PAST_SCREEN_OFFSET
			|| motion.position.y > gameCamera.position.y + 3.0 * window_height_px / 4.0 + RENDER_PAST_SCREEN_OFFSET) {
			continue;
		}

		if (render_request.shader != "") {
			const Shader* shader = this->asset_manager->getShader(render_request.shader);
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
			}
			else {
				// Fallback to render_y if no Motion component
				position = vec2(0, render_request.smooth_position.render_y);
			}

			mat4 transform = mat4(1.0f);
			vec2 isoPos = IsometricGrid::convertToIsometric(position);
			transform = translate(transform, glm::vec3(isoPos, 0.0f));

			// Rotate the sprite for all eight directions if request type is a PROJECTILE
			if (render_request.type == PROJECTILE) {
				// printd("Fireball angle: %f\n", motion.angle);
				transform = rotate(transform, motion.angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Apply rotation
			}

			transform = scale(transform, vec3(motion.scale * renderScaleModifier * zoomFactor, 1.0f));


			if (render_request.shader == "sprite" || render_request.shader == "animatedsprite") {
				
				const Texture* texture = this->asset_manager->getTexture(render_request.texture);
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
						
						if (animation.currentFrame - animation.startFrame >= animation.frameCount) {
							if (animation.state == AnimationState::DYING) {
								// "Freeze" at last frame until entity gets removed
								animation.currentFrame -= 1;
								animation.frameTime = 7000.f;
							}
							else {
								animation.currentFrame = animation.startFrame;

								if (animation.oneTime) {
									animation.state = AnimationState::IDLE;
									animation.frameTime = DEFAULT_LOOP_TIME;
									animation.oneTime = false;
								}
							}
						}
					}

					glUniform1i(glGetUniformLocation(shaderProgram, "slow"), false);

					// TODO: Rework this, maybe make an Invisible component
					if (registry.onHeals.has(registry.players.entities[0]) && registry.interactables.has(entity)) {
						glUniform1i(glGetUniformLocation(shaderProgram, "visible"), 0);
					}
					else {
						glUniform1i(glGetUniformLocation(shaderProgram, "visible"), 1);
					}

					if (registry.players.has(entity) && registry.onHits.has(entity) && registry.onHits.get(entity).isInvincible) {
						if (registry.onHits.get(entity).invicibilityShader) {
							glUniform1i(glGetUniformLocation(shaderProgram, "state"), 2);
						}
						else {
							glUniform1i(glGetUniformLocation(shaderProgram, "state"), 1);
						}

						if (registry.debuffs.has(entity) && registry.debuffs.get(entity).type == DebuffType::SLOW) {
							glUniform1i(glGetUniformLocation(shaderProgram, "slow"), true);
						}
					}
					else if (registry.enemies.has(entity) && registry.onHits.has(entity) && registry.onHits.get(entity).isInvincible)
					{
						if (registry.onHits.get(entity).invicibilityShader) glUniform1i(glGetUniformLocation(shaderProgram, "state"), 1);
						else glUniform1i(glGetUniformLocation(shaderProgram, "state"), 0);
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

		const Mesh* mesh = this->asset_manager->getMesh(render_request.mesh);

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
	}

	drawParticles();
	drawHealthBars();
	drawHUD();

	for (const Entity& debug_entity : registry.debug_requests.entities)
	{
		DebugRequest& debug = registry.debug_requests.get(debug_entity);

		mat4 transform = mat4(1.0f);
		transform = translate(transform, vec3({ debug.position, 1.0f }));
		transform = rotate(transform, debug.angle, glm::vec3(0.0f, 0.0f, 1.0f));
		transform = scale(transform, vec3(debug.collider, 1.0f));

		const Shader* shader = this->asset_manager->getShader("debug");
		const GLuint shaderProgram = shader->program;
		glUseProgram(shaderProgram);

		const GLint colorLoc = glGetUniformLocation(shaderProgram, "color_override");
		glUniform3fv(colorLoc, 1, glm::value_ptr(debug.color));

		const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		const GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(registry.viewMatrix));
		const GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(registry.projectionMatrix));
		gl_has_errors();

		const Mesh* debug_mesh = asset_manager->getMesh("debug");

		glBindVertexArray(debug_mesh->vao);
		GLenum draw_type = GL_LINE_LOOP;
		if (debug.type == DebugType::fill)
		{
			draw_type = GL_TRIANGLES;
		}
		glDrawElements(draw_type, debug_mesh->indexCount, GL_UNSIGNED_INT, 0);
		gl_has_errors();
	}

	// TODO: does this work with the camera????
	if (globalOptions.showFps) {
		drawText(std::to_string(globalOptions.fps), "deutsch", window_width_px - 100.0f, window_height_px - 50.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	Player& playerObj = registry.players.get(player);
	SpellQueue& spell_queue = playerObj.spell_queue;
}

// source: inclass SimpleGL-3
// the x,y values should be the position we want the center of our text to be
void RenderSystem::drawText(const std::string& text, const std::string& fontName, float x, float y, float scale, const glm::vec3& color, bool centered) {
	Font* font = this->asset_manager->getFont(fontName);
	float textWidth = getTextWidth(text, fontName, scale);

	if (centered)
	{
		x = x - textWidth / 2;
	}
	y = y - font->size / 2;

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, glm::vec3(0.0, 0, 0.0));
	trans = glm::rotate(trans, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(1.0f, 1.0f, 1.0f));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const Shader* fontShader = this->asset_manager->getShader("font");
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
	const Font* font = this->asset_manager->getFont(fontName);
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

void RenderSystem::drawParticles() {
	const Shader* shader = this->asset_manager->getShader("particle");
	const GLuint shaderProgram = shader->program;
	glUseProgram(shaderProgram);

	mat4 projection = projectionMatrix;
	mat4 view = viewMatrix;
	const GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	const GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	const Mesh* mesh = this->asset_manager->getMesh("particle");
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->instanceVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * registry.particles.size(), registry.particles.components.data());

	glDrawElementsInstanced(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0, registry.particles.size());
	//printd("Current Particles: %d\n", registry.particles.size());
}

void RenderSystem::updateCameraPosition(float x, float y) {
	Camera& gameCamera = registry.cameras.get(camera);
	gameCamera.position.x = 3 * x / 4;
	gameCamera.position.y = 3 * y / 4;

	viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-gameCamera.position, 0.0f));
	viewMatrix = glm::scale(viewMatrix, glm::vec3(0.75, 0.75, 1.0));
	registry.viewMatrix = viewMatrix;
}


void RenderSystem::drawBackgroundObjects() {
	for (const auto& sub_renderer : sub_renderers) {
		sub_renderer.second->render(this);
	}
}

void RenderSystem::drawHealthBars() {

	for (Entity e : registry.healthBars.entities) {

		if (!registry.healthBars.has(e) || !registry.healthBars.get(e).assigned) {
			assert("Healthbar shader can only be used on entities with an assigned HealthBar component.\n");
		}

		HealthBar& healthBar = registry.healthBars.get(e);

		const Shader* shader = this->asset_manager->getShader("healthbar");
		if (!shader)
		{
			printf("Could not find shader with id healthbar\n");
			printf("Skipping rendering of this shader\n");
			continue;
		}
		const GLuint shaderProgram = shader->program;
		glUseProgram(shaderProgram);

		const Texture* texture = this->asset_manager->getTexture("healthbar");
		if (!texture)
		{
			std::cerr << "Texture with id healthbar not found!" << std::endl;
			continue;
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->handle);
		glUniform1i(glGetUniformLocation(shader->program, "image"), 0);

		if (!registry.healths.has(healthBar.assignedTo)) {
			printd("Entity %d has a health bar, but no health component.\n", healthBar.assignedTo);
			continue;
		}
		Health& health = registry.healths.get(healthBar.assignedTo);

		glUniform1f(glGetUniformLocation(shaderProgram, "proportionFilled"), health.health / health.maxHealth);

		mat4 transform = mat4(1.0f);
		transform = translate(transform, glm::vec3(healthBar.position, 0.0f));
		transform = scale(transform, glm::vec3(healthBar.scale * renderScaleModifier * zoomFactor, 1.0f));
		mat4 projection = projectionMatrix;
		mat4 view = viewMatrix;

		const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		const GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		const GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		const Mesh* mesh = this->asset_manager->getMesh("sprite");

		if (!mesh)
		{
			std::cerr << "Mesh with id sprite not found!" << std::endl;
			std::cerr << "Skipping rendering of this mesh" << std::endl;
			continue;
		}

		glBindVertexArray(mesh->vao);
		glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << error << std::endl;

		}
	}
}

void RenderSystem::updateRenderOrder(ComponentContainer<RenderRequest>& render_requests) {
	sorted_indices.clear();
	sorted_indices.reserve(render_requests.components.size());

	for (size_t i = 0; i < render_requests.components.size(); ++i) {
		RenderRequest& request = render_requests.components[i];

		// sort by using request type and render_y
		float combined_value = request.type * 10000.0f + request.smooth_position.render_y;
		sorted_indices.emplace_back(i, combined_value);
	}

	std::sort(sorted_indices.begin(), sorted_indices.end(),
		[&render_requests](const RenderIndex& a, const RenderIndex& b) {
			const auto& request_a = render_requests.components[a.index];
			const auto& request_b = render_requests.components[b.index];

			if (request_a.type != request_b.type) {
				return request_a.type < request_b.type;
			}
			return a.render_y < b.render_y;
		});
}

/**
 * @brief Draw textured elements onto screen.
 * Translation in screen coordinates (pixels).
 * Scale in decimal (0.f - 1.f)
 */
void RenderSystem::drawHUDElement(std::string textureId, vec2 translation, vec2 scale)
{
	const Shader* shader = this->asset_manager->getShader("screen");
	if (!shader)
	{
		std::cerr << "Could not find shader with id sprite" << std::endl;
		return;
	}
	const Texture* texture = this->asset_manager->getTexture(textureId);
	const GLuint shaderProgram = shader->program;
	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->handle);
	glUniform1i(glGetUniformLocation(shaderProgram, "image"), 0);

	mat4 transform = mat4(1.0f);

	transform = glm::translate(transform, vec3({ translation.x, translation.y, 0.f }));
	transform = glm::scale(transform, vec3(scale.x, scale.y, 1.f));
	transform = glm::ortho(0.f, (float)window_width_px, (float)window_height_px, 0.0f) * transform;

	const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	const Mesh* mesh = this->asset_manager->getMesh("sprite");

	if (!mesh)
	{
		std::cerr << "Could not find mesh with id sprite" << std::endl;
		return;
	}

	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
	gl_has_errors();
}

void RenderSystem::drawCooldownElement(vec2 translation, vec2 scale)
{
	const Shader* shader = this->asset_manager->getShader("basic");
	if (!shader)
	{
		std::cerr << "Could not find shader with id sprite" << std::endl;
		return;
	}
	const GLuint shaderProgram = shader->program;
	glUseProgram(shaderProgram);

	mat4 transform = mat4(1.f);
	transform = glm::translate(transform, vec3({ translation.x, translation.y, 0.f }));
	transform = glm::scale(transform, vec3(scale.x, scale.y, 1.f));
	transform = glm::ortho(0.f, (float)window_width_px, (float)window_height_px, 0.0f) * transform;

	const GLint opacity_loc = glGetUniformLocation(shaderProgram, "opacity");
	glUniform1f(opacity_loc, 0.8f);
	const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	const GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(mat4(1.f)));
	const Mesh* mesh = this->asset_manager->getMesh("square");

	if (!mesh)
	{
		std::cerr << "Could not find mesh with id square" << std::endl;
		return;
	}

	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
	gl_has_errors();
}

void RenderSystem::drawCooldown(const Player& player)
{
	if (player.leftCooldown > 0)
	{
		float percent = player.leftCooldown / player.leftCooldownTotal;
		drawCooldownElement(LEFT_COOLDOWN_TRANSLATE, COOLDOWN_SCALE * percent);
	}
	if (player.rightCooldown > 0)
	{
		float percent = player.rightCooldown / player.rightCooldownTotal;
		drawCooldownElement(RIGHT_COOLDOWN_TRANSLATE, COOLDOWN_SCALE * percent);
	}
}

void RenderSystem::drawSpellProgress(Player& player)
{
	const Shader* shader = this->asset_manager->getShader("progressbar");
	if (!shader)
	{
		std::cerr << "Could not find shader with id sprite" << std::endl;
		return;
	}
	const GLuint shaderProgram = shader->program;
	glUseProgram(shaderProgram);

	vec3 bar_translate = LEFTMOST_GAUGE_POS;
	vec3 spell_translate = LEFTMOST_GAUGE_TEXT_POS;

	for (int i = 0; i < static_cast<int>(SpellType::COUNT); i++) {
		SpellType type = static_cast<SpellType>(i);

		// plasma doesn't level
		if (type == SpellType::PLASMA) continue;

		const int level = player.spell_queue.getSpellLevel(type);
		bool is_max_level = level >= MAX_SPELL_LEVEL;
		const int progress = player.spell_queue.getSpellUpgradeTrack(type);

		if (level < 1) continue;

		glUseProgram(shaderProgram);
		const vec3 color = spellTypeToColor(type);

		mat4 transform = mat4(1.f);
		transform = glm::translate(transform, bar_translate);
		transform = glm::scale(transform, GAUGE_SCALE);
		transform = glm::ortho(0.f, (float)window_width_px, (float)window_height_px, 0.0f) * transform;

		const GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		const GLint progress_loc = glGetUniformLocation(shaderProgram, "progress");
		glUniform1f(progress_loc, is_max_level ? 1.f : (float)progress / (float)UPGRADE_KILL_COUNT[level - 1]);
		const GLint vertical_loc = glGetUniformLocation(shaderProgram, "is_vertical");
		glUniform1i(vertical_loc, true);
		const GLint color_loc = glGetUniformLocation(shaderProgram, "progress_color");
		glUniform3f(color_loc, color.r, color.g, color.b);

		const float darken = NON_PROGRESS_DARKEN_FACTOR;
		const GLint non_color_loc = glGetUniformLocation(shaderProgram, "non_progress_color");
		glUniform3f(non_color_loc, color.r + darken, color.g + darken, color.b + darken);

		const Mesh* mesh = this->asset_manager->getMesh("uncoloredSquare");
		if (!mesh)
		{
			std::cerr << "Could not find mesh with id square" << std::endl;
			return;
		}

		glBindVertexArray(mesh->vao);
		glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
		gl_has_errors();

		drawText(is_max_level ? "*" : std::to_string(level), "king", spell_translate.x, spell_translate.y, 0.3, is_max_level ? color + NON_PROGRESS_DARKEN_FACTOR : color + TEXT_LIGHTEN_FACTOR, false);

		bar_translate.x += GAUGE_SPACING;
		spell_translate.x += GAUGE_SPACING;
	}
}

/**
 * @brief Draw HUD elements
 */
void RenderSystem::drawHUD()
{
	vec2 translate_spells = TRANSLATE_QUEUE_SPELLS;
	// Spell Queue Bar Rendering
	drawHUDElement("queue", QUEUE_TRANSLATE, QUEUE_SCALE);


	// Spells in Queue Rendering
	Player& player = registry.players.get(registry.players.entities[0]); // hard-coded player get
	const SpellQueue spell_queue = player.spell_queue;
	const std::deque<SpellType>& queue = spell_queue.getQueue();
	int count = 0;

	for (SpellType spell : queue)
	{
		drawHUDElement(spellTypeToCollect(spell), translate_spells, SCALE_QUEUE_SPELLS);
		translate_spells.x += QUEUE_SPACING;
		count++;
	}

	// Spells in L Hand Rendering
	drawHUDElement(spellTypeToCollect(spell_queue.getLeftSpell()), LEFT_SLOT_TRANSLATE, SCALE_QUEUE_SPELLS);

	// Spells in R Hand Rendering
	drawHUDElement(spellTypeToCollect(spell_queue.getRightSpell()), RIGHT_SLOT_TRANSLATE, SCALE_QUEUE_SPELLS);

	// Gauge Rendering
	drawHUDElement("gauge", GAUGE_TEXTURE_TRANSLATE, GAUGE_TEXTURE_SCALE);

	drawSpellProgress(player);
	drawCooldown(player);

}

void RenderSystem::setCustomCursor() {

	std::string cursorPath = textures_path("cursor") + ".png";
	// std::cout << "Loading cursor directly from file: " << cursorPath << std::endl;

	GLFWimage cursorImage;
	cursorImage.pixels = stbi_load(cursorPath.c_str(), &cursorImage.width, &cursorImage.height, 0, 4);
	// std::cout << "Cursor image loaded: " << cursorImage.width << "x" << cursorImage.height << std::endl;
	if (!cursorImage.pixels) {
		// std::cerr << "Failed to load cursor image from: " << cursorPath << std::endl;
		return;
	}

	// cursor hotspot is TOP LEFT
	cursor = glfwCreateCursor(&cursorImage, 0, 0);
	if (!cursor) {
		std::cerr << "Failed to create custom cursor!" << std::endl;
		stbi_image_free(cursorImage.pixels); // Free memory if cursor creation fails
		return;
	}

	// Set the custom cursor for the current window
	glfwSetCursor(this->window, cursor);

	// Free the image data as GLFW copies it internally
	stbi_image_free(cursorImage.pixels);
}



bool RenderSystem::playVideo(const std::string& filename) {
	static bool ffmpeg_initialized = false;
	if (!ffmpeg_initialized) {
	#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
			// Old FFmpeg version (before 4.0)
			av_register_all();
	#endif
			avformat_network_init();
			ffmpeg_initialized = true;
	}


	// Create video player if it doesn't exist
	if (!video_player) {
		video_player = std::make_unique<VideoPlayer>();
	}

	// Get or create video shader
	const Shader* video_shader = this->asset_manager->getShader("video");
	if (!video_shader) {
		// Load video shader if not already loaded
		this->asset_manager->loadShader("video", shader_path("video") + ".vs.glsl", shader_path("video") + ".fs.glsl");
		video_shader = this->asset_manager->getShader("video");
	}

	// Initialize video player with the file
	if (!video_player->initialize(filename, video_shader->program)) {
		std::cerr << "Failed to initialize video player with file: " << filename << std::endl;
		return false;
	}

	is_playing_video = true;
	return true;
}

void RenderSystem::stopVideo() {
	if (video_player) {
		video_player->cleanup();
		is_playing_video = false;
		globalOptions.pause = false;
		SoundManager* soundManager = SoundManager::getSoundManager();
		soundManager->playMusic(Song::MAIN);
	}
}

void RenderSystem::updateVideo() {
	if (!is_playing_video || !video_player) return;

	if (!video_player->readFrame()) {
		// End of video or error
		stopVideo();
		return;
	}
}

bool RenderSystem::isPlayingVideo() const {
	return is_playing_video;
}

void RenderSystem::playCutscene(const std::string& filename, Song song) {
	globalOptions.pause = true;
	stopVideo();
	SoundManager* soundManager = SoundManager::getSoundManager();
	soundManager->playMusic(song);
	this->playVideo(filename);

}