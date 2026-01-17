#version 440 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture0;

void main()
{
   float intensity = texture(texture0, TexCoord).r;
   FragColor = vec4(vec3(intensity), 1.0); // Render as grayscale
}
