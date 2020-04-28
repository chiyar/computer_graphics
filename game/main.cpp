// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <iostream>  // for debugging

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "controls.hpp"
#include "objects.hpp"
#include "common/texture.hpp"
#include "common/shader.hpp"


GLFWwindow* initialize() {
    // Initialise GLFW
    if(!glfwInit()) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        exit(-1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow( 1024, 768, "GoChi", NULL, NULL);
    if(window == NULL) {
        fprintf( stderr, "Failed to open GLFW window.\n" );
        getchar();
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        exit(-1);
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwSetCursorPos(window, 1024/2, 768/2);

    // background
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
//    glEnable(GL_CULL_FACE);  todo ENABLE after debug

    return window;
}


bool is_too_far(const Object& object) {
    return glm::distance(Controls::position, object.center) > 10.0f;
}

template <typename U, typename V>
bool are_close(const U& lhs, const V& rhs) {
    return glm::distance(lhs.center, rhs.center) < lhs.radius + rhs.radius;
}


std::default_random_engine generator;
std::uniform_real_distribution<float> uniform(0.0, 1.0);

void create_target(std::vector<Target>& targets, std::vector<glm::vec3>& speeds, int cur_ts) {
    float x = uniform(generator) * 2 * 3.14;
    float h = uniform(generator);
    glm::vec3 center(5 * sin(x), 0.1 + 3 * h, 5 * cos(x));
    GLfloat radius = 0.1f + 0.05 * uniform(generator);
    glm::vec3 angle(
            uniform(generator) * 3.14,
            uniform(generator) * 3.14,
            uniform(generator) * 3.14
    );
    std::vector<GLfloat> color({
           uniform(generator),
           uniform(generator),
           uniform(generator)
    });
    float brightness = std::accumulate(color.begin(), color.end(), 0.f);
    targets.emplace_back(center + Controls::position * 0.5f, radius, angle, color,
            cur_ts + brightness * 1000);
    speeds.emplace_back(
            uniform(generator) / 100,
            uniform(generator) / 100,
            uniform(generator) / 100
            );
}

template <typename T>
void remove_object(std::vector<T>& objects, std::vector<glm::vec3>& speeds, int id=0) {
    if (objects.size() > id) {
        objects.erase(objects.begin() + id);
        speeds.erase(speeds.begin() + id);
    }
}


void create_fireball(std::vector<Fireball>& fireballs, std::vector<glm::vec3>& speeds,
    const glm::vec3& direction) {
    auto fireball = Fireball(0.5, 20);
    fireball.move(Controls::position - glm::vec3(0, 1, 0));
    fireballs.emplace_back(fireball);
    speeds.emplace_back(direction * 0.5f);
}


bool fireball_is_available(size_t iteration, size_t last_shoot_time) {
    return (iteration - last_shoot_time > 20);
}


int main() {
    GLFWwindow* window = initialize();

    // Create and compile our GLSL program from the shaders
    GLuint ProgramID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");

//     Get a handle for our buffers
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(ProgramID, "vertexPosition_modelspace");
    GLuint vertexColorID = glGetAttribLocation(ProgramID, "vertexColor");
    GLuint vertexUVID = glGetAttribLocation(ProgramID, "vertexUV");


    std::vector<Target> targets;
    std::vector<glm::vec3> target_speeds;
    std::vector<Fireball> fireballs;
    std::vector<glm::vec3> fireball_speeds;

    Buffer buffer;
    Floor floor;

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);

    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);

    // Load the texture using any two methods
    //GLuint Texture = loadBMP_custom("uvtemplate.bmp");
    GLuint Texture = loadBMP_custom("fireearth.bmp");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(ProgramID, "myTextureSampler");

    size_t iteration = 0;
    size_t last_shoot_time = 0;
    do {
        buffer.clear();

        // create targets
        if (uniform(generator) < 0.3) {  // 0.03
            create_target(targets, target_speeds, iteration);
        }

        // remove objects that are too far
        for (size_t i = 0; i < targets.size(); ++i) {
            if (targets[i].expired(iteration)) {
                remove_object(targets, target_speeds, i);
            }
        }

        bool has_collision = false;
        // remove collided objects
        for (size_t i = 0; i < targets.size(); ++i) {
            for (size_t j = 0; j < fireballs.size(); ++j) {
                if (are_close(targets[i], fireballs[j])) {
                    std::cout << "COLLIDE" << std::endl;
                    remove_object(targets, target_speeds, i);
                    remove_object(fireballs, fireball_speeds, j);
                    has_collision = true;
                    break;
                }
            }
        }
        if (has_collision) {
            glClearColor(1.0f, 1.0f, 0.2f, 0.0f);
        } else {
            glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        }


        if (Controls::isSpacePressed(window) && fireball_is_available(iteration, last_shoot_time)) {
            last_shoot_time = iteration;
            std::cout << "Fire!\n";
            create_fireball(fireballs, fireball_speeds, Controls::direction);
        }

        floor.draw(buffer);
        for (size_t i = 0; i < targets.size(); ++i) {
            targets[i].move(target_speeds[i]);
            targets[i].draw(buffer);
        }

        for (size_t i = 0; i < fireballs.size(); ++i) {
            fireballs[i].move(fireball_speeds[i]);
            fireballs[i].draw(buffer);
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get position from controls
        Controls::computeMatricesFromInputs(window);
        glm::mat4 ProjectionMatrix = Controls::getProjectionMatrix();
        glm::mat4 ViewMatrix = Controls::getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glUseProgram(ProgramID);
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glUniform1i(TextureID, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(vertexPosition_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * buffer.size(), buffer.vertex_data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(vertexColorID);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * buffer.size(), buffer.color_data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertexColorID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 3rd attribute buffer : textures
        glEnableVertexAttribArray(vertexUVID);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * buffer.texture_size(), buffer.texture_data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_TRIANGLES, 0, buffer.size() / 3);

        glDisableVertexAttribArray(vertexPosition_modelspaceID);
        glDisableVertexAttribArray(vertexColorID);
        glDisableVertexAttribArray(vertexUVID);
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        ++iteration;
        // todo count time not iterations

    } // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
          && glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteTextures(1, &Texture);
    glDeleteProgram(ProgramID);
//    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}
