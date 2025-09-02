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