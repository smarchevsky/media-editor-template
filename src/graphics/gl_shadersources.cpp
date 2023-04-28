#include "gl_shadersources.h"

const char* GLShaderSources::getNoTransformFullScreen2d_VS()
{
    return R"(\
#version 330 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;

out vec2 UV;

void main()
{
   gl_Position = vec4(inPos, 0.0, 1.0);
   UV = inUV;
}
)";
}

const char* GLShaderSources::getDefault2d_VS()
{
    return R"(\
#version 330 core
#define IDENTITY mat4(1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1)

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

out vec2 UV;

uniform mat4 matView = IDENTITY;
uniform mat4 matModel = IDENTITY;

void main()
{
   gl_Position = matView * matModel * vec4(inPos, 1.0);
   UV = inUV;
}
)";
}

const char* GLShaderSources::getDefault2d_FS()
{
    return R"(\
#version 330 core

in vec2 UV;
out vec4 FragColor;

uniform sampler2D texture0;
// uniform sampler2D texture1;

void main()
{
    //FragColor = vec4(UV, 0, 1);

    vec4 t0 = texture2D(texture0, UV);
    // vec4 t1 = texture2D(texture1, UV);

    // FragColor = mix(t0, t1, vec4(0.75));
    FragColor = t0;
}
)";
}
