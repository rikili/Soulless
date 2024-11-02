#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 ourColor;

uniform vec3 color_override;
uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * transform * vec4(aPos, 1.0);
    ourColor = color_override;
}