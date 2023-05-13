#version 330 core

in vec2 fragCoord;
out vec4 FragColor;
uniform sampler2D texture0;


void main()
{
    FragColor = texture2D(texture0, fragCoord);
   // FragColor = vec4(fragCoord, 0, 1);
}
