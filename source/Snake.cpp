#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <vector>

using namespace std;

#include "Texture.h"
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// --- Grid Settings ---
const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;


// --- Snake Data Structure ---
struct SnakeSegment {
    int x, y;
};

// Start the snake with 3 segments in the middle of the grid
vector<SnakeSegment> snake = {
    {10, 10}, // Head
    {9, 10},  // Body 1
    {8, 10}   // Body 2
};

// Calculate exactly how big one tile is in Normalized Device Coordinates (-1.0 to 1.0)
const float TILE_WIDTH = 2.0f / GRID_WIDTH;
const float TILE_HEIGHT = 2.0f / GRID_HEIGHT;

// A single square exactly the size of one grid tile, starting at (0,0)
GLfloat snakeVertices[] = {
    0.0f,        0.0f,        // Bottom-Left
    TILE_WIDTH,  0.0f,        // Bottom-Right
    TILE_WIDTH,  TILE_HEIGHT, // Top-Right
    0.0f,        TILE_HEIGHT  // Top-Left
};

GLuint snakeIndices[] = {
    0, 1, 2,
    2, 3, 0
};

// Quad Vertices for Full-Screen Background
GLfloat bgVertices[] = {
    //  Positions   |  TexCoords
    -1.0f, -1.0f,     0.0f, 0.0f, // Bottom-Left
     1.0f, -1.0f,     2.5f, 0.0f, // Bottom-Right
     1.0f,  1.0f,     2.5f, 2.5f, // Top-Right
    -1.0f,  1.0f,     0.0f, 2.5f  // Top-Left
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

    // Load Snake Shader
    Shader snakeShader("snake.vert", "snake.frag");

    // Setup Snake VAO, VBO, EBO
    VAO snakeVAO;
    snakeVAO.Bind();
    VBO snakeVBO(snakeVertices, sizeof(snakeVertices));
    EBO snakeEBO(snakeIndices, sizeof(snakeIndices));
    // Link attributes: Position = 2 floats
    snakeVAO.LinkAttrib(snakeVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
    snakeVAO.Unbind();
    snakeVBO.Unbind();
    snakeEBO.Unbind();

    // Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // --- DRAW BACKGROUND ---
        shaderProgram.Activate();
        bgTexture.Bind();
        bgVAO.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // --- DRAW SNAKE ---
        snakeShader.Activate();
        snakeVAO.Bind();

        // Find the locations of our uniform variables in the shader
        GLuint offsetLoc = glGetUniformLocation(snakeShader.ID, "offset");
        GLuint colorLoc = glGetUniformLocation(snakeShader.ID, "color");

        // Set the snake color to Green (R, G, B)
        glUniform3f(colorLoc, 0.2f, 0.8f, 0.2f);

        // Loop through every piece of the snake's body
        for (int i = 0; i < snake.size(); i++) {
            // Convert the grid coordinate (e.g., 10, 10) to OpenGL screen coordinates
            float ndcX = -1.0f + (snake[i].x * TILE_WIDTH);
            float ndcY = -1.0f + (snake[i].y * TILE_HEIGHT);

            // Push the offset to the vertex shader
            glUniform2f(offsetLoc, ndcX, ndcY);

            // Draw this specific segment
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

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