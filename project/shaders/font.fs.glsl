#version 330 core
/* simpleGL freetype font fragment shader */
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform bool flip;

void main()
{
	
	vec2 coords = TexCoords;

	if (flip) {
		coords.y = 1.0 - coords.y;
	}
	

	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, coords).r);
	color = vec4(textColor, 1.0) * sampled;
}
