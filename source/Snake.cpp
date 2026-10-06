#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

#include "Texture.h"
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// grid size
const int GRID_WIDTH= 20;
const int GRID_HEIGHT= 20;

// snake segment coords
struct SnakeSegment {
    int x, y;
};

// current score
int score= 0;

// start with 3 body segments
vector<SnakeSegment> snake= {
    {10, 10}, // head
    {9, 10},  // body 1
    {8, 10}   // body 2
};

// fruit coords
struct Fruit {
    int x, y;
};

Fruit fruits[2]; // two fruits on board

// spawn a fruit at random free tile
void SpawnFruit(int index) {
    bool validPosition= false;

    while (!validPosition) {
        fruits[index].x= rand() % GRID_WIDTH;
        fruits[index].y= rand() % GRID_HEIGHT;
        validPosition= true;

        // don't spawn inside snake
        for (int i= 0; i < (int)snake.size(); i++) {
            if (snake[i].x == fruits[index].x && snake[i].y == fruits[index].y) {
                validPosition= false;
                break;
            }
        }

        // don't spawn on top of other fruit
        int otherIndex= 0;
        if (index == 0) {
            otherIndex= 1;
        }

        if (fruits[index].x == fruits[otherIndex].x && fruits[index].y == fruits[otherIndex].y) {
            validPosition= false;
        }
    }
}

// spawn both fruits at start
void InitFruits() {
    fruits[0]= {-1, -1}; // temporary off-grid pos
    fruits[1]= {-1, -1};
    SpawnFruit(0);
    SpawnFruit(1);
}

// tile dimensions in ndc (-1 to 1)
const float TILE_WIDTH= 2.0f / GRID_WIDTH;
const float TILE_HEIGHT= 2.0f / GRID_HEIGHT;

// movement state
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction currentDir= RIGHT;      // wanted direction
Direction lastMovedDir= RIGHT;    // last moved direction

float lastTime= 0.0f;
float moveInterval= 0.15f;        // seconds per tile step
bool gameOver= false;

// unit tile quad
GLfloat snakeVertices[]= 
{
    0.0f,        0.0f,        // bottom-left
    TILE_WIDTH,  0.0f,        // bottom-right
    TILE_WIDTH,  TILE_HEIGHT, // top-right
    0.0f,        TILE_HEIGHT  // top-left
};

GLuint snakeIndices[]= {
    0, 1, 2,
    2, 3, 0
};

// centered fruit quad
const float PADDING= 0.25f; // 25% padding

GLfloat fruitVertices[]= {
    TILE_WIDTH * PADDING,          TILE_HEIGHT * PADDING,          // bottom-left
    TILE_WIDTH * (1.0f - PADDING), TILE_HEIGHT * PADDING,          // bottom-right
    TILE_WIDTH * (1.0f - PADDING), TILE_HEIGHT * (1.0f - PADDING), // top-right
    TILE_WIDTH * PADDING,          TILE_HEIGHT * (1.0f - PADDING)  // top-left
};

// full screen quad for background
GLfloat bgVertices[]= {
    // positions      | tex coords
    -1.0f, -1.0f,     0.0f, 0.0f, // bottom-left
     1.0f, -1.0f,     2.5f, 0.0f, // bottom-right
     1.0f,  1.0f,     2.5f, 2.5f, // top-right
    -1.0f,  1.0f,     0.0f, 2.5f  // top-left
};

GLuint bgIndices[]= {
    0, 1, 2, // first tri
    2, 3, 0  // second tri
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window= glfwCreateWindow(800, 800, "Snake", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, 800, 800);

    // load shaders
    Shader shaderProgram("assets/shaders/default.vert", "assets/shaders/default.frag");
    Shader snakeShader("assets/shaders/snake.vert", "assets/shaders/snake.frag");

    // background setup
    VAO bgVAO;
    bgVAO.Bind();
    VBO bgVBO(bgVertices, sizeof(bgVertices));
    EBO bgEBO(bgIndices, sizeof(bgIndices));
    bgVAO.LinkAttrib(bgVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    bgVAO.LinkAttrib(bgVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    bgVAO.Unbind();
    bgVBO.Unbind();
    bgEBO.Unbind();

    Texture bgTexture("assets/textures/checkerboard.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    bgTexture.texUnit(shaderProgram, "tex0", 0);

    // snake quad setup
    VAO snakeVAO;
    snakeVAO.Bind();
    VBO snakeVBO(snakeVertices, sizeof(snakeVertices));
    EBO snakeEBO(snakeIndices, sizeof(snakeIndices));
    snakeVAO.LinkAttrib(snakeVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
    snakeVAO.Unbind();
    snakeVBO.Unbind();
    snakeEBO.Unbind();

    // fruit quad setup
    VAO fruitVAO;
    fruitVAO.Bind();
    VBO fruitVBO(fruitVertices, sizeof(fruitVertices));
    EBO fruitEBO(snakeIndices, sizeof(snakeIndices));
    fruitVAO.LinkAttrib(fruitVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
    fruitVAO.Unbind();
    fruitVBO.Unbind();
    fruitEBO.Unbind();

    // seed random generator
    srand((unsigned int)time(NULL));
    InitFruits();

    // main loop
    while (!glfwWindowShouldClose(window)) {
        // handle input
        if (!gameOver) {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && lastMovedDir != DOWN) {
                currentDir= UP;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && lastMovedDir != UP) {
                currentDir= DOWN;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && lastMovedDir != RIGHT) {
                currentDir= LEFT;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && lastMovedDir != LEFT) {
                currentDir= RIGHT;
            }
        }
        else {
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                snake= {
                    {10, 10}, {9, 10}, {8, 10}
                };
                currentDir= RIGHT;
                lastMovedDir= RIGHT;
                InitFruits();
                score= 0;

                // reset title bar
                glfwSetWindowTitle(window, "Snake - Score: 0");

                gameOver= false;
                lastTime= (float)glfwGetTime();
            }
        }

        // game tick movement
        float currentTime= (float)glfwGetTime();
        if (currentTime - lastTime >= moveInterval && !gameOver) {
            lastTime= currentTime;

            for (int i= (int)snake.size() - 1; i > 0; i--) {
                snake[i]= snake[i - 1];
            }

            if (currentDir == UP) {
                snake[0].y+= 1;
            }
            if (currentDir == DOWN) {
                snake[0].y-= 1;
            }
            if (currentDir == LEFT) {
                snake[0].x-= 1;
            }
            if (currentDir == RIGHT) {
                snake[0].x+= 1;
            }

            // check collisions
            bool justDied= false;

            if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
                justDied= true;
            }
            for (int i= 1; i < (int)snake.size(); i++) {
                if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                    justDied= true;
                }
            }

            // game over
            if (justDied) {
                gameOver= true;
            }

            // eating fruit
            if (!gameOver) {
                for (int f= 0; f < 2; f++) {
                    if (snake[0].x == fruits[f].x && snake[0].y == fruits[f].y) {
                        snake.push_back(snake.back());
                        SpawnFruit(f); // respawn only the eaten fruit
                        score+= 20;

                        string title= "Snake - Score: " + to_string(score);
                        glfwSetWindowTitle(window, title.c_str());

                        break; // only eat one per tick
                    }
                }
                lastMovedDir= currentDir;
            }
        }

        // clear screen
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw background
        shaderProgram.Activate();
        bgTexture.Bind();
        bgVAO.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // draw fruit and snake
        snakeShader.Activate();
        GLuint offsetLoc= glGetUniformLocation(snakeShader.ID, "offset");
        GLuint colorLoc= glGetUniformLocation(snakeShader.ID, "color");

        // draw fruits
        fruitVAO.Bind();
        if (gameOver) {
            glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // turn red on death
        }
        else {
            glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
        }

        // render both fruits
        for (int f= 0; f < 2; f++) {
            float fruitNdcX= -1.0f + (fruits[f].x * TILE_WIDTH);
            float fruitNdcY= -1.0f + (fruits[f].y * TILE_HEIGHT);
            glUniform2f(offsetLoc, fruitNdcX, fruitNdcY);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // draw snake
        snakeVAO.Bind();
        glUniform3f(colorLoc, 0.0f, 0.0f, 139.0f / 255.0f);

        for (int i= 0; i < (int)snake.size(); i++) {
            float ndcX= -1.0f + (snake[i].x * TILE_WIDTH);
            float ndcY= -1.0f + (snake[i].y * TILE_HEIGHT);

            glUniform2f(offsetLoc, ndcX, ndcY);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    bgVAO.Delete();
    bgVBO.Delete();
    bgEBO.Delete();
    bgTexture.Delete();
    shaderProgram.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
