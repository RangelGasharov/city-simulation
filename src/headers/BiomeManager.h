#pragma once
#include "Biome.h"
#include "Perlin.h"
#include <vector>
#include <fstream>

class BiomeManager
{
private:
    Perlin tempNoise;
    Perlin moistNoise;
    Perlin contNoise;
    Perlin erosionNoise;
    Perlin weirdNoise;
    double smoothRemap(double value, double inMin, double inMax)
    {
        double t = (value - inMin) / (inMax - inMin);
        t = std::clamp(t, 0.0, 1.0);
        return t * t * (3 - 2 * t);
    }

public:
    std::vector<Biome> biomes;

    BiomeManager(unsigned int seed)
        : tempNoise(seed),
          moistNoise(seed + 101),
          contNoise(seed + 202),
          erosionNoise(seed + 303),
          weirdNoise(seed + 404)
    {
        biomes.push_back(Biome("Plains", {0.4f, 0.8f, 0.2f},
                               10.0, 10.0, 6, 0.5, 2.0, 1.0,
                               0.6, 0.6, 0.0, 0.5, 0.3));
        /*biomes.push_back(Biome("Hills", {0.6f, 0.4f, 0.2f},
                               40.0, 150.0, 16, 0.5, 2.6, 1.0,
                               0.4, 0.7, 0.2, 0.7, 0.4));
        biomes.push_back(Biome("Mountains", {0.5f, 0.5f, 0.6f},
                               60.0, 300.0, 16, 0.45, 2.6, 1.2,
                               0.2, 0.3, 0.8, 0.2, 0.8));*/
        /*biomes.push_back(Biome("Desert", {0.9f, 0.85f, 0.3f},
                               10.0, 5.0, 4, 0.5, 2.0, 0.8,
                               0.95, 0.05, 0.0, 0.1, 0.4));
        biomes.push_back(Biome("Savanna", {0.74f, 0.51f, 0.27f},
                               20.0, 20.0, 6, 0.5, 2.0, 1.0,
                               0.85, 0.4, 0.2, 0.3, 0.5));
        biomes.push_back(Biome("Taiga", {0.1f, 0.3f, 0.1f},
                               15.0, 30.0, 6, 0.5, 2.0, 1.0,
                               0.25, 0.65, 0.3, 0.5, 0.2));
        biomes.push_back(Biome("Tundra", {0.8f, 0.8f, 0.85f},
                               5.0, 15.0, 6, 0.5, 2.0, 1.0,
                               0.1, 0.2, 0.1, 0.2, 0.3));
        biomes.push_back(Biome("Swamp", {0.1f, 0.2f, 0.05f},
                               8.0, 8.0, 4, 0.6, 2.0, 0.9,
                               0.7, 0.9, 0.0, 0.8, 0.3));
        biomes.push_back(Biome("Jungle", {0.0f, 0.6f, 0.0f},
                               20.0, 60.0, 6, 0.6, 2.1, 1.0,
                               0.95, 0.95, 0.2, 0.9, 0.7));
        biomes.push_back(Biome("Snowy Mountains", {0.9f, 0.9f, 1.0f},
                               70.0, 220.0, 16, 0.45, 2.2, 1.3,
                               0.05, 0.3, 0.9, 0.3, 0.9));
        biomes.push_back(Biome("Mesa", {0.8f, 0.4f, 0.2f},
                               20.0, 40.0, 6, 0.5, 2.0, 1.0,
                               0.9, 0.2, 0.4, 0.4, 0.9));
        biomes.push_back(Biome("Ocean", {0.0f, 0.0f, 0.7f},
                               -20.0, 5.0, 4, 0.5, 2.0, 0.7,
                               0.5, 0.5, -0.9, 0.6, 0.2));
        biomes.push_back(Biome("Deep Ocean", {0.0f, 0.0f, 0.4f},
                               -50.0, 5.0, 6, 0.5, 2.0, 0.7,
                               0.5, 0.5, -1.0, 0.7, 0.4));*/
        // biomes.push_back(Biome("River", {0.2f, 0.4f, 0.8f}, -1.0, 5.0, 4, 0.5, 2.0, 0.5, 0.7, 0.6, -0.5, 0.5, 0.5));
    }

    // Biome river = Biome("River", {0.2f, 0.4f, 0.8f}, -1.0, 5.0, 4, 0.5, 2.0, 0.5, 0.7, 0.6, -0.5, 0.5, 0.5);

    struct ClimatePoint
    {
        double temperature;
        double moisture;
        double continentalness;
        double erosion;
        double weirdness;
    };

    ClimatePoint getClimate(double x, double z)
    {
        auto remap = [](double v)
        { return (v + 1.0) * 0.5; };
        double spreadingFactor = 0.0002;
        return {
            remap(tempNoise.noise(x * spreadingFactor, z * spreadingFactor, 0.0)),
            remap(moistNoise.noise(x * spreadingFactor, z * spreadingFactor, 0.0)),
            remap(contNoise.noise(x * spreadingFactor, z * spreadingFactor, 0.0)) * 2.0 - 1,
            remap(erosionNoise.noise(x * spreadingFactor, z * spreadingFactor, 0.0)),
            remap(weirdNoise.noise(x * spreadingFactor, z * spreadingFactor, 0.0))};
    }

    Biome getBiomeAt(double x, double z)
    {
        ClimatePoint climate = getClimate(x, z);
        std::vector<std::pair<const Biome *, double>> candidates;
        for (auto &biome : biomes)
        {
            double dx = climate.temperature - biome.temperature;
            double dy = climate.moisture - biome.moisture;
            double dz = climate.continentalness - biome.continentalness;
            double de = climate.erosion - biome.erosion;
            double dw = climate.weirdness - biome.weirdness;
            double dist2 = dx * dx + dy * dy + dz * dz + de * de + dw * dw;

            double w = 1.0 / (0.0001 + dist2);
            w = pow(w, 2.0);

            candidates.push_back({&biome, w});
        }

        /*double riverNoise = fabs(contNoise.noise(x * 0.001, z * 0.001, 0.0));
        double riverWeight = std::clamp(1.0 - (riverNoise / 0.01), 0.0, 1.0);
        riverWeight = pow(riverWeight, 4.0);

        if (riverWeight > 0.0)
        {
            candidates.push_back({&river, riverWeight * 300.0});
        }*/

        return Biome::blendMultiple(candidates);
    }

    void exportWorldBiomeMap(int width, int height,
                             double worldSizeX, double worldSizeZ)
    {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];

        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);

        std::string filename = "terrain-biomes-" + std::string(buf) + ".ppm";

        std::ofstream file(filename, std::ios::binary);
        file << "P6\n"
             << width << " " << height << "\n255\n";

        for (int y = 0; y < height; y++)
        {
            double worldZ = (double)y / (height - 1) * worldSizeZ;

            for (int x = 0; x < width; x++)
            {
                double worldX = (double)x / (width - 1) * worldSizeX;

                Biome result = getBiomeAt(worldX, worldZ);

                glm::vec3 col = glm::clamp(result.color, 0.0f, 1.0f);
                unsigned char r = static_cast<unsigned char>(col.r * 255);
                unsigned char g = static_cast<unsigned char>(col.g * 255);
                unsigned char b = static_cast<unsigned char>(col.b * 255);

                file.write(reinterpret_cast<char *>(&r), 1);
                file.write(reinterpret_cast<char *>(&g), 1);
                file.write(reinterpret_cast<char *>(&b), 1);
            }
        }
        file.close();
    }
};