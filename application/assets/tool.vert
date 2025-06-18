{
	"settings": {
		"shaderType": "vertex"
	},
	"codes": [
		{
			"code": [
				"$next_in vec4 POSITION;",
				"out gl_PerVertex {",
				"   vec4 gl_Position;",
				"};"
			]
		},
		{
			"code": [
				"void main() {",
				"   gl_Position = POSITION;",
				"}"
			]
		}
	]
}