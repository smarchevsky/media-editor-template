#version 330 core
#define IDENTITY mat4(1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1)

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out VS_OUT {
    vec4 wPos;  // world pos
    vec3 n;     // normal
    vec2 uv;    // texture coordinate
} vs;

uniform mat4 cameraViewInv = IDENTITY;
uniform mat4 cameraProjection = IDENTITY;
uniform mat4 modelWorld = IDENTITY;

void main()
{
    vs.wPos = modelWorld * vec4(position, 1.0);
    gl_Position = cameraProjection * cameraViewInv * vs.wPos;
    vs.n = mat3(modelWorld) * normal;
    vs.uv = uv;
}
