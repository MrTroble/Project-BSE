{
	"settings": {
		"shaderType": "vertex"
	},
	"codes": [
		{
			"code": [
				"$next_in vec4 POSITION;",
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
				"void main() {",
				"   gl_Position = proj.proj * mvp.matrix * POSITION;",
				"}"
			]
		}
	]
}