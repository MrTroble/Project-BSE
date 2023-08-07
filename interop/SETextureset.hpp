#pragma once

struct SETextureSet {
  const char* diffuse;
  const char* normal;
  const char* specular;
  const char* environmentMask;
  const char* height;
  const char* environment;
  const char* multilayer;
  const char* emissive;
};

struct SECornerSets {
  SETextureSet topRight;
  SETextureSet bottomRight;
  SETextureSet topLeft;
  SETextureSet bottomLeft;
};