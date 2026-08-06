#version 460 core

layout (location = 0) in vec2 aPos;

// This allows us to push the square to different grid coordinates from C++
uniform vec2 offset; 

void main()
{
    gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, 0.0, 1.0);
}