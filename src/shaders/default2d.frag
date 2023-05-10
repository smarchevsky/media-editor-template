#version 330 core

in vec2 UV;

out vec4 FragColor;

uniform sampler2D texture0;
// uniform sampler2D texture1;
uniform float opacity;

void main()
{
    // FragColor = vec4(UV, 0, 1);

    vec4 t0 = texture2D(texture0, UV);
    // vec4 t1 = texture2D(texture1, UV);

    // FragColor = mix(t0, t1, vec4(0.75));
    FragColor = t0;
    FragColor.a = opacity;
}
