#version 330 core
#define IDENTITY mat4(1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 fragCoord;

void main()
{
   gl_Position = vec4(position, 1.0);
   fragCoord = uv;
}
