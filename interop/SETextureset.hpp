#pragma once

#include <string>

struct SETextureSet {
  const char* diffuse = nullptr;
  const char* normal = nullptr;
  const char* specular = nullptr;
  const char* environmentMask = nullptr;
  const char* height = nullptr;
  const char* environment = nullptr;
  const char* multilayer = nullptr;
  const char* emissive = nullptr;
};

inline std::string toInternal(const char* string) {
  return "";
}

struct SETextureSetInternal {
  std::string diffuse;
  std::string normal;
  std::string specular;
  std::string environmentMask;
  std::string height;
  std::string environment;
  std::string multilayer;
  std::string emissive;

  SETextureSetInternal() = default;

  SETextureSetInternal(const SETextureSet& sets)
      : diffuse(toInternal(sets.diffuse)),
        normal(toInternal(sets.normal)),
        specular(toInternal(sets.specular)),
        environmentMask(toInternal(sets.environmentMask)),
        height(toInternal(sets.height)),
        environment(toInternal(sets.environment)),
        multilayer(toInternal(sets.multilayer)),
        emissive(toInternal(sets.emissive)) {}
};

struct SECornerSets {
  SETextureSet topRight;
  SETextureSet bottomRight;
  SETextureSet topLeft;
  SETextureSet bottomLeft;
};

struct SECornerSetsInternal {
  SETextureSetInternal topRight;
  SETextureSetInternal bottomRight;
  SETextureSetInternal topLeft;
  SETextureSetInternal bottomLeft;

  SECornerSetsInternal() = default;

  SECornerSetsInternal(const SECornerSets& sets)
      : topRight(sets.topRight),
        bottomRight(sets.bottomRight),
        topLeft(sets.topLeft),
        bottomLeft(sets.bottomLeft) {}
};