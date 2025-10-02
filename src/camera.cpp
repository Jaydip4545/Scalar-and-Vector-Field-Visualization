#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(int windowWidth, int windowHeight) 
    : isDragging(false), lastMouseX(0.0), lastMouseY(0.0), 
      yaw(45.0f), pitch(-35.0f), zoom(250.0f) {}

glm::mat4 Camera::getViewMatrix() {
    glm::mat4 view = glm::mat4(1.0f);
    // 1. Move the camera away from the origin (zooming out).
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -this->zoom));
    // 2. Apply pitch rotation (up/down) around the X-axis.
    view = glm::rotate(view, glm::radians(this->pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    // 3. Apply yaw rotation (left/right) around the Y-axis.
    view = glm::rotate(view, glm::radians(this->yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    return view;
}

void Camera::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            this->isDragging = true;
            glfwGetCursorPos(window, &this->lastMouseX, &this->lastMouseY);
        } else if (action == GLFW_RELEASE) {
            this->isDragging = false;
        }
    }
}

void Camera::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (this->isDragging) {
        float xoffset = xpos - this->lastMouseX;
        float yoffset =  ypos - this->lastMouseY; 
        this->lastMouseX = xpos;
        this->lastMouseY = ypos;

        float sensitivity = 0.2f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        this->yaw += xoffset;
        this->pitch += yoffset;

        if (this->pitch > 89.0f) this->pitch = 89.0f;
        if (this->pitch < -89.0f) this->pitch = -89.0f;
    }
}

void Camera::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    this->zoom -= (float)yoffset;
    if (this->zoom < 1.0f) this->zoom = 1.0f;
    if (this->zoom > 2000.0f) this->zoom = 2000.0f;
}

void Camera::setZoom(float newZoom) {
    this->zoom = newZoom;
    if (this->zoom < 1.0f) this->zoom = 1.0f;
    if (this->zoom > 2000.0f) this->zoom = 2000.0f;
}
