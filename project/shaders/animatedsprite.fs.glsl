#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform int state;
uniform bool visible;

uniform sampler2D image;
uniform float frame;

uniform int SPRITE_COLS;
uniform int SPRITE_ROWS;
uniform int NUM_SPRITES;


void main()
{    
    int spriteIdx = int(frame) % NUM_SPRITES; // Index of sprite to be rendered
    int col = spriteIdx % SPRITE_COLS; // Column of the sprite
    int row = spriteIdx / SPRITE_COLS; // Row of the sprite

    // Calculate the size of each sprite in texture coordinates
    vec2 spriteSize = vec2(1.0 / float(SPRITE_COLS), 1.0 / float(SPRITE_ROWS));

    // Calculate the texture coordinates for the selected sprite
    // (First resize to one sprite, then add offset)
    vec2 spriteTexCoords = vec2(
        TexCoords.x * spriteSize.x + col * spriteSize.x,
        TexCoords.y * spriteSize.y + row * spriteSize.y
    );

    vec4 computedColor = vec4(1.0f);

    if (state == 1) {
        computedColor = vec4(1.0f, 0.0f, 0.0f, 1.0f) * texture(image, spriteTexCoords);
    } else if (state == 2) {
        computedColor = vec4(1.0f, 1.0f, 1.0f, 0.75f) *  texture(image, spriteTexCoords);
    } 
    
    if (!visible) {
        computedColor.a = 0.0f;
    }

    color = computedColor * texture(image, spriteTexCoords);
}  