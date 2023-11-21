#version 330 core

in vec2 uv;

uniform sampler2D texture0;
uniform float opacity;

out vec4 FragColor;

void main()
{
    vec4 t0 = texture2D(texture0, uv);

    FragColor = t0;
    FragColor.a = opacity;
}
