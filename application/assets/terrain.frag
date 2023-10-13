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

        "uint getQuadrant(vec2 uv) {",
        "  uint x = uint(floor(uv.x * 2.0f / Quadrants.maxUV));",
        "  uint y = uint(floor(uv.y * 2.0f / Quadrants.maxUV));",
        "  return x + y * uint(2);",
        "}",

        "$next_in vec3 NORMALIN;",
        "$next_in vec3 COLORIN;",
        "$next_in vec2 UVIN;",

        "layout(location=0) out vec4 COLOR;",
        "layout(location=1) out vec4 NORMAL;",
        "layout(location=2) out float ROUGHNESS;",
        "layout(location=3) out float METALLIC;",
        "layout(binding=0) uniform sampler samplertex;",
        "layout(binding=1) uniform texture2D colorTextures[192];",

        "vec4 colorFromQuadrant(uint select) {",
        "  Quadrant quadrant = Quadrants.quadrants[select];",
        "  return texture(sampler2D(colorTextures[quadrant.base.Diffuse], samplertex), UVIN);",
        "}",

        "void main() {",
        "   ROUGHNESS = 10000000000000000.0f;",
        "   METALLIC = 0;",
        "   uint index = getQuadrant(UVIN);",
        "   COLOR = vec4(COLORIN, 1) * colorFromQuadrant(index);",
        "   NORMAL = vec4(NORMALIN, 1);",
        "}"
      ]
    }
  ]
}