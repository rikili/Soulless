#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform float frame;

uniform int SPRITE_COLS;
uniform int SPRITE_ROWS;
uniform int NUM_SPRITES;

void main()
{    
    int spriteIdx = int(frame) % NUM_SPRITES;
    vec2 pos = vec2(spriteIdx % SPRITE_COLS, int(spriteIdx / SPRITE_COLS));

    vec2 spriteTexCoords = vec2(
        (TexCoords.x / SPRITE_COLS + frame / float(SPRITE_COLS)),
        (TexCoords.y / SPRITE_ROWS + frame / float(SPRITE_ROWS))
    );

    color = texture(image, spriteTexCoords);
}  