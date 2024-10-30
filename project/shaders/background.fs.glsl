#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D backgroundTexture;
uniform vec2 repeatFactor;

void main()
{
    vec2 repeatedTexCoord = TexCoord * repeatFactor;
    FragColor = texture(backgroundTexture, repeatedTexCoord);
}
