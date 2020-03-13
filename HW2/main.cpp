// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

glm::mat4 build_mvp_matrix(int i) {
    // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

    // Camera matrix
    glm::mat4 View       = glm::lookAt(
        glm::vec3(sin(i / 200 % 360), 2, cos(i / 200 % 360)), // Camera is at (4,3,3), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    return MVP;
};

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "HW 2", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint first_triangle_shaders =
        LoadShaders( "SimpleTransform.vertexshader", "FirstTriangle.shader" );

    GLuint second_triangle_shaders =
        LoadShaders( "SimpleTransform.vertexshader", "SecondTriangle.shader" );

    // Get a handle for our "MVP" uniform
    GLuint FirstMatrix = glGetUniformLocation(first_triangle_shaders, "MVP");
    GLuint SecondMatrix = glGetUniformLocation(second_triangle_shaders, "MVP");


    static const GLfloat g_vertex_buffer_data[] = {
        -0.7f, -0.7f, 0.0f,
        0.7f, -0.7f, 0.0f,
        0.0f,  0.7f, 0.0f,
        0.7f, 0.7f, 0.0f,
        -0.7f, 0.7f, 0.0f,
        0.f,  -0.7f, 0.0f,
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    int i = 0;
    do{

        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT );

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto MVP = build_mvp_matrix(i);

        glUseProgram(first_triangle_shaders);
        glUniformMatrix4fv(FirstMatrix, 1, GL_FALSE, &MVP[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(second_triangle_shaders);
        glUniformMatrix4fv(SecondMatrix, 1, GL_FALSE, &MVP[0][0]);
        glDrawArrays(GL_TRIANGLES, 3, 3);


        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        ++i;

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(first_triangle_shaders);
    glDeleteProgram(second_triangle_shaders);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;}

