#version 460 core

// Positions/Coordinates
layout (location = 0) in vec2 aPos;
// Texture Coordinates
layout (location = 1) in vec2 aTex;

// Outputs the texture coordinates to the fragment shader
out vec2 texCoord;

void main()
{
    // We set Z to 0.0 and W to 1.0 since this is a 2D game
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    // Assigns the texture coordinates from the Vertex Data to "texCoord"
    texCoord = aTex;
}