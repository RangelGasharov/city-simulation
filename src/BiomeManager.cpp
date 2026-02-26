#include "headers/BiomeManager.h"

BiomeManager::BiomeManager(unsigned int seed)
    : tempNoise(seed),
      moistNoise(seed + 101),
      contNoise(seed + 202),
      erosionNoise(seed + 303),
      weirdNoise(seed + 404)
{
    biomes.push_back(Biome("Deep Ocean", {0.0f, 0.05f, 0.3f}, -60.0, 5.0, 4, 0.5, 2.0, 0.5, 0.5, 0.5, -0.8, 0.5, 0.2));
    biomes.push_back(Biome("Ocean", {0.0f, 0.2f, 0.6f}, -25.0, 8.0, 4, 0.5, 2.0, 0.6, 0.5, 0.6, -0.5, 0.4, 0.3));
    biomes.push_back(Biome("Beach", {0.9f, 0.8f, 0.5f}, 1.0, 2.0, 3, 0.5, 2.0, 0.4, 0.7, 0.4, -0.1, 0.8, 0.2));
    biomes.push_back(Biome("Plains", {0.4f, 0.7f, 0.2f}, 10.0, 15.0, 6, 0.5, 2.0, 0.8, 0.6, 0.5, 0.1, 0.4, 0.3));
    biomes.push_back(Biome("Jungle", {0.0f, 0.4f, 0.0f}, 15.0, 40.0, 8, 0.6, 2.1, 1.1, 0.9, 0.9, 0.2, 0.7, 0.5));
    biomes.push_back(Biome("Forest", {0.1f, 0.5f, 0.1f}, 12.0, 25.0, 6, 0.5, 2.0, 1.0, 0.5, 0.7, 0.2, 0.5, 0.4));
    biomes.push_back(Biome("Desert", {0.9f, 0.8f, 0.4f}, 8.0, 10.0, 4, 0.5, 2.0, 0.7, 0.9, 0.1, 0.3, 0.2, 0.5));
    biomes.push_back(Biome("Badlands", {0.8f, 0.4f, 0.2f}, 20.0, 40.0, 8, 0.6, 2.0, 0.7, 0.9, 0.1, 0.5, 0.5, 0.5));
    biomes.push_back(Biome("Tundra", {0.7f, 0.8f, 0.8f}, 5.0, 10.0, 5, 0.5, 2.0, 0.9, 0.1, 0.3, 0.3, 0.5, 0.3));
    biomes.push_back(Biome("Mountains", {0.5f, 0.5f, 0.55f}, 40.0, 120.0, 12, 0.5, 2.4, 1.3, 0.3, 0.4, 0.7, 0.3, 0.8));
    biomes.push_back(Biome("Snowy Peaks", {1.0f, 1.0f, 1.0f}, 100.0, 250.0, 14, 0.55, 2.5, 1.5, 0.0, 0.4, 0.8, 0.2, 0.9));
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

    float warpStrength = 0.005f;
    double qx = contNoise.noise(p.x + 1.1, p.y + 1.1, p.z + 1.1);
    double qy = contNoise.noise(p.x + 2.2, p.y + 2.2, p.z + 2.2);
    double qz = contNoise.noise(p.x + 3.3, p.y + 3.3, p.z + 3.3);
    glm::vec3 warpedP = glm::normalize(p + glm::vec3(qx, qy, qz) * warpStrength);

    double latitude = std::abs(p.y);
    double tempBase = 1.0 - std::pow(latitude, 1.2);
    double tNoise = tempNoise.noise(p.x * 2.0, p.y * 2.0, p.z * 2.0) * 0.15;
    double temperature = std::clamp(tempBase + tNoise, 0.0, 1.0);

    double cont = fbm3D(warpedP * 0.8f, 6, 0.5, contNoise);
    cont = cont - 0.1;

    double mNoise = (moistNoise.noise(warpedP.x * 2.5, warpedP.y * 2.5, warpedP.z * 2.5) + 1.0) * 0.5;
    double moisture = glm::mix(mNoise, 1.0 - std::abs(cont), 0.3);
    moisture = std::clamp(moisture, 0.0, 1.0);

    return {
        temperature,
        moisture,
        cont,
        (erosionNoise.noise(warpedP.x, warpedP.y, warpedP.z) + 1.0) * 0.5,
        (weirdNoise.noise(warpedP.x, warpedP.y, warpedP.z) + 1.0) * 0.5};
}

Biome BiomeManager::getBiomeAt(glm::vec3 direction)
{
    ClimatePoint climate = getClimate(direction);
    std::vector<std::pair<const Biome *, double>> candidates;

    double minSquareDist = 1e10;
    std::vector<double> dists2;

    for (auto &biome : biomes)
    {
        double dx = climate.temperature - biome.temperature;
        double dy = climate.moisture - biome.moisture;
        double dz = (climate.continentalness - biome.continentalness) * 1.5;
        double de = (climate.erosion - biome.erosion) * 0.5;
        double dw = (climate.weirdness - biome.weirdness) * 0.5;

        double d2 = dx * dx + dy * dy + dz * dz + de * de + dw * dw;
        dists2.push_back(d2);

        if (d2 < minSquareDist)
            minSquareDist = d2;
    }

    double blendingSharpness = 6.0;

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
            double phi = (double)x / (width - 1) * 2.0 * M_PI;

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