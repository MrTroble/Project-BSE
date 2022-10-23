{
	"settings": {
		"shaderType": "vertex"
	},
	"codes": [
		{
			"code": [
				"layout(location=0) in vec3 POSITION;",
				"layout(location=0) out vec2 UVOUT;",
				"layout(binding=2) uniform MVP {",
				"   mat4 matrix;",
				"} mvp;",
				"layout(binding=3) uniform PROJ {",
				"   mat4 proj;",
				"} proj;",
				"out gl_PerVertex {",
				"   vec4 gl_Position;",
				"};",
				"void main() {",
				"   UVOUT = vec2(0,0);",
				"   gl_Position = mvp.matrix * proj.proj * vec4(POSITION, 1);",
				"}"
			]
		}
	]
}