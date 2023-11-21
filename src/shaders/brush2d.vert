#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out VS_OUT
{
    vec2 brushUV;
    vec2 imageUV;
}
vs;

uniform mat4 cameraView;
uniform mat4 modelWorld;
uniform mat4 imageViewMatrix;

void main()
{
    vec4 pos = vec4(position, 1);
    gl_Position = cameraView * modelWorld * pos;
    vs.brushUV = uv;
    vs.imageUV = (imageViewMatrix * modelWorld * pos).xy;
    vs.imageUV = vs.imageUV * 0.5 + 0.5;
}
