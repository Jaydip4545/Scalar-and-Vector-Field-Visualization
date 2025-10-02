#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:
    Camera(int windowWidth, int windowHeight);
    glm::mat4 getViewMatrix();

    // Input handlers
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // Public setter for zoom
    void setZoom(float newZoom);

private:
    bool isDragging;
    double lastMouseX, lastMouseY;
    float yaw;
    float pitch;
    float zoom;
};

#endif // CAMERA_H
