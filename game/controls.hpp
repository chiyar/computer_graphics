#pragma once

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Controls {

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
    return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
    return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 2, 0 );
float direction_up = 0;
float direction_right = 0;
glm::vec3 direction = glm::vec3(0, 0, 1);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;


bool isSpacePressed(GLFWwindow* window) {
    return glfwGetKey(window, GLFW_KEY_SPACE ) == GLFW_PRESS;
}

void computeMatricesFromInputs(GLFWwindow* window){

    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    // Get mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Reset mouse position for next frame
    glfwSetCursorPos(window, 1024/2, 768/2);
    // Compute new orientation
    horizontalAngle += mouseSpeed * float(1024/2 - xpos );
    verticalAngle   += mouseSpeed * float( 768/2 - ypos );

    float C = 90.;
    // Direction vectors
    glm::vec3 right_vec(-cos(direction_right / C), 0, sin(direction_right / C));
    glm::vec3 forward_vec(sin(direction_right / C), 0, cos(direction_right / C));
    glm::vec3 up = glm::vec3(0, 1, 0);

    // Move forward
    if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
        position += forward_vec * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
        position -= forward_vec * deltaTime * speed;
    }
    // Move right
    if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
        position += right_vec * deltaTime * speed;
    }
    // Move left
    if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
        position -= right_vec * deltaTime * speed;
    }
    // Turn up
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
        direction_up += 1;
    }
    // Turn down
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
        direction_up -= 1;
    }
    // Turn right
    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
        direction_right -= 1;
    }
    // Turn left
    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
        direction_right += 1;
    }

    direction = glm::vec3(
            sin(direction_right / C),
            direction_up / C,
            cos(direction_right / C)
    );


    float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    ViewMatrix       = glm::lookAt(
            position,           // Camera is here
            position+direction, // and looks here : at the same position, plus "direction"
            up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}
}  // namespace Controls

