{
	"settings": {
		"shaderType": "fragment"
	},
	"codes": [
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
				"$next_in vec3 NORMALIN;",
				"$next_in vec3 COLORIN;",
				"$next_in vec2 UVIN;",

				"layout(location=0) out vec4 COLOR;",
				"layout(location=1) out vec4 NORMAL;",
				"layout(location=2) out float ROUGHNESS;",
				"layout(location=3) out float METALLIC;",

				"void main() {",
				"   ROUGHNESS = 1.0f / 0.0f;",
				"   METALLIC = 0;",
				"   NORMAL = vec4(1, 1, 1, 1);",
				"   COLOR = vec4(COLORIN, 1);"
			]
		},
		{
			"code": [
				"   COLOR = texture(sampler2D(colorTexture, samplertex), UVIN);",
				"   NORMAL = texture(sampler2D(normalTexture, samplertex), UVIN);"
			],
			"dependsOn": [ "UV", "TEXTURES" ]
		},
		{
			"code": [
				"}"
			]
		}
	]
}