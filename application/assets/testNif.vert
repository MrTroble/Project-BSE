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
				"};"
			]
		},
		{
			"code": [
				"$next_in vec2 UVIN;",
				"$next_out vec2 UVOUT;"
			],
			"dependsOn": [ "UV" ]
		},
		{
			"code": [
				"$next_in vec3 NORMALIN;",
				"$next_out vec3 NORMALOUT;"
			],
			"dependsOn": [ "NORMAL" ]
		},
		{
			"code": [
				"$next_in vec4 COLORIN;",
				"$next_out vec4 COLOROUT;"
			],
			"dependsOn": [ "COLOR" ]
		},
		{
			"code": [
				"void main() {",
				"   gl_Position = proj.proj * mvp.matrix * vec4(POSITION, 1);"
			]
		},
		{
			"code": [
				"   UVOUT = UVIN;"
			],
			"dependsOn": [ "UV" ]
		},
		{
			"code": [
				"   NORMALOUT = NORMALIN;"
			],
			"dependsOn": [ "NORMAL" ]
		},
		{
			"code": [
				"   COLOROUT = COLORIN;"
			],
			"dependsOn": [ "COLOR" ]
		},
		{
			"code": [
				"}"
			]
		}
	]
}