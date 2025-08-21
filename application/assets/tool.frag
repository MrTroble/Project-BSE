{
  "settings": {
    "shaderType": "fragment"
  },
  "codes": [
    {
      "code": [
        "layout(location=0) out vec4 COLOR;",
        "layout(location=1) out int MATERIAL_ID;",
        "layout(push_constant) uniform constants { vec4 color; int id; } pushConst;",
        "",
        "void main() {",
        "   COLOR = pushConst.color;",
        "   MATERIAL_ID = pushConst.id;",
        "}"
      ]
    }
  ]
}