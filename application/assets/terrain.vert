{
  "settings": {
    "shaderType": "vertex"
  },
  "codes": [
    {
      "code": [
        "$next_in vec3 POSITION;",
        "layout(binding=2) uniform MVP {",
        "   mat4 matrix;",
        "} mvp;",
        "layout(binding=3) uniform PROJ {",
        "   mat4 proj;",
        "} proj;",
        "out gl_PerVertex {",
        "   vec4 gl_Position;",
        "};",

        "$next_in vec3 NORMALIN;",
        "$next_out vec3 NORMALOUT;",
        "$next_in vec3 COLORIN;",
        "$next_out vec3 COLOROUT;",
        "$next_in vec2 UVIN;",
        "$next_out vec2 UVOUT;",
        "$next_in uint INDEXIN;",
        "$next_out float INDEXX;",
        "$next_out float INDEXY;",

        "void main() {",
        "   gl_Position = proj.proj * mvp.matrix * vec4(POSITION, 1);",
        "   UVOUT = UVIN;",
        "   INDEXX = float(INDEXIN & 1);",
        "   INDEXY = float((INDEXIN & 2) >> 1);",
        "   NORMALOUT = NORMALIN;",
        "   COLOROUT = COLORIN;",
        "}"
      ]
    }
  ]
}