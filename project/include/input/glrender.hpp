/*
 * SimpleGL - a simple OpenGL 'hello triangle'
 */
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// glfw (OpenGL)
#define NOMINMAX
#include <GLFW/glfw3.h>

// matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <assert.h>
#include <fstream>			// for ifstream
#include <sstream>			// for ostringstream

#include "../ext/project_path.hpp"		// built by CMake, contains project path

std::string readShaderFile(const std::string& filename)
{
	std::cout << "Loading shader filename: " << filename << std::endl;

	std::ifstream ifs(filename);

	if (!ifs.good())
	{
		std::cerr << "ERROR: invalid filename loading shader from file: " << filename << std::endl;
		return "";
	}

	std::ostringstream oss;
	oss << ifs.rdbuf();
	std::cout << oss.str() << std::endl;
	return oss.str();
}

class GLWindow
{
private:
	int m_window_width_px;
	int m_window_height_px;

	bool m_close_window = false;

public:

	GLFWwindow* window;

	GLWindow()
	{
		// use default values
		m_window_width_px = 640;
		m_window_height_px = 480;
		window = create_window();
	}

	GLWindow(int window_width_px, int window_height_px)
		: m_window_width_px(window_width_px), m_window_height_px(window_height_px)
	{
		window = create_window();
	}

	~GLWindow()
	{
		if (window) {
			// close and release the window
			glfwDestroyWindow(window);
		}
	}

	int window_width_px() { return m_window_width_px; }
	int window_height_px() { return m_window_height_px; }

	GLFWwindow* create_window()
	{
		// init GLFW first
		if (!glfwInit()) {
			std::cerr << "ERROR: Failed to initialize GLFW" << std::endl;
			return nullptr;
		}

		// OpenGL config -- version 3.3 core
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		// create the window
		window = glfwCreateWindow(m_window_width_px, m_window_height_px, "SOULLESS", nullptr, nullptr);
		if (window == nullptr) {
			std::cerr << "ERROR: Failed to glfwCreateWindow" << std::endl;
			// glfwTerminate();
			return nullptr;
		}


		// register calls for key presses
		// Setting callbacks to member functions (that's why the redirect is needed)
		// Input is handled using GLFW, for more info see
		// http://www.glfw.org/docs/latest/input_guide.html
		glfwSetWindowUserPointer(window, this);
		auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((GLWindow*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
		glfwSetKeyCallback(window, key_redirect);

		return window;
	}

	bool shouldClose() {
		return bool(glfwWindowShouldClose(window)) || m_close_window;
	}

	void on_key(int key, int, int action, int mod) {

		(void)mod;	// remove unused warning

		// ESC key to exit
		if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
			std::cout << "INFO: ESC key pressed, close window" << std::endl;
			m_close_window = true;
		}
	}
};

class GLRender {

private:
	GLuint m_shaderProgram;
	GLuint m_VAO;
	GLuint m_VBO;

public:

	~GLRender() {
		// clean up
		glDeleteProgram(m_shaderProgram);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_VAO);
	}

	void init(GLWindow& window) {

	}

	void render(GLWindow& window) {
	}
};
