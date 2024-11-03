#include <vector>
#include <random>

#include "entities/ecs_registry.hpp"


#include "utils/isometric_helper.hpp"
#include "entities/general_components.hpp"

class TileGenerator {
private:
    int numRows, numCols;
    std::vector<std::vector<float>> noiseMap;
    std::random_device rd;
    std::mt19937 gen;
    
    void generateNoiseMap() {
        // Initialize noise map with random values
        noiseMap.resize(numRows, std::vector<float>(numCols));
        
        // Generate initial random noise
        std::uniform_real_distribution<> dis(0.0, 1.0);
        for (int row = 0; row < numRows; row++) {
            for (int col = 0; col < numCols; col++) {
                noiseMap[row][col] = dis(gen);
            }
        }
        
        // Smooth the noise to create clusters
        smoothNoiseMap();
    }
    
    void smoothNoiseMap() {
        auto tempMap = noiseMap;
        const int smoothPasses = 2;
        
        for (int pass = 0; pass < smoothPasses; pass++) {
            for (int row = 0; row < numRows; row++) {
                for (int col = 0; col < numCols; col++) {
                    float sum = 0.0f;
                    int count = 0;
                    
                    // Average with neighboring cells
                    for (int dr = -1; dr <= 1; dr++) {
                        for (int dc = -1; dc <= 1; dc++) {
                            int newRow = row + dr;
                            int newCol = col + dc;
                            
                            if (newRow >= 0 && newRow < numRows && 
                                newCol >= 0 && newCol < numCols) {
                                sum += noiseMap[newRow][newCol];
                                count++;
                            }
                        }
                    }
                    tempMap[row][col] = sum / count;
                }
            }
            noiseMap = tempMap;
        }
    }

public:
    TileGenerator(int rows, int cols, bool safetyMultiplier = true) :
        numRows(rows * (safetyMultiplier ? 1 : 1)),
        numCols(cols * (safetyMultiplier ? 1 : 1)),
        gen(rd()) {
        generateNoiseMap();
    }

    
    void createTile(TileType type, vec2 position, vec2 scale) {
        Entity tile;
        Tile& tile_component = registry.tiles.emplace(tile);
        tile_component.type = type;
        tile_component.position = position;
        tile_component.scale = scale;

        RenderRequest& request = registry.static_render_requests.emplace(tile);
        request.mesh = "sprite";
        switch (type) {
            case TileType::GRASS2:
                request.texture = "grass2";
                break;
            case TileType::GRASS3:
                request.texture = "grass3";
                break;
            case TileType::GRASS4:
                request.texture = "grass4";
                break;
            case TileType::GRASS5:
                request.texture = "grass5";
                break;
            case TileType::CLAY1:
                request.texture = "clay1";
                break;
            case TileType::CLAY2:
                request.texture = "clay2";
                break;
            case TileType::CLAY3:
                request.texture = "clay3";
                break;
            default:
                request.texture = "grass1";
                break;
        }
        request.shader = "sprite";
        request.type = BACK;

    }
    
    void generateTiles() {
        vec2 offset = {-window_width_px  , -window_height_px / 2};
        for (int row = 0; row < numRows; row++) {
            for (int col = 0; col < numCols; col++) {
                vec2 pos = IsometricGrid::getIsometricPosition(col, row, true) + offset;
                float noiseValue = noiseMap[row][col];
                TileType type;
                
                // Use noise value to determine tile type
                if (noiseValue < 0.6) {
                    // Grass area (60% chance)
                    int randNum = rand() % 100;
                    if (randNum < 40) type = TileType::GRASS1;
                    else if (randNum < 70) type = TileType::GRASS1;
                    else if (randNum < 77) type = TileType::GRASS2;
                    else if (randNum < 84) type = TileType::GRASS3;
                    else if (randNum < 92) type = TileType::GRASS4;
                    else type = TileType::GRASS5;
                }
                else {
                    // Clay area (40% chance)
                    int randNum = rand() % 100;
                    if (randNum < 50) type = TileType::CLAY1;
                    else if (randNum < 75) type = TileType::CLAY2;
                    else if (randNum < 90) type = TileType::CLAY3;
                    else type = TileType::CLAY3;
                }
                
                createTile(type, pos, {0.5f, 0.5f});
            }
        }
    }
};