#pragma once
#include "Biome.h"
#include "Perlin.h"
#include <vector>
#include <fstream>

class BiomeManager
{
private:
    Perlin temperatureNoise;
    Perlin moistureNoise;
    double smoothRemap(double value, double inMin, double inMax)
    {
        double t = (value - inMin) / (inMax - inMin);
        t = std::clamp(t, 0.0, 1.0);
        return t * t * (3 - 2 * t);
    }

public:
    std::vector<Biome> biomes;

    BiomeManager(unsigned int seed)
        : temperatureNoise(seed),
          moistureNoise(seed + 1337)
    {
        biomes.push_back(Biome("Plains", 10.0, 10.0, 4, 0.5, 2.0, 0.8, {0.0f, 1.0f, 0.0f}, 0.5, 0.5));
        biomes.push_back(Biome("Hills", 40.0, 50.0, 6, 0.5, 2.0, 1.0, {1.0f, 0.0f, 0.0f}, 0.3, 0.4));
        biomes.push_back(Biome("Mountains", 50.0, 200.0, 8, 0.45, 2.2, 1.2, {0.0f, 0.0f, 1.0f}, 0.1, 0.2));
        biomes.push_back(Biome("Desert", 20.0, 5.0, 4, 0.5, 2.0, 0.8, {1.0f, 0.9f, 0.2f}, 0.9, 0.1));
        biomes.push_back(Biome("Taiga", 40.0, 50.0, 6, 0.5, 2.0, 1.0, {0.0f, 0.5f, 0.0f}, 0.2, 0.7));
    }

    glm::dvec2 getClimate(double x, double z)
    {
        double temp = (temperatureNoise.noise(x * 0.0005, z * 0.0005, 0.0) + 1.0) * 0.5;
        double moist = (moistureNoise.noise(x * 0.0005, z * 0.0005, 0.0) + 1.0) * 0.5;
        return glm::dvec2(std::clamp(temp, 0.0, 1.0), std::clamp(moist, 0.0, 1.0));
    }

    Biome getBiomeAt(double x, double z)
    {
        glm::dvec2 climate = getClimate(x, z);

        std::vector<std::pair<const Biome *, double>> candidates;
        for (auto &biome : biomes)
        {
            double dx = climate.x - biome.temperature;
            double dy = climate.y - biome.moisture;
            double dist2 = dx * dx + dy * dy;

            double w = 1.0 / (0.0001 + dist2);

            candidates.push_back({&biome, w});
        }

        return Biome::blendMultiple(candidates);
    }

    void exportClimateMap(const std::string &filename, int width, int height)
    {
        std::ofstream file(filename, std::ios::binary);
        file << "P6\n"
             << width << " " << height << "\n255\n";

        for (int y = 0; y < height; y++)
        {
            double moist = (double)y / (height - 1);
            for (int x = 0; x < width; x++)
            {
                double temp = (double)x / (width - 1);

                std::vector<std::pair<const Biome *, double>> candidates;
                for (auto &biome : biomes)
                {
                    double dx = temp - biome.temperature;
                    double dy = moist - biome.moisture;
                    double dist2 = dx * dx + dy * dy;
                    double w = 1.0 / (0.0001 + dist2);
                    candidates.push_back({&biome, w});
                }

                Biome result = Biome::blendMultiple(candidates);
                glm::vec3 col = glm::clamp(result.color, 0.0f, 1.0f);

                unsigned char r = (unsigned char)(col.r * 255);
                unsigned char g = (unsigned char)(col.g * 255);
                unsigned char b = (unsigned char)(col.b * 255);
                file << r << g << b;
            }
        }

        file.close();
    }
};