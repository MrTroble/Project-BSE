struct TextureSet {
  uint Diffuse;
  uint Normal;
  uint Specular;
  uint EnvironmentMask;
  uint Height;
  uint Environment;
  uint Multilayer;
  uint Emissive;
};

struct Quadrant {
  TextureSet base;
};

// texture(sampler2D(colorTexture, samplertex), UVIN);

layout(binding=4) uniform QuadrantsBlock { 
	Quadrant quadrants[4];
	float maxUV;
} Quadrants;

uint getQuadrant(vec2 uv) {
	uint x = uint(floor(uv.x * 2.0f / Quadrants.maxUV));
    uint y = uint(floor(uv.y * 2.0f / Quadrants.maxUV));
	return x + y * uint(2);
}

layout(binding=0) uniform sampler samplertex;
layout(binding=1) uniform texture2D colorTextures[192];

vec4 colorFromQuadrant(uint select) {
	Quadrant quadrant = Quadrants.quadrants[select];
	return texture(sampler2D(colorTextures[quadrant.base.Diffuse], samplertex), UVIN);
}