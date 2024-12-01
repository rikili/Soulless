#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 vertexPos;

uniform mat4 transform;

void main()
{
    vertexPos = aPos;
    gl_Position = transform * vec4(aPos, 1.0);
}