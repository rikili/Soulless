#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform bool flip;

void main()
{
	vec2 coords = TexCoords;

	if (flip) {
		coords.y = 1.0 - coords.y;
	}
	color = texture(image, coords);
}