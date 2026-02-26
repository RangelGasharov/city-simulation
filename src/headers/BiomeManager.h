
#pragma once
#include "Biome.h"
#include "Perlin.h"
#include <vector>
#include <fstream>
#include <algorithm>
#include "../include/glm/glm.hpp"

class BiomeManager
{
private:
    Perlin tempNoise;
    Perlin moistNoise;
    Perlin contNoise;
    Perlin erosionNoise;
    Perlin weirdNoise;
    std::vector<Biome> biomes;

    double smoothRemap(double value, double inMin, double inMax);
    double fbm3D(glm::vec3 p, int octaves, double persistence, Perlin &noiseGen);

public:
    struct ClimatePoint
    {
        double temperature;
        double moisture;
        double continentalness;
        double erosion;
        double weirdness;
    };

    BiomeManager(unsigned int seed);

    ClimatePoint getClimate(glm::vec3 direction);
    Biome getBiomeAt(glm::vec3 direction);
    void exportWorldBiomeMap(int width, int height);
};