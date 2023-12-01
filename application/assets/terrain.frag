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
        "$next_in float INDEXX;",
        "$next_in float INDEXY;",

        "layout(location=0) out vec4 COLOR;",
        "layout(location=1) out vec4 NORMAL;",
        "layout(location=2) out float ROUGHNESS;",
        "layout(location=3) out float METALLIC;",
        "layout(binding=0) uniform sampler samplertex;",
        "layout(binding=1) uniform texture2D colorTextures[192];",

        "vec4 colorFromQuadrant() {",
        "  int x = clamp(0, 1, int(round(INDEXX)));",
        "  int y = clamp(0, 1, int(round(INDEXY)));",
        "  return vec4(INDEXX, INDEXY, 0, 1);",
        "  int quadrantID = x+2*y;",
        "  const Quadrant quadrant = Quadrants.quadrants[quadrantID];",
        "  //return vec4(vec3(0.25) * float(quadrantID), 1);",
        "  return texture(sampler2D(colorTextures[quadrant.base.Diffuse], samplertex), UVIN);",
        "}",

        "void main() {",
        "   ROUGHNESS = 0;",
        "   METALLIC = 0;",
        "   COLOR = colorFromQuadrant();",
        "   //COLOR = vec4(colorFromQuadrant().r * vec3(1), 1);",
        "   NORMAL = vec4(NORMALIN, 1);",
        "}"
      ]
    }
  ]
}