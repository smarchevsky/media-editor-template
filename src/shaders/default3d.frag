#version 330 core

in VS_OUT
{
    vec4 wPos; // world pos
    vec3 n;
    vec2 uv;
}
vs;

uniform sampler2D texture0;
uniform vec3 cameraPosition;

out vec4 FragColor;

void main()
{
    vec3 n = normalize(vs.n);
    vec3 fragOffset = vs.wPos.xyz - cameraPosition;
    float fragOffsetLength = length(fragOffset);
    vec3 lightDir = fragOffset / fragOffsetLength;
    float fragOffsetLength2 = fragOffsetLength * fragOffsetLength;

    vec4 c1 = vec4(cos(vs.n * 3.14) * 0.5 + 0.5, 1);
    vec4 c2 = texture2D(texture0, vs.uv);
    vec4 c3 = vec4(.2, .2, .2, 1);
    FragColor = mix(c1, c3, c2 * c2);
    FragColor += pow(clamp(-dot(lightDir, n), 0., 1.), 80. * fragOffsetLength) * 0.2 / fragOffsetLength;
}
