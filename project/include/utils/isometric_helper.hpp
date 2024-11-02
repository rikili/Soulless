#pragma once
#include "core/common.hpp"

class IsometricGrid {
public:
    // Base tile dimensions
    static constexpr float TILE_WIDTH = 32.0f;
    static constexpr float TILE_HEIGHT = 32.0f;
    
    // Spacing multipliers for isometric view
    static constexpr float ISO_SPACING_X = 1.2f;  // Increased horizontal spacing
    static constexpr float ISO_SPACING_Y = 1.1f;  // Increased vertical spacing
    
    // Calculated isometric dimensions
    static constexpr float ISO_TILE_WIDTH = TILE_WIDTH * ISO_SPACING_X;
    static constexpr float ISO_TILE_HEIGHT = TILE_HEIGHT * ISO_SPACING_Y;

    // Core position conversion with adjusted spacing
    static vec2 getIsometricPosition(int col, int row) {
        float x = (col * ISO_TILE_WIDTH)  + (row * ISO_TILE_HEIGHT /2);
        float y = (row * ISO_TILE_HEIGHT) / 2;
        return {x, y};
    }

    // Convert screen position to grid coordinates (adjusted for new spacing)
    static vec2 screenToGrid(vec2 screenPos) {
        float col = (screenPos.x / (TILE_WIDTH * ISO_SPACING_X * 0.5f) + 
                    screenPos.y / (TILE_HEIGHT * ISO_SPACING_Y * 0.25f)) / 2;
        float row = (screenPos.y / (TILE_HEIGHT * ISO_SPACING_Y * 0.25f) - 
                    screenPos.x / (TILE_WIDTH * ISO_SPACING_X * 0.5f)) / 2;
        return {col , row};
    }

    // Get the dimensions of a grid that would cover the screen (adjusted for spacing)
    static vec2 getGridDimensions(float screenWidth, float screenHeight) {
        int cols = static_cast<int>(screenWidth / (ISO_TILE_WIDTH ));
        int rows = static_cast<int>(screenHeight / (ISO_TILE_HEIGHT * 0.5f)) + 2;
        return {static_cast<float>(cols), static_cast<float>(rows)};
    }

    // Additional utility for getting world space dimensions of the grid
    static vec2 getWorldDimensions(int cols, int rows) {
        float width = cols * ISO_TILE_WIDTH;
        float height = rows * ISO_TILE_HEIGHT;
        return {width, height};
    }

    // Check if a grid position is within bounds (unchanged)
    static bool isInBounds(int col, int row, int maxCols, int maxRows) {
        return col >= 0 && col < maxCols && row >= 0 && row < maxRows;
    }

    static vec2 convertToIsometric(vec2 cartesianPos) {
    // Standard isometric projection with 2:1 ratio
        float isoX = (cartesianPos.x - cartesianPos.y) * (1);
        float isoY = (cartesianPos.x + cartesianPos.y) * (1);
        return {isoX, isoY};
    }
};