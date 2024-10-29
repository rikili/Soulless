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

std::string readShaderFile(const std::string& filename)
{
	// std::cout << "Loading shader filename: " << filename << std::endl;

	std::ifstream ifs(filename);

	if (!ifs.good())
	{
		std::cerr << "ERROR: invalid filename loading shader from file: " << filename << std::endl;
		return "";
	}

	std::ostringstream oss;
	oss << ifs.rdbuf();
	// std::cout << oss.str() << std::endl;
	return oss.str();
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
bool RenderSystem::initialize(InputHandler& input_handler, const int width, const int height, const char* title)
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

// source: in class simpleGL-3
// TODO: do this in loadFont (so we can have more than one!)
void RenderSystem::setUpFont() {
	std::string font_filename = PROJECT_SOURCE_DIR +
	std::string("data/fonts/Deutsch.ttf");
	unsigned int font_default_size = 48;

	std::string vertexShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.vs.glsl"));
	std::string fragmentShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.fs.glsl"));
	const char* vertexShaderSource_c = vertexShaderSource.c_str();
	const char* fragmentShaderSource_c = fragmentShaderSource.c_str();

	// enable blending or you will just get solid boxes instead of text
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// font buffer setup
	glGenVertexArrays(1, &m_font_VAO);
	glGenBuffers(1, &m_font_VBO);

	// font vertex shader
	unsigned int font_vertexShader;
	font_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(font_vertexShader, 1, &vertexShaderSource_c, NULL);
	glCompileShader(font_vertexShader);

	// font fragement shader
	unsigned int font_fragmentShader;
	font_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(font_fragmentShader, 1, &fragmentShaderSource_c, NULL);
	glCompileShader(font_fragmentShader);

	// font shader program
	m_font_shaderProgram = glCreateProgram();
	glAttachShader(m_font_shaderProgram, font_vertexShader);
	glAttachShader(m_font_shaderProgram, font_fragmentShader);
	glLinkProgram(m_font_shaderProgram);

	// apply orthographic projection matrix for font, i.e., screen space
	glUseProgram(m_font_shaderProgram);
	mat4 projection = glm::ortho(0.f, (float)window_width_px, 0.0f, (float)window_height_px);

	GLint project_location = glGetUniformLocation(m_font_shaderProgram, "projection");
	assert(project_location > -1);
	// std::cout << "project_location: " << project_location << std::endl;
	glUniformMatrix4fv(project_location, 1, GL_FALSE, glm::value_ptr(projection));

	// clean up shaders
	glDeleteShader(font_vertexShader);
	glDeleteShader(font_fragmentShader);

	// init FreeType fonts
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		// return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, font_filename.c_str(), 0, &face))
	{
		std::cerr << "ERROR::FREETYPE: Failed to load font: " << font_filename << std::endl;
		// return false;
	}

	// extract a default size
	FT_Set_Pixel_Sizes(face, 0, font_default_size);

	// disable byte-alignment restriction in OpenGL
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load each of the chars - note only first 128 ASCII chars
	for (unsigned char c = (unsigned char)0; c < (unsigned char)128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}

		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// std::cout << "texture: " << c << " = " << texture << std::endl;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned int>(face->glyph->advance.x),
			(char)c
		};
		m_ftCharacters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// clean up
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// bind buffers
	glBindVertexArray(m_font_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	// release buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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
		// TODO: save text... ? render everything saved? idk how we want to handle this...
		// TODO: have different font sizes?
		// could just drawText manually whenever we need to
		// TODO: make a method for this!
		// TODO: make 48.0f / default size?
		// use a different font...
		// 
		// maybe do init font for each type of font we want to use and add respective VBO and VAO and Character??
		// we can then pick which font ttf to use + font size
		// need to load shader like armin did... (not like assignment) -> maybe give each text object a shader
		// font object wont have colour or string but will have everything else needed?
		// OR can we change the font size manually??? idk. we may only have a few fonts to loads...
		drawText("Welcome to Soulless!", window_width_px / 3.0f, window_height_px / 2.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		drawText("Press any key to start", window_width_px / 3.0f, window_height_px / 2.0f - 48.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
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
			transform = scale(transform, vec3(motion.scale * 100.f, 1.0f));


			if (render_request.shader == "sprite") {
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

	// TODO: does this work with the camera????
	if (globalOptions.showFps) {
		drawText(std::to_string(globalOptions.fps), window_width_px - 100.0f, window_height_px - 50.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	// makes it kinda slow
	// for (Entity entity : registry.enemies.entities) {
	// 	if (registry.healths.has(entity) && registry.motions.has(entity)) {
	// 		Motion &motion = registry.motions.get(entity);
	// 		Health &health = registry.healths.get(entity);
	//
	// 		int percentage = static_cast<int>((health.health / health.maxHealth) * 100);
	//
	// 		drawText(std::to_string(percentage) + "%", motion.position.x - 50.0f, window_height_px - motion.position.y + 48.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	// 	}
	// }
}

// insperation from in class excercise
void RenderSystem::drawText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
	// TODO: init, where is VAO, and VBO for font?
	// make it so that top left is 0,0?
	// new lines ? better formatting automation?
	// y = (float) window_height_px - y + 50.0f;
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, glm::vec3(0.0, 0, 0.0));
	trans = glm::rotate(trans, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(1.0f, 1.0f, 1.0f));

	// const Shader* fontShader = this->asset_manager.getShader("font");
	// GLuint m_font_shaderProgram = fontShader->program;
	// glUseProgram(fontShader->program);

	glUseProgram(m_font_shaderProgram);

	GLint textColor_location =
		glGetUniformLocation(m_font_shaderProgram, "textColor");
	glUniform3f(textColor_location, color.x, color.y, color.z);

	GLint transformLoc =
		glGetUniformLocation(m_font_shaderProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	glBindVertexArray(m_font_VAO);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_ftCharacters[*c];

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

		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
