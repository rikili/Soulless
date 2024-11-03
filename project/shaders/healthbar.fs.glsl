#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform float proportionFilled;
uniform sampler2D image;

void main()
{    
    float offset = TexCoords.x > proportionFilled ? 0.0 : 0.5;
    vec2 newTexCoords = vec2(TexCoords.x / 2 + offset, TexCoords.y);
    color = texture(image, newTexCoords);
}  