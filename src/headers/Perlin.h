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
};

#endif