#include <vector>
#include <random>

#include "entities/ecs_registry.hpp"

#include "utils/isometric_helper.hpp"
#include "entities/general_components.hpp"
#include "graphics/batch_renderer.hpp"

class TileGenerator
{
private:
    int numRows, numCols;
    int w, h;
    std::vector<std::vector<float>> noiseMap;
    std::random_device rd;
    std::mt19937 gen;

    // learned from https://www.youtube.com/watch?v=slTEz6555Ts
    void generateNoiseMap()
    {
        noiseMap.resize(numRows, std::vector<float>(numCols));

        std::uniform_real_distribution<> dis(0.0, 1.0);
        for (int row = 0; row < numRows; row++)
        {
            for (int col = 0; col < numCols; col++)
            {
                noiseMap[row][col] = dis(gen);
            }
        }

        smoothNoiseMap();
    }

    void smoothNoiseMap()
    {
        auto tempMap = noiseMap;
        const int smoothPasses = 2;

        for (int pass = 0; pass < smoothPasses; pass++)
        {
            for (int row = 0; row < numRows; row++)
            {
                for (int col = 0; col < numCols; col++)
                {
                    float sum = 0.0f;
                    int count = 0;

                    for (int dr = -1; dr <= 1; dr++)
                    {
                        for (int dc = -1; dc <= 1; dc++)
                        {
                            int newRow = row + dr;
                            int newCol = col + dc;

                            if (newRow >= 0 && newRow < numRows &&
                                newCol >= 0 && newCol < numCols)
                            {
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
    TileGenerator(int rows, int cols, int w, int h,
                  bool safetyMultiplier = true) : numRows(rows * (safetyMultiplier ? 2 : 1)),
                                                  numCols(cols * (safetyMultiplier ? 3 : 1)),
                                                  w(w), h(h), gen(rd()) {
        generateNoiseMap();
    }

    ~TileGenerator() {
    }

    void generateTiles(BatchRenderer *batchRenderer) {
        vec2 offset = {-w , -h / 2};
        for (int row = 0; row < numRows; row++)
        {
            for (int col = 0; col < numCols; col++)
            {
                vec2 pos = IsometricGrid::getIsometricPosition(col, row, true) + offset;
                float noiseValue = noiseMap[row][col];

                std::string textureId;

                // Determine texture based on noise value
                // TODO: Refactor this to use a more flexible system
                // like what if we want to add more textures?
                if (noiseValue < 0.6)
                {
                    // Grass area (60% chance)
                    int randNum = rand() % 100;
                    if (randNum < 40)
                        textureId = "grass1";
                    else if (randNum < 70)
                        textureId = "grass2";
                    else if (randNum < 77)
                        textureId = "grass3";
                    else if (randNum < 84)
                        textureId = "grass4";
                    else if (randNum < 92)
                        textureId = "grass2";
                    else
                        textureId = "grass1";
                }
                else
                {
                    // Clay area (40% chance)
                    int randNum = rand() % 100;
                    if (randNum < 50)
                        textureId = "clay1";
                    else if (randNum < 75)
                        textureId = "clay2";
                    else
                        textureId = "clay1";
                }

                batchRenderer->addTile(pos, {0.5f, 0.5f}, textureId);
            }
        }
    }
};