#version 330 core
#define IDENTITY mat4(1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 UV;

uniform mat4 cameraView = IDENTITY;
uniform mat4 modelWorld = IDENTITY;

void main()
{
   gl_Position = cameraView * modelWorld * vec4(position, 1.0);
   UV = uv;
}
