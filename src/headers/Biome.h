#pragma once
#include <string>
#include "../include/glm/glm.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

struct Biome
{
  std::string name;
  glm::vec3 color;

  double baseHeight;
  double heightScale;
  int octaves;
  double persistence;
  double lacunarity;
  double roughness;

  double temperature;
  double moisture;
  double continentalness;
  double erosion;
  double weirdness;

  Biome(std::string name,
        glm::vec3 color,

        double baseHeight,
        double heightScale,
        int octaves,
        double persistence,
        double lacunarity,
        double roughness,

        double temperature,
        double moisture,
        double continentalness,
        double erosion,
        double weirdness)
      : name(name), baseHeight(baseHeight), heightScale(heightScale), octaves(octaves),
        persistence(persistence), lacunarity(lacunarity),
        roughness(roughness), color(color),
        temperature(temperature), moisture(moisture),
        continentalness(continentalness), erosion(erosion), weirdness(weirdness)
  {
  }

  static Biome blend(const Biome &a, const Biome &b, double t)
  {
    t = std::clamp(t, 0.0, 1.0);
    double transition = t * t * t * (t * (t * 6 - 15) + 10);

    auto lerp = [&](double x, double y)
    {
      return x * (1.0 - transition) + y * transition;
    };

    double blendedBase = a.baseHeight * (1.0 - transition) + b.baseHeight * transition;
    double blendedScale = a.heightScale * (1.0 - transition) + b.heightScale * transition;

    return Biome(
        "Blend(" + a.name + "," + b.name + ")",
        glm::mix(a.color, b.color, static_cast<float>(transition)),
        lerp(a.baseHeight, b.baseHeight),
        lerp(a.heightScale, b.heightScale),
        static_cast<int>(std::round(lerp(a.octaves, b.octaves))),
        lerp(a.persistence, b.persistence),
        lerp(a.lacunarity, b.lacunarity),
        lerp(a.roughness, b.roughness),
        lerp(a.temperature, b.temperature),
        lerp(a.moisture, b.moisture),
        lerp(a.continentalness, b.continentalness),
        lerp(a.erosion, b.erosion),
        lerp(a.weirdness, b.weirdness));
  }

  static Biome blendMultiple(const std::vector<std::pair<const Biome *, double>> &inputs)
  {
    if (inputs.empty())
    {
      throw std::runtime_error("blendMultiple called with no inputs");
    }

    double totalW = 0.0;
    for (auto &in : inputs)
      totalW += in.second;

    auto safeDiv = [&](double x, double y)
    { return (y == 0.0 ? 0.0 : x / y); };

    double baseHeight = 0, heightScale = 0, octaves = 0;
    double persistence = 0, lacunarity = 0, roughness = 0;
    double temperature = 0, moisture = 0, minHeight = 0, maxHeight = 0;
    double continentalness = 0, erosion = 0, weirdness = 0;
    glm::vec3 color(0);

    std::string name = "BlendedBiome_";

    for (auto &in : inputs)
    {
      double w = safeDiv(in.second, totalW);
      const Biome *b = in.first;

      baseHeight += b->baseHeight * w;
      heightScale += b->heightScale * w;
      octaves += b->octaves * w;
      persistence += b->persistence * w;
      lacunarity += b->lacunarity * w;
      roughness += b->roughness * w;
      temperature += b->temperature * w;
      moisture += b->moisture * w;
      continentalness += b->continentalness * w;
      erosion += b->erosion * w;
      weirdness += b->weirdness * w;
      color += b->color * static_cast<float>(w);

      name += b->name;
    }

    return Biome(
        name,
        color,
        baseHeight,
        heightScale,
        static_cast<int>(std::round(octaves)),
        persistence,
        lacunarity,
        roughness,
        temperature,
        moisture,
        continentalness,
        erosion,
        weirdness);
  }
};