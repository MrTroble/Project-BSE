{
  "settings": {
    "shaderType": "fragment"
  },
  "codes": [
    {
      "code": [
        "struct TextureSet {",
        "  uint Diffuse;",
        "  uint Normal;",
        "  uint Specular;",
        "  uint EnvironmentMask;",
        "  uint Height;",
        "  uint Environment;",
        "  uint Multilayer;",
        "  uint Emissive;",
        "};",

        "struct Quadrant {",
        "  TextureSet base;",
        "};",

        "layout(binding=4) uniform QuadrantsBlock {",
        "  Quadrant quadrants[4];",
        "  float maxUV;",
        "} Quadrants;",

        "$next_in vec3 NORMALIN;",
        "$next_in vec3 COLORIN;",
        "$next_in vec2 UVIN;",
        "$next_in flat uint INDEX;",

        "layout(location=0) out vec4 COLOR;",
        "layout(location=1) out vec4 NORMAL;",
        "layout(location=2) out float ROUGHNESS;",
        "layout(location=3) out float METALLIC;",
        "layout(binding=0) uniform sampler samplertex;",
        "layout(binding=1) uniform texture2D colorTextures[192];",

        "vec4 colorFromQuadrant() {",
        "  Quadrant quadrant = Quadrants.quadrants[INDEX];",
        "  return texture(sampler2D(colorTextures[quadrant.base.Diffuse], samplertex), UVIN);",
        "}",

        "void main() {",
        "   ROUGHNESS = ~0;",
        "   METALLIC = ~0;",
        "   COLOR = vec4(COLORIN, 1) * colorFromQuadrant();",
        "   NORMAL = vec4(NORMALIN, 1);",
        "}"
      ]
    }
  ]
}