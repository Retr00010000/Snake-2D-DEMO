# Snake 2D (OpenGL)

A classic 2D Snake game built with C++, OpenGL 4.6, and GLFW.

## Features
- Smooth 2D grid rendering using modern OpenGL shaders
- Checkerboard grid background
- Score tracking and dynamic window title updates
- Self-contained, lightweight build using MinGW-w64 (GCC)

## Project Structure
- `source/Snake.cpp` - Game logic, loop, and input handling
- `source/engine/` - OpenGL boilerplate (VAO, VBO, EBO, Texture, Shaders)
- `assets/` - Shaders (`.vert`, `.frag`) and textures (`.png`)
- `Dependencies/` - GLFW headers and MinGW library, GLAD, and stb_image
- `build.ps1` - PowerShell script to build and launch the game

## How to Build and Run

### In VS Code
1. Open the project folder in VS Code.
2. Press `F5` to build and launch.

### In Terminal (PowerShell)
```powershell
.\build.ps1
```
*(Requires MinGW-w64 `g++`)*
