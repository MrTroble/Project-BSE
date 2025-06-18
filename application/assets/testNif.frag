{
  "settings": {
    "shaderType": "fragment"
  },
  "codes": [
    {
      "code": [
        "$next_in vec2 UVIN;"
      ],
      "dependsOn": [ "UV" ]
    },
    {
      "code": [
        "$next_in vec3 NORMALIN;"
      ],
      "dependsOn": [ "NORMAL" ]
    },
    {
      "code": [
        "$next_in vec4 COLORIN;"
      ],
      "dependsOn": [ "COLOR" ]
    },
    {
      "code": [
        "layout(binding=0) uniform sampler samplertex;",
        "layout(binding=1) uniform texture2D colorTexture;",
        "layout(binding=4) uniform texture2D normalTexture;"
      ],
      "dependsOn": [ "UV", "TEXTURES" ]
    },
    {
      "code": [
        "layout(location=0) out vec4 COLOR;",
        "layout(location=1) out vec4 NORMAL;",
        "layout(location=2) out int MATERIAL_ID;",
        "layout(push_constant) uniform constants { int id; } pushConst;",
        "",
        "void main() {",
        "   NORMAL = vec4(1, 1, 1, 1);",
        "   COLOR = vec4(1, 1, 1, 1);"
      ]
    },
    {
      "code": [
        "   COLOR = texture(sampler2D(colorTexture, samplertex), UVIN);",
        "   NORMAL = texture(sampler2D(normalTexture, samplertex), UVIN);"
      ],
      "dependsOn": [ "UV", "TEXTURES" ]
    },
    {
      "code": [
        "   COLOR *= COLORIN;"
      ],
      "dependsOn": [ "COLOR" ]
    },
    {
      "code": [
        "   NORMAL *= vec4(NORMALIN, 1);"
      ],
      "dependsOn": [ "NORMAL" ]
    },
    {
      "code": [
        "   if(abs(COLOR.a - 1) < 0.01f) MATERIAL_ID = pushConst.id;",
        "}"
      ]
    }
  ]
}