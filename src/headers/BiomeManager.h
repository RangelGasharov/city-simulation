#pragma once
#include "Biome.h"
#include "Perlin.h"
#include <vector>

class BiomeManager
{
private:
    Perlin biomeNoise;

public:
    std::vector<Biome> biomes;

    BiomeManager(unsigned int seed)
        : biomeNoise(seed)
    {
        biomes.push_back(Biome("Plains", 10.0, 10.0, 4, 0.5, 2.0, 0.8, {0.0f, 1.0f, 0.0f}));
        biomes.push_back(Biome("Hills", 20.0, 50.0, 6, 0.5, 2.0, 1.0, {1.0f, 0.0f, 0.0f}));
        biomes.push_back(Biome("Mountains", 80.0, 200.0, 8, 0.45, 2.2, 1.2, {0.0f, 0.0f, 1.0f}));
    }

    Biome getBiomeAt(double x, double z)
    {
        double val = biomeNoise.noise(x * 0.001, z * 0.001, 0.0);
        Biome plains = biomes[0];
        Biome hills = biomes[1];
        Biome mountains = biomes[2];
        double t;
        if (val < 0.2)
        {
            t = val / 0.2;
            t = t * t * (3 - 2 * t);
            return Biome::blend(plains, hills, t);
        }
        else
        {
            t = (val - 0.2) / 0.8;
            t = t * t * (3 - 2 * t);
            return Biome::blend(hills, mountains, t);
        }
    }
};