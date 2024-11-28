#version 330 core
in vec3 ourColor;

out vec4 FragColor;

uniform sampler2D ourTexture;
uniform float opacity;

void main()
{
	FragColor = 1 * vec4(ourColor, opacity);
}