{
  "settings": {
    "shaderType": "fragment"
  },
  "codes": [
    {
      "code": [
        "$next_in vec4 POSITION;"
      ]
    },
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
        "struct ValueSystem {",
        "   mat4 model;",
        "   mat4 normalModel;",
        "   vec4 color;",
        "   vec4 padding[7];",
        "};",
        "layout(binding=2) uniform _system { ValueSystem values; } system;",

        "layout(location=0) out vec4 COLOR;",
        "layout(location=1) out vec4 NORMAL;",
        "layout(location=2) out float ROUGHNESS;",
        "layout(location=3) out vec4 POSOUT;",
        "layout(push_constant) uniform constants { uint id; } pushConst;",
        "void main() {",
        "   ROUGHNESS = pushConst.id;",
        "   POSOUT = POSITION;",
        "   NORMAL = vec4(1, 1, 1, 1);",
        "   COLOR = vec4(1, 1, 1, 1);"
      ]
    },
    {
      "code": [
        "   COLOR = COLORIN;"
      ],
      "dependsOn": [ "COLOR" ]
    },
    {
      "code": [
        "   COLOR *= texture(sampler2D(colorTexture, samplertex), UVIN);",
        "   NORMAL = texture(sampler2D(normalTexture, samplertex), UVIN);"
      ],
      "dependsOn": [ "UV", "TEXTURES" ]
    },
    {
      "code": [
        "   NORMAL *= vec4(NORMALIN, 1);"
      ],
      "dependsOn": [ "NORMAL" ]
    },
    {
      "code": [
        "   if(COLOR.a < 1) discard;"
      ],
      "dependsOn": ["ALPHATEST"]
    },
    {
      "code": [
        "   NORMAL = system.values.normalModel * NORMAL;",
        "   COLOR.xyz = COLOR.xyz + system.values.color.xyz;",
        "}"
      ]
    }
  ]
}