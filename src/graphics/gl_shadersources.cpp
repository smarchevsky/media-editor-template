#include "gl_shadersources.h"

const char* GLShaderSources::getDefault2d_VS()
{
    return R"(\
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
    // FragColor = vec4(UV, 0, 1);

    vec4 t0 = texture2D(texture0, UV);
    // vec4 t1 = texture2D(texture1, UV);

    // FragColor = mix(t0, t1, vec4(0.75));
    FragColor = t0;
}
)";
}

const char* GLShaderSources::getDefault3d_VS()
{
    return R"(\
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

uniform mat4 cameraView = IDENTITY;
uniform mat4 cameraProjection = IDENTITY;
uniform mat4 modelWorld = IDENTITY;

void main()
{
    vs.wPos = modelWorld * vec4(position, 1.0);
    gl_Position = cameraProjection * cameraView * vs.wPos;
    vs.n = mat3(modelWorld) * normal;
    vs.uv = uv;
}
)";
}

const char* GLShaderSources::getDefault3d_FS()
{
    return R"(\
#version 330 core

in VS_OUT {
    vec4 wPos;  // world pos
    vec3 n;
    vec2 uv;
} vs;

uniform sampler2D texture0;
uniform vec3 cameraPosition;

out vec4 FragColor;

#define FLIP_Y(c) vec2(c.x, 1.0 - c.y)

void main()
{
    vec3 n = normalize(vs.n);
    vec3 fragOffset = vs.wPos.xyz - cameraPosition;
    float fragOffsetLength = length(fragOffset);
    vec3 lightDir = fragOffset / fragOffsetLength;
    float fragOffsetLength2 = fragOffsetLength * fragOffsetLength;

    vec2 uv = vs.uv;
    vec4 c1 = vec4(cos(vs.n * 3.14) * 0.5 + 0.5, 1);
    vec4 c2 = texture2D(texture0, FLIP_Y(uv));
    vec4 c3 = vec4(.2, .2, .2, 1);
    FragColor = mix(c1, c3, c2 * c2);
    FragColor += pow(clamp(-dot(lightDir, n), 0., 1.), 80. * fragOffsetLength) * 0.2 / fragOffsetLength;

    // FragColor = vec4(uv, 0, 1);
}
)";
}
