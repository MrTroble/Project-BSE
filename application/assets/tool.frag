{
  "settings": {
    "shaderType": "fragment"
  },
  "codes": [
    {
      "code": [
        "layout(location=0) out vec4 COLOR;",
        "layout(location=2) out int MATERIAL_ID;",
        "layout(push_constant) uniform constants { int id; } pushConst;",
        "",
        "void main() {",
        "   COLOR = vec4(1, 1, 1, 1);",
        "   MATERIAL_ID = pushConst.id;",
        "}"
      ]
    }
  ]
}