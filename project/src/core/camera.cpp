#include "core/camera.hpp"
#include "entities/ecs_registry.hpp"
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/matrix_transform.hpp>

// centers camera on player's current position
void Camera::updateCamera(Camera& camera, Entity& player) {
	camera.setPosition(registry.motions.get(player).position.x - window_width_px * 0.5f, registry.motions.get(player).position.y - window_height_px * 0.5f);
}

// sets camera position to (x, y)
void Camera::setPosition(float x, float y) {
	position = glm::vec2(x, y);
	updateViewMatrix();
}

// translate view matrix to camera position
void Camera::updateViewMatrix() {
	viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
}

// set orthographic projection matrix
void Camera::setOrthographicProjection() {
	projectionMatrix = glm::ortho(0.f, (float)window_width_px, (float)window_height_px, 0.f, -1.f, 1.f);
}
