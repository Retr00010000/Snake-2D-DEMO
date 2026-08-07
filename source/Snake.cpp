#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>

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

// --- Fruit Variables ---
int fruitX;
int fruitY;

void SpawnFruit() {
    fruitX = rand() % GRID_WIDTH;
    fruitY = rand() % GRID_HEIGHT;
}

// Start the snake with 3 segments in the middle of the grid
vector<SnakeSegment> snake = {
    {10, 10}, // Head
    {9, 10},  // Body 1
    {8, 10}   // Body 2
};

// Calculate exactly how big one tile is in Normalized Device Coordinates (-1.0 to 1.0)
const float TILE_WIDTH = 2.0f / GRID_WIDTH;
const float TILE_HEIGHT = 2.0f / GRID_HEIGHT;

// --- Snake Movement Variables ---
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction currentDir = RIGHT;      // The direction the player WANTS to go
Direction lastMovedDir = RIGHT;    // The direction the snake ACTUALLY moved last tick

float lastTime = 0.0f;
float moveInterval = 0.15f;        // How fast the snake moves (0.15 seconds per tile)
bool gameOver = false;

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

// A smaller square for the fruit, centered inside a grid tile
const float PADDING = 0.25f; // Leaves 25% empty space on all sides
GLfloat fruitVertices[] = {
    TILE_WIDTH * PADDING,          TILE_HEIGHT * PADDING,          // Bottom-Left
    TILE_WIDTH * (1.0f - PADDING), TILE_HEIGHT * PADDING,          // Bottom-Right
    TILE_WIDTH * (1.0f - PADDING), TILE_HEIGHT * (1.0f - PADDING), // Top-Right
    TILE_WIDTH * PADDING,          TILE_HEIGHT * (1.0f - PADDING)  // Top-Left
};

// You can reuse the snakeIndices array for the fruit EBO since it's also just a square!

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

    // --- LOAD SHADERS ---
    Shader shaderProgram("default.vert", "default.frag");
    Shader snakeShader("snake.vert", "snake.frag");

    // --- SETUP BACKGROUND ---
    VAO bgVAO;
    bgVAO.Bind();
    VBO bgVBO(bgVertices, sizeof(bgVertices));
    EBO bgEBO(bgIndices, sizeof(bgIndices));
    bgVAO.LinkAttrib(bgVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    bgVAO.LinkAttrib(bgVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    bgVAO.Unbind();
    bgVBO.Unbind();
    bgEBO.Unbind();

    Texture bgTexture("checkerboard.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    bgTexture.texUnit(shaderProgram, "tex0", 0);

    // --- SETUP SNAKE ---
    VAO snakeVAO;
    snakeVAO.Bind();
    VBO snakeVBO(snakeVertices, sizeof(snakeVertices));
    EBO snakeEBO(snakeIndices, sizeof(snakeIndices));
    snakeVAO.LinkAttrib(snakeVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
    snakeVAO.Unbind();
    snakeVBO.Unbind();
    snakeEBO.Unbind();

    // --- SETUP FRUIT ---
    VAO fruitVAO;
    fruitVAO.Bind();
    VBO fruitVBO(fruitVertices, sizeof(fruitVertices));
    EBO fruitEBO(snakeIndices, sizeof(snakeIndices));
    fruitVAO.LinkAttrib(fruitVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
    fruitVAO.Unbind();
    fruitVBO.Unbind();
    fruitEBO.Unbind();

    // --- SEED RANDOMNESS ---
    srand(time(NULL));
    SpawnFruit();

    // Main Game Loop
    // Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        // --- 1. INPUT HANDLING ---
        // Only accept input if the game is still running
        if (!gameOver) {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && lastMovedDir != DOWN) currentDir = UP;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && lastMovedDir != UP) currentDir = DOWN;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && lastMovedDir != RIGHT) currentDir = LEFT;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && lastMovedDir != LEFT) currentDir = RIGHT;
        }

        // --- 2. GAME TICK & MOVEMENT ---
        float currentTime = glfwGetTime();
        if (currentTime - lastTime >= moveInterval && !gameOver) {
            lastTime = currentTime;

            // Shift the body segments
            for (int i = snake.size() - 1; i > 0; i--) {
                snake[i] = snake[i - 1];
            }

            // Move the head
            if (currentDir == UP) snake[0].y += 1;
            if (currentDir == DOWN) snake[0].y -= 1;
            if (currentDir == LEFT) snake[0].x -= 1;
            if (currentDir == RIGHT) snake[0].x += 1;

            // --- COLLISION CHECKS ---
            // A. Check Wall Collision (Has the head left the 0-19 grid?)
            if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
                gameOver = true;
            }

            // B. Check Body Collision (Did the head overlap with any body segment?)
            for (int i = 1; i < snake.size(); i++) {
                if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                    gameOver = true;
                }
            }

            // --- EATING LOGIC ---
            // Only eat if we haven't crashed this tick
            if (!gameOver) {
                if (snake[0].x == fruitX && snake[0].y == fruitY) {
                    snake.push_back(snake.back());
                    SpawnFruit();
                }
                lastMovedDir = currentDir;
            }
        }

        // --- 3. CLEAR SCREEN ---
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // --- 4. DRAW BACKGROUND ---
        shaderProgram.Activate();
        bgTexture.Bind();
        bgVAO.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // --- 5. DRAW FRUIT AND SNAKE ---
        snakeShader.Activate();
        GLuint offsetLoc = glGetUniformLocation(snakeShader.ID, "offset");
        GLuint colorLoc = glGetUniformLocation(snakeShader.ID, "color");

        // Draw Fruit (Yellow)
        // If the game is over, we can turn the fruit Red or just leave it Yellow!
        fruitVAO.Bind();
        if (gameOver) glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // Turns red on death
        else glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);

        float fruitNdcX = -1.0f + (fruitX * TILE_WIDTH);
        float fruitNdcY = -1.0f + (fruitY * TILE_HEIGHT);
        glUniform2f(offsetLoc, fruitNdcX, fruitNdcY);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw Snake
        snakeVAO.Bind();
        glUniform3f(colorLoc, 0.0f, 0.0f, 139.0f / 255.0f);

        for (int i = 0; i < snake.size(); i++) {
            float ndcX = -1.0f + (snake[i].x * TILE_WIDTH);
            float ndcY = -1.0f + (snake[i].y * TILE_HEIGHT);

            glUniform2f(offsetLoc, ndcX, ndcY);
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