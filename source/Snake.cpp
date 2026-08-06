#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

using namespace std;

#include "Texture.h"
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// --- Grid Settings ---
const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;

// Quad Vertices for Full-Screen Background
// Coordinates: Position (X, Y) | UV Texture Coordinates (S, T)
GLfloat bgVertices[] = {
    //  Positions   |  TexCoords
    -1.0f, -1.0f,     0.0f, 0.0f, // Bottom-Left
     1.0f, -1.0f,     1.0f, 0.0f, // Bottom-Right
     1.0f,  1.0f,     1.0f, 1.0f, // Top-Right
    -1.0f,  1.0f,     0.0f, 1.0f  // Top-Left
};

GLuint bgIndices[] = {
    0, 1, 2, // First Triangle
    2, 3, 0  // Second Triangle
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Snake", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, 800, 800);

    // Load Shaders
    Shader shaderProgram("default.vert", "default.frag");

    // Setup Background VAO, VBO, EBO
    VAO bgVAO;
    bgVAO.Bind();

    VBO bgVBO(bgVertices, sizeof(bgVertices));
    EBO bgEBO(bgIndices, sizeof(bgIndices));

    // Link attributes: Position = 2 floats, TexCoord = 2 floats
    bgVAO.LinkAttrib(bgVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    bgVAO.LinkAttrib(bgVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    bgVAO.Unbind();
    bgVBO.Unbind();
    bgEBO.Unbind();

    // Load Checkerboard Texture (Ensure image file is in your project directory)
    Texture bgTexture("checkerboard.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    bgTexture.texUnit(shaderProgram, "tex0", 0);

    // Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();
        bgTexture.Bind();
        bgVAO.Bind();

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    bgVAO.Delete();
    bgVBO.Delete();
    bgEBO.Delete();
    bgTexture.Delete();
    shaderProgram.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}