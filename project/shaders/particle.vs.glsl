#version 330 core
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec2 aParticlePos;
layout (location = 2) in float aParticleSize;
layout (location = 3) in vec3 aParticleColor;

out vec3 fColor;

uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec2 pos = aPos.xy * aParticleSize + aParticlePos;
    fColor = aParticleColor;

    gl_Position = projection * view * vec4(pos, aPos.z, 1.0);
}