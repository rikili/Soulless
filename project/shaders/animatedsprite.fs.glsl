#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform float frame;

const int SPRITE_COLS = 4;
const int SPRITE_ROWS = 1;
const int NUM_SPRITES = 4;

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