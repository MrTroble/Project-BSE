#include "Gizmos.hpp"
#include <graphics/Material.hpp>
#include <graphics/GameShaderModule.hpp>
#include <glm/glm.hpp>
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <Util.hpp>

constexpr std::array<glm::vec4, 8> unitCubePoints = {
	glm::vec4{0.0, 0.0, 0.0, 1},
	glm::vec4{0.0, 1.0, 0.0, 1},
	glm::vec4{1.0, 1.0, 0.0, 1},
	glm::vec4{1.0, 0.0, 0.0, 1},
	glm::vec4{0.0, 0.0, 1.0, 1},
	glm::vec4{0.0, 1.0, 1.0, 1},
	glm::vec4{1.0, 1.0, 1.0, 1},
	glm::vec4{1.0, 0.0, 1.0, 1},
};
constexpr std::array<uint16_t, 36> indexToCube = {
	2, 6, 7, 2, 7, 3,//
	0, 4, 5, 0, 5, 1,//
	6, 2, 1, 6, 1, 5,//
	3, 7, 4, 3, 4, 0,//
	7, 6, 5, 7, 5, 4,//
	2, 3, 0, 2, 0, 1
};

inline void transformAllVertexPoints(const glm::mat4 value, std::span<glm::vec4> toChange) {
	for (auto& vertex : toChange) {
		vertex = value * vertex;
	}
}

GizmoLibrary loadLibrary(tge::graphics::APILayer* api) {
	using namespace tge;
	using namespace tge::graphics;
	GizmoLibrary library;

	BufferInfo indexBuffer{ indexToCube.data(), indexToCube.size() * sizeof(uint32_t), DataType::IndexData };

	auto cube = unitCubePoints;
	transformAllVertexPoints(glm::scale(glm::identity<glm::mat4>(), glm::vec3(1.5)), cube);
	BufferInfo cubeBuffer{ cube.data(), cube.size() * sizeof(glm::vec4), DataType::VertexData };

	auto dirX = unitCubePoints;
	transformAllVertexPoints(glm::scale(glm::identity<glm::mat4>(), glm::vec3(3, 0, 0)), cube);
	BufferInfo dirXBuffer{ dirX.data(), dirX.size() * sizeof(glm::vec4), DataType::VertexData };

	auto dirY = unitCubePoints;
	transformAllVertexPoints(glm::scale(glm::identity<glm::mat4>(), glm::vec3(0, 3, 0)), cube);
	BufferInfo dirYBuffer{ dirY.data(), dirY.size() * sizeof(glm::vec4), DataType::VertexData };

	auto dirZ = unitCubePoints;
	transformAllVertexPoints(glm::scale(glm::identity<glm::mat4>(), glm::vec3(0, 0, 3)), cube);
	BufferInfo dirZBuffer{ dirZ.data(), dirZ.size() * sizeof(glm::vec4), DataType::VertexData };

	std::array values = { indexBuffer, dirXBuffer, dirYBuffer, dirZBuffer, cubeBuffer };
	const auto dataHolders = api->pushData(values, "bufferForTransform");

	auto shaderPipe = api->getShaderAPI()->loadShaderPipeAndCompile({ "assets/tool.vert", "assets/tool.frag" });

	Material material(shaderPipe);
	material.target = RenderTarget::TRANSLUCENT_TARGET;
	const auto materialIDs = api->pushMaterials(std::array{ material });

	RenderInfo block;
	block.indexBuffer = dataHolders[0];
	block.indexCount = indexToCube.size();
	block.indexSize = IndexSize::UINT16;
	block.vertexBuffer.emplace_back();
	block.constRanges.emplace_back();
	block.constRanges[0].type = tge::shader::ShaderType::FRAGMENT;
	block.materialId = materialIDs[0];
	std::array<RenderInfo, values.size() - 1> renderInfos;
	uint32_t index = 1;
	for (auto& value : renderInfos)
	{
		block.vertexBuffer[0] = dataHolders[index];
		std::vector<std::byte> data(4);
		*((uint32_t*)data.data()) = index;
		block.constRanges[0].pushConstData = data;
		value = block;
		index++;
	}
	const auto render = api->pushRender(renderInfos, {}, RenderTarget::TRANSLUCENT_TARGET);
	return library;
}