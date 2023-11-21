#version 330 core

out vec4 FragColor;

// uniform vec4 color;

uniform sampler2D backgroundTexture;

in VS_OUT
{
    vec2 brushUV;
    vec2 imageUV;
}
vs;

void main()
{
    vec4 imageColor = texture2D(backgroundTexture, vs.imageUV);
    FragColor = imageColor;
}
