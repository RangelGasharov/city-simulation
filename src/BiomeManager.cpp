#include "headers/BiomeManager.h"

BiomeManager::BiomeManager(unsigned int seed)
    : tempNoise(seed),
      moistNoise(seed + 101),
      contNoise(seed + 202),
      erosionNoise(seed + 303),
      weirdNoise(seed + 404)
{

    biomes.push_back(Biome("Deep Ocean", {0.0f, 0.05f, 0.3f}, -60.0, 5.0, 4, 0.5, 2.0, 0.5, 0.5, 0.5, -0.8, 0.5, 0.2));
    biomes.push_back(Biome("Ocean", {0.0f, 0.2f, 0.6f}, -25.0, 8.0, 4, 0.5, 2.0, 0.6, 0.5, 0.6, -0.4, 0.4, 0.3));

    biomes.push_back(Biome("Plains", {0.4f, 0.7f, 0.2f}, 10.0, 15.0, 6, 0.5, 2.0, 0.8, 0.5, 0.4, 0.2, 0.3, 0.2));
    biomes.push_back(Biome("Forest", {0.1f, 0.5f, 0.1f}, 12.0, 25.0, 6, 0.5, 2.0, 1.0, 0.5, 0.8, 0.2, 0.5, 0.4));
    biomes.push_back(Biome("Beach", {1.0f * 244 / 255, 1.0f * 232 / 255, 1.0f * 124 / 255}, 1.0, 2.0, 3, 0.5, 2.0, 0.4, 0.7, 0.4, 0.003, 0.8, 0.35));

    biomes.push_back(Biome("Jungle", {0.0f, 0.3f, 0.0f}, 15.0, 40.0, 8, 0.6, 2.1, 1.1, 0.8, 0.95, 0.2, 0.6, 0.4));
    biomes.push_back(Biome("Sand Desert", {0.9f, 0.8f, 0.4f}, 8.0, 10.0, 4, 0.5, 2.0, 0.7, 0.9, 0.1, 0.3, 0.2, 0.3));
    biomes.push_back(Biome("Badlands", {0.8f, 0.4f, 0.2f}, 20.0, 60.0, 8, 0.6, 2.0, 0.7, 0.9, 0.1, 0.3, 0.8, 0.3));

    biomes.push_back(Biome("Tundra", {0.7f, 0.8f, 0.8f}, 5.0, 10.0, 5, 0.5, 2.0, 0.9, 0.2, 0.3, 0.2, 0.5, 0.3));
    biomes.push_back(Biome("Snow Desert", {0.8f, 0.9f, 0.9f}, 8.0, 10.0, 4, 0.5, 2.0, 0.7, 0.1, 0.05, 0.3, 0.2, 0.4));

    /* biomes.push_back(Biome("Mountains", {0.5f, 0.5f, 0.55f}, 40.0, 450.0, 12, 0.5, 2.4, 1.3, 0.4, 0.4, 0.6, 0.2, 0.8));
     biomes.push_back(Biome("Snowy Peaks", {1.0f, 1.0f, 1.0f}, 100.0, 650.0, 14, 0.55, 2.5, 1.5, 0.1, 0.3, 0.7, 0.1, 1.0));*/
}

// Biome river = Biome("River", {0.2f, 0.4f, 0.8f}, -1.0, 5.0, 4, 0.5, 2.0, 0.5, 0.7, 0.6, -0.5, 0.5, 0.5);

double BiomeManager::smoothRemap(double value, double inMin, double inMax)
{
    double t = (value - inMin) / (inMax - inMin);
    t = std::clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double BiomeManager::fbm3D(glm::vec3 p, int octaves, double persistence, Perlin &noiseGen)
{
    double total = 0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0;
    for (int i = 0; i < octaves; i++)
    {
        total += noiseGen.noise(p.x * frequency, p.y * frequency, p.z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    return total / maxValue;
}

BiomeManager::ClimatePoint BiomeManager::getClimate(glm::vec3 direction)
{
    glm::vec3 p = glm::normalize(direction);

    double latitude = std::abs(p.y);
    double tempBase = 1.0 - std::pow(latitude, 1.7);
    double tFreq = 10.0;
    double tNoise = tempNoise.fractalNoise(p.x * tFreq, p.y * tFreq, p.z * tFreq, 4, 0.5, 2.0);
    double temperature = tempBase * 0.8 + tNoise * 0.2;

    double contRaw = fbm3D(direction * 1.2f, 8, 0.55, contNoise);

    double cont;
    if (contRaw > 0)
    {
        cont = std::pow(contRaw, 0.45);
        cont -= 0.1;
    }
    else
    {
        cont = -std::pow(std::abs(contRaw), 0.25);
        cont -= 0.15;
    }

    double mFreq = 2.30;
    // double mRaw = (moistNoise.noise(p.x * mFreq, p.y * mFreq, p.z * mFreq) + 1.0) * 0.5;
    double mRaw = (moistNoise.fractalNoise(p.x * mFreq, p.y * mFreq, p.z * mFreq, 4, 0.5, 2.0) + 1.0) * 0.5;
    double moisture = 3.0 * mRaw * mRaw - 2.0 * mRaw * mRaw * mRaw;

    double wFreq = 2.80;
    // double wRaw = (weirdNoise.noise(p.x * wFreq, p.y * wFreq, p.z * wFreq) + 1.0) * 0.5;
    double wRaw = (weirdNoise.fractalNoise(p.x * wFreq, p.y * wFreq, p.z * wFreq, 4, 0.5, 2.0) + 1.0) * 0.5;
    double weirdness = std::pow(wRaw, 2.2);

    double eFreq = 5.80;
    // double eRaw = (erosionNoise.noise(p.x * eFreq, p.y * eFreq, p.z * eFreq) + 1.0) * 0.5;
    double eRaw = (erosionNoise.fractalNoise(p.x * eFreq, p.y * eFreq, p.z * eFreq, 4, 0.5, 2.0) + 1.0) * 0.5;
    double erosion = std::pow(eRaw, 1.1);

    return {
        temperature,
        moisture,
        cont,
        erosion,
        weirdness};
}

Biome BiomeManager::getBiomeAt(glm::vec3 direction)
{
    ClimatePoint climate = getClimate(direction);
    std::vector<std::pair<const Biome *, double>> candidates;

    double minSquareDist = 1e10;
    std::vector<double> dists2;

    for (auto &biome : biomes)
    {
        double dx = (climate.temperature - biome.temperature) * 5.0;
        double dy = (climate.moisture - biome.moisture) * 2.0;
        double dz = (climate.continentalness - biome.continentalness) * 3.5;
        double de = (climate.erosion - biome.erosion) * 2.5;
        double dw = (climate.weirdness - biome.weirdness) * 8.0;

        double d2 = dx * dx + dy * dy + dz * dz + de * de + dw * dw;
        dists2.push_back(d2);

        if (d2 < minSquareDist)
            minSquareDist = d2;
    }

    double blendingSharpness = 8.0;

    for (size_t i = 0; i < biomes.size(); ++i)
    {
        double w = std::pow(minSquareDist / (dists2[i] + 0.0001), blendingSharpness);

        if (w > 0.0001)
        {
            candidates.push_back({&biomes[i], w});
        }
    }

    return Biome::blendMultiple(candidates);
}

void BiomeManager::exportWorldBiomeMap(int width, int height)
{
    time_t now = time(0);
    char buf[80];
    struct tm tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);

    std::string filename = "planet-biomes-" + std::string(buf) + ".ppm";
    std::ofstream file(filename, std::ios::binary);

    file << "P6\n"
         << width << " " << height << "\n255\n";

    for (int y = 0; y < height; y++)
    {
        double theta = (double)y / (height - 1) * M_PI;

        for (int x = 0; x < width; x++)
        {
            double phi = (1 - (double)x) / (width - 1) * 2.0 * M_PI;

            glm::vec3 direction;
            direction.x = std::sin(theta) * std::cos(phi);
            direction.y = std::cos(theta);
            direction.z = std::sin(theta) * std::sin(phi);

            Biome result = getBiomeAt(direction);

            glm::vec3 col = glm::clamp(result.color, 0.0f, 1.0f);
            unsigned char rgb[3] = {
                (unsigned char)(col.r * 255),
                (unsigned char)(col.g * 255),
                (unsigned char)(col.b * 255)};
            file.write(reinterpret_cast<char *>(rgb), 3);
        }
    }
    file.close();
}