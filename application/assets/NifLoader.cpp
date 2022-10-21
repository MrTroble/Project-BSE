#include "NifLoader.hpp"
#include <NifFile.hpp>
#include <TGEngine.hpp>

namespace tge::nif {

	using namespace tge::graphics;
	using namespace tge::main;

	Error NifModule::init()
	{
		return Error();
	}

	size_t NifModule::load(const std::string& name, void* shaderPipe) const {
		const auto api = getAPILayer();
		const auto ggm = getGameGraphicsModule();
		const auto sha = api->getShaderAPI();
		nifly::NifFile file(name);
		const auto& shapes = file.GetShapes();

		std::vector<RenderInfo> renderInfos;
		renderInfos.resize(shapes.size());

		std::vector<const void*> dataPointer;
		std::vector<size_t> sizes;
		size_t current = 0;
		dataPointer.reserve(shapes.size() * 5);
		sizes.reserve(shapes.size() * 5);
		for (auto shape : shapes) {
			nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
			if (!bishape)
				continue;
			auto& info = renderInfos[current];
			info.vertexBuffer.push_back(dataPointer.size());

			const auto& verticies = bishape->UpdateRawVertices();
			dataPointer.push_back(verticies.data());
			sizes.push_back(verticies.size() * sizeof(nifly::Vector3));

			std::vector<std::Triangle> triangles;
			shape->GetTriangles(triangles);
			if (!triangles.empty()) {
				info.indexBuffer = dataPointer.size();
				sizes.push_back(triangles.size() * sizeof(std::Triangle));
				dataPointer.push_back(triangles.data());
				info.indexCount = triangles.size() * 3;
				info.indexSize = IndexSize::UINT16;
			}
			else {
				info.indexCount = verticies.size();
				info.indexSize = IndexSize::NONE;
			}
			current++;
		}

		const auto indexBufferID = api->pushData(dataPointer.size(), dataPointer.data(), sizes.data(), DataType::VertexIndexData);

		std::vector<tge::graphics::NodeInfo> nodeInfos;
		nodeInfos.resize(shapes.size() + 1);
		current = 0;
		for (const auto shape : shapes) {
			auto& info = renderInfos[current];
			const auto geom = shape->GetGeomData();
			info.materialId = ggm->defaultMaterial;
			info.bindingID = sha->createBindings(ggm->defaultPipe);
			info.indexBuffer += indexBufferID;
			for (auto& index : info.vertexBuffer) {
				index += indexBufferID;
			}
			current++;
			const auto translate = shape->transform.translation;
			auto& nodeInfo = nodeInfos[current];
			nodeInfo.parent = 0;
			nodeInfo.bindingID = info.bindingID;
			nodeInfo.transforms.translation = glm::vec3(translate.x, translate.y, translate.z);
		}

		const auto nodes = ggm->addNode(nodeInfos.data(), nodeInfos.size());

		api->pushRender(renderInfos.size(), renderInfos.data());

		return nodes;
	}
}