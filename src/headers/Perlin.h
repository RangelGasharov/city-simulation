#ifndef PERLIN_H
#define PERLIN_H

#include <vector>

class Perlin
{
private:
    std::vector<int> permutationTable;

    static double fade(double t);
    static double lerp(double a, double b, double t);
    static double grad(int hash, double x, double y, double z);

public:
    Perlin(unsigned int seed = 0);

    double noise(double x, double y, double z = 0.0) const;
    inline double noise(double x, double y) const
    {
        return noise(x, y, 0.0);
    }
    double fractalNoise(double x, double y, double z, int octaves, double persistence, double lacunarity) const;
    double ridgedNoise(double x, double y, double z, int octaves, double persistence, double lacunarity) const;
    inline double fractalNoise(double x, double y, int octaves, double persistence, double lacunarity) const
    {
        return fractalNoise(x, y, 0.0, octaves, persistence, lacunarity);
    };
};

#endif