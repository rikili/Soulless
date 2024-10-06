#version 330 core
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

uniform mat4 transform;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoord;
    gl_Position = projection * transform * vec4(aPos, 1.0);
}