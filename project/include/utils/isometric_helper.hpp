#pragma once
#include "core/common.hpp"

class IsometricGrid {
public:
    static constexpr bool IS_ISOMETRIC = false;
    // Base tile dimensions
    static constexpr float TILE_WIDTH = 32.0f;
    static constexpr float TILE_HEIGHT = 32.0f;
    
    // Spacing multipliers for isometric view
    static constexpr float ISO_SPACING_X = 1.0f;  // Increased horizontal spacing
    static constexpr float ISO_SPACING_Y = 0.9f;  // Increased vertical spacing
    
    // Calculated isometric dimensions
    static constexpr float ISO_TILE_WIDTH = TILE_WIDTH * ISO_SPACING_X;
    static constexpr float ISO_TILE_HEIGHT = TILE_HEIGHT * ISO_SPACING_Y;

    static vec2 getIsometricPosition(int col, int row, bool isIsometricOverride = false) {
        if (isIsometricOverride || IS_ISOMETRIC) {
            float x = (col * ISO_TILE_WIDTH)  + (row * ISO_TILE_HEIGHT /2);
            float y = (row * ISO_TILE_HEIGHT) / 2;
            return {x, y};
        } else {
            float x = col * TILE_WIDTH;
            float y = row * TILE_HEIGHT;
            return {x, y};
        }
    }

    static vec2 screenToGrid(vec2 screenPos) {
        
        float col = (screenPos.x / (TILE_WIDTH * ISO_SPACING_X * 0.5f) + 
                    screenPos.y / (TILE_HEIGHT * ISO_SPACING_Y * 0.25f)) / 2;
        float row = (screenPos.y / (TILE_HEIGHT * ISO_SPACING_Y * 0.25f) - 
                    screenPos.x / (TILE_WIDTH * ISO_SPACING_X * 0.5f)) / 2;
        return {col , row};
    }

    static vec2 getGridDimensions(float screenWidth, float screenHeight) {
        if (IS_ISOMETRIC) { 
            int cols = static_cast<int>(screenWidth / (ISO_TILE_WIDTH ));
            int rows = static_cast<int>(screenHeight / (ISO_TILE_HEIGHT * 0.5f)) + 2;
            return {static_cast<float>(cols), static_cast<float>(rows)};
        } else {
            int cols = static_cast<int>(screenWidth / TILE_WIDTH);
            int rows = static_cast<int>(screenHeight / TILE_HEIGHT);
            return {static_cast<float>(cols), static_cast<float>(rows)};
        }
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
        if (IS_ISOMETRIC) {
            // Standard isometric projection with 2:1 ratio
            float isoX = (cartesianPos.x - cartesianPos.y) * (1);
            float isoY = (cartesianPos.x + cartesianPos.y) * (1);
            return {isoX, isoY};
        } else {
            return cartesianPos;
        }
    }
};