#pragma once

#include <string>
#include <vector>

#define SkyrimSE 1

using ushort = unsigned short;
using ubyte = unsigned char;

inline std::string toInternal(const char* string) {
  return string == nullptr ? "" : string;
}

#if SkyrimSE
template <class Type>
struct TextureSetInternal {
  Type Diffuse = {};
  Type Normal = {};
  Type Specular = {};
  Type EnvironmentMask = {};
  Type Height = {};
  Type Environment = {};
  Type Multilayer = {};
  Type Emissive = {};
};

template <class Type>
inline TextureSetInternal<std::string> toInternal(
    const TextureSetInternal<Type>& sets) {
  return TextureSetInternal<std::string>{
      toInternal(sets.Diffuse),    toInternal(sets.Normal),
      toInternal(sets.Specular),   toInternal(sets.EnvironmentMask),
      toInternal(sets.Height),     toInternal(sets.Environment),
      toInternal(sets.Multilayer), toInternal(sets.Emissive)};
}
#endif

struct AlphaData {
  float Opacity = 0;
  ushort Position = 0;
};

template <class TextureSet>
struct AlphaLayer {
  TextureSet TextureSet;
  AlphaData* Data = nullptr;  // max 289 (pass by pointer)
  ushort DataLength = 0;
};

inline AlphaLayer<TextureSetInternal<std::string>> toInternal(
    const AlphaLayer<TextureSetInternal<const char*>>& string) {
  return {toInternal(string.TextureSet), string.Data, string.DataLength};
}

struct Quadrant {
  TextureSetInternal<const char*> BaseLayer;
  AlphaLayer<TextureSetInternal<const char*>>* AlphaLayers =
      nullptr;  // max 8 layers (pass by pointer)
  ubyte AlphaLayersLength = 0;
};

using AlphaLayers = std::vector<AlphaLayer<TextureSetInternal<std::string>>>;

struct QuadrantInternal {
  TextureSetInternal<std::string> BaseLayer;
  AlphaLayers AlphaLayers;
};

inline QuadrantInternal toInternal(const Quadrant& string) {
  AlphaLayers layers(string.AlphaLayersLength);
  const auto end = string.AlphaLayersLength + string.AlphaLayers;
  auto output = layers.begin();
  for (auto iter = string.AlphaLayers; iter != end; iter++, output++) {
    *output = toInternal(*iter);
  }
  return {toInternal(string.BaseLayer), layers};
}

template <class Quadrant>
struct CornerSets {
  Quadrant TopRight;
  Quadrant BottomRight;
  Quadrant TopLeft;
  Quadrant BottomLeft;
};
using CornerSetsDefault = CornerSets<Quadrant>;
using CornerSetsInternal = CornerSets<QuadrantInternal>;

template <class Quadrant>
inline auto forEachCorner(const CornerSets<Quadrant>& set) {
  return std::array{set.TopRight, set.BottomRight, set.TopLeft, set.BottomLeft};
}

inline CornerSetsInternal toInternal(const CornerSetsDefault& string) {
  return {toInternal(string.TopRight), toInternal(string.BottomRight),
          toInternal(string.TopLeft), toInternal(string.BottomLeft)};
}
