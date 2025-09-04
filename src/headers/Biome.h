#pragma once
#include <string>
#include "../include/glm/glm.hpp"
#include <algorithm>

struct Biome
{
  std::string name;
  double baseHeight;
  double heightScale;
  int octaves;
  double persistence;
  double lacunarity;
  double roughness;
  glm::vec3 color;

  Biome(std::string name,
        double baseHeight,
        double heightScale,
        int octaves,
        double persistence,
        double lacunarity,
        double roughness,
        glm::vec3 color)
      : name(name), baseHeight(baseHeight), heightScale(heightScale), octaves(octaves),
        persistence(persistence), lacunarity(lacunarity),
        roughness(roughness), color(color) {}

  static Biome blend(const Biome &a, const Biome &b, double t)
  {
    t = std::clamp(t, 0.0, 1.0);
    double transition = t * t * (3.0 - 2.0 * t);

    double blendedBase = a.baseHeight * (1.0 - transition) + b.baseHeight * transition;
    double blendedScale = a.heightScale * (1.0 - transition) + b.heightScale * transition;

    return Biome(
        "Blend(" + a.name + "," + b.name + ")",
        blendedBase,
        blendedScale,
        static_cast<int>(std::round(a.octaves * (1 - transition) + b.octaves * transition)),
        a.persistence * (1 - transition) + b.persistence * transition,
        a.lacunarity * (1 - transition) + b.lacunarity * transition,
        a.roughness * (1 - transition) + b.roughness * transition,
        glm::mix(a.color, b.color, static_cast<float>(transition)));
  }
};