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
        return t * t * (3.0 - 2.0 * t);
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
        biomes.push_back(Biome("Deep Ocean", {0.0f, 0.05f, 0.3f},
                               -60.0, 5.0, 4, 0.5, 2.0, 0.5, 0.5, 0.5, -0.8, 0.5, 0.2));
        biomes.push_back(Biome("Ocean", {0.0f, 0.2f, 0.6f},
                               -25.0, 8.0, 4, 0.5, 2.0, 0.6, 0.5, 0.6, -0.5, 0.4, 0.3));
        biomes.push_back(Biome("Beach", {0.9f, 0.8f, 0.5f},
                               1.0, 2.0, 3, 0.5, 2.0, 0.4, 0.7, 0.4, -0.1, 0.8, 0.2));
        biomes.push_back(Biome("Plains", {0.4f, 0.7f, 0.2f},
                               10.0, 15.0, 6, 0.5, 2.0, 0.8, 0.6, 0.5, 0.1, 0.4, 0.3));
        biomes.push_back(Biome("Jungle", {0.0f, 0.4f, 0.0f},
                               15.0, 40.0, 8, 0.6, 2.1, 1.1, 0.9, 0.9, 0.2, 0.7, 0.5));
        biomes.push_back(Biome("Forest", {0.1f, 0.5f, 0.1f},
                               12.0, 25.0, 6, 0.5, 2.0, 1.0, 0.5, 0.7, 0.2, 0.5, 0.4));
        biomes.push_back(Biome("Desert", {0.9f, 0.8f, 0.4f},
                               8.0, 10.0, 4, 0.5, 2.0, 0.7, 0.9, 0.1, 0.3, 0.2, 0.5));
        biomes.push_back(Biome("Tundra", {0.7f, 0.8f, 0.8f},
                               5.0, 10.0, 5, 0.5, 2.0, 0.9, 0.1, 0.3, 0.3, 0.5, 0.3));
        biomes.push_back(Biome("Mountains", {0.5f, 0.5f, 0.55f},
                               60.0, 180.0, 12, 0.5, 2.4, 1.3, 0.3, 0.4, 0.7, 0.3, 0.8));

        biomes.push_back(Biome("Snowy Peaks", {1.0f, 1.0f, 1.0f},
                               100.0, 250.0, 14, 0.55, 2.5, 1.5, 0.0, 0.4, 0.8, 0.2, 0.9));
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

        double globalScale = 0.00001;
        double qx = contNoise.noise(x * globalScale, z * globalScale, 0.0) * 2000.0;
        double qz = contNoise.noise(x * globalScale + 5.2, z * globalScale + 1.3, 0.0) * 2000.0;

        double wx = x + qx;
        double wz = z + qz;

        auto fbm = [&](double nx, double nz, int octaves, double persistence)
        {
            double total = 0;
            double frequency = 0.000004;
            double amplitude = 1.0;
            double maxValue = 0;
            for (int i = 0; i < octaves; i++)
            {
                total += contNoise.noise(nx * frequency, nz * frequency, 0.0) * amplitude;
                maxValue += amplitude;
                amplitude *= persistence;
                frequency *= 2.0;
            }
            return total / maxValue;
        };

        double contRaw = fbm(wx, wz, 6, 0.5);
        double ridge = 1.0 - std::abs(contRaw);
        double finalContRaw = (contRaw * 0.8) + (ridge * 0.2);

        double waterThreshold = 0.25;
        double adjustedCont = finalContRaw - waterThreshold;

        double cont;
        const double eps = 0.005;

        if (adjustedCont < -eps)
        {
            cont = -std::pow(-adjustedCont, 0.6) * 3.0;
        }
        else if (adjustedCont > eps)
        {
            cont = std::pow(adjustedCont, 0.5) * 2.0;
        }
        else
        {
            double t = (adjustedCont + eps) / (2.0 * eps);
            double s = t * t * (3.0 - 2.0 * t);
            double waterVal = -std::pow(eps, 0.6) * 3.0;
            double landVal = std::pow(eps, 0.5) * 2.0;
            cont = waterVal + s * (landVal - waterVal);
        }

        double erosionFactor = remap(erosionNoise.noise(wx * 0.0001, wz * 0.0001, 20.0));
        if (cont > 0)
        {
            cont *= (1.1 - erosionFactor);
        }

        double climateScale = 0.000001;
        return {
            remap(tempNoise.noise(wx * climateScale, wz * climateScale, 0.0)),
            remap(moistNoise.noise(wx * climateScale, wz * climateScale, 10.0)),
            cont,
            remap(erosionNoise.noise(wx * climateScale, wz * climateScale, 20.0)),
            remap(weirdNoise.noise(wx * climateScale, wz * climateScale, 30.0))};
    }

    Biome getBiomeAt(double x, double z)
    {
        ClimatePoint climate = getClimate(x, z);
        std::vector<std::pair<const Biome *, double>> candidates;
        for (auto &biome : biomes)
        {
            double dx = climate.temperature - biome.temperature;
            double dy = climate.moisture - biome.moisture;
            double dz = (climate.continentalness - biome.continentalness) * 1.5;
            double de = climate.erosion - biome.erosion;
            double dw = climate.weirdness - biome.weirdness;
            double dist2 = dx * dx + dy * dy + dz * dz + de * de + dw * dw;

            double w = 1.0 / (0.0001 + dist2);
            w = pow(w, 4.0);

            candidates.push_back({&biome, w});
        }

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