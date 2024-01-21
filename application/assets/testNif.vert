{
    "settings": {
        "shaderType": "vertex"
    },
    "codes": [
        {
          "code": [
            "struct ValueSystem {",
            "   mat4 model;",
            "   mat4 normalModel;",
            "   vec4 color;",
            "   vec4 padding[7];",
            "};",
            "layout(binding=2) uniform _system { ValueSystem values; } system;",
            "layout(binding=3) uniform PROJ {",
            "   mat4 proj;",
            "} proj;",
            "$next_in vec3 POSITION;",
            "$next_out vec4 POSITIONOUT;",
            "out gl_PerVertex {",
            "   vec4 gl_Position;",
            "};"
          ]
        },
        {
            "code": [
                "$next_in vec2 UVIN;",
                "$next_out vec2 UVOUT;"
            ],
            "dependsOn": [
                "UV"
            ]
        },
        {
            "code": [
                "$next_in vec3 NORMALIN;",
                "$next_out vec3 NORMALOUT;"
            ],
            "dependsOn": [
                "NORMAL"
            ]
        },
        {
            "code": [
                "$next_in vec4 COLORIN;",
                "$next_out vec4 COLOROUT;"
            ],
            "dependsOn": [
                "COLOR"
            ]
        },
        {
          "code": [
            "void main() {",
            "   POSITIONOUT = system.values.model * vec4(POSITION, 1);",
            "   gl_Position = proj.proj * POSITIONOUT;"
          ]
        },
        {
            "code": [
                "   UVOUT = UVIN;"
            ],
            "dependsOn": [
                "UV"
            ]
        },
        {
            "code": [
                "   NORMALOUT = NORMALIN;"
            ],
            "dependsOn": [
                "NORMAL"
            ]
        },
        {
            "code": [
                "   COLOROUT = COLORIN;"
            ],
            "dependsOn": [
                "COLOR"
            ]
        },
        {
            "code": [
                "}"
            ]
        }
    ]
}