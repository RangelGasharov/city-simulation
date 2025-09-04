#include <cmath>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include "headers/Perlin.h"

Perlin::Perlin(unsigned int seed)
{
    permutationTable.resize(256);
    std::iota(permutationTable.begin(), permutationTable.end(), 0);

    std::default_random_engine engine(seed);
    std::shuffle(permutationTable.begin(), permutationTable.end(), engine);

    permutationTable.insert(permutationTable.end(), permutationTable.begin(), permutationTable.end());
}

double Perlin::fade(double t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double Perlin::lerp(double a, double b, double t)
{
    return a + t * (b - a);
}

double Perlin::grad(int hash, double x, double y, double z)
{
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

double Perlin::noise(double x, double y, double z) const
{
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int A = permutationTable[X] + Y;
    int AA = permutationTable[A] + Z;
    int AB = permutationTable[A + 1] + Z;
    int B = permutationTable[X + 1] + Y;
    int BA = permutationTable[B] + Z;
    int BB = permutationTable[B + 1] + Z;

    return lerp(
        lerp(
            lerp(grad(permutationTable[AA], x, y, z),
                 grad(permutationTable[BA], x - 1, y, z), u),
            lerp(grad(permutationTable[AB], x, y - 1, z),
                 grad(permutationTable[BB], x - 1, y - 1, z), u),
            v),
        lerp(
            lerp(grad(permutationTable[AA + 1], x, y, z - 1),
                 grad(permutationTable[BA + 1], x - 1, y, z - 1), u),
            lerp(grad(permutationTable[AB + 1], x, y - 1, z - 1),
                 grad(permutationTable[BB + 1], x - 1, y - 1, z - 1), u),
            v),
        w);
}

double Perlin::fractalNoise(double x, double y, double z, int octaves, double persistence, double lacunarity) const
{
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 1.0;

    double epsilon = 0.001;
    double gradientFactor = 0.001;

    for (int i = 0; i < octaves; i++)
    {
        double n = noise(x * frequency, y * frequency, z * frequency);

        double nx = (noise((x + epsilon) * frequency, y * frequency, z * frequency) -
                     noise((x - epsilon) * frequency, y * frequency, z * frequency)) /
                    (2.0 * epsilon);
        double ny = (noise(x * frequency, (y + epsilon) * frequency, z * frequency) -
                     noise(x * frequency, (y - epsilon) * frequency, z * frequency)) /
                    (2.0 * epsilon);

        double slope = std::sqrt(nx * nx + ny * ny);

        double scaledNoise = n * std::exp(-slope * gradientFactor);

        total += scaledNoise * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return std::max(-1.0, std::min(1.0, total / maxValue));
}

double Perlin::ridgedNoise(double x, double y, double z, int octaves, double persistence, double lacunarity) const
{
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 0.5;
    double maxValue = 0.0;

    for (int i = 0; i < octaves; i++)
    {
        double n = noise(x * frequency, y * frequency, z * frequency);
        n = 1.0 - fabs(n);
        total += n * amplitude;

        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}