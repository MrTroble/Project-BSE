{
	"settings": {
		"shaderType": "fragment"
	},
	"codes": [
		{
			"code": [
				"layout(location=0) in vec2 UV;",
				"layout(location=0) out vec4 COLOR;",
				"layout(location=1) out vec4 NORMAL;",
				"layout(location=2) out float ROUGHNESS;",
				"layout(location=3) out float METALLIC;",
				"layout(binding=0) uniform sampler samp;",
				"layout(binding=1) uniform texture2D tex;",
				"void main() {",
				"   COLOR = vec4(1, 0, 0, 1);",
				"   NORMAL = vec4(1, 1, 1, 1);",
				"   ROUGHNESS = 0;",
				"   METALLIC = 0;",
				"}"
			]
		}
	]
}