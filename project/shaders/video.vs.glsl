#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;

void main() {
    // Convert from [-1,1] to [0,1] space
    vec2 pos = (aPos + 1.0) * 0.5;
    gl_Position = projection * view * vec4(pos, 0.0, 1.0);
    TexCoord = aTexCoord;
}