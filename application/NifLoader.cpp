#include "NifLoader.hpp"
#include <NifFile.hpp>
#include <TGEngine.hpp>

namespace tge::nif {

	using namespace tge::graphics;
	using namespace tge::main;

	size_t load(const std::string& name, void* shaderPipe) {
		const auto api = getAPILayer();
		const auto ggm = getGameGraphicsModule();
		const nifly::NifFile file(name);
		const auto& shapes = file.GetShapes();

		std::vector<RenderInfo> renderInfos;
		renderInfos.resize(shapes.size());

		std::vector<void*> dataPointer;
		std::vector<size_t> sizes;
		for (const auto shape : shapes) {
			const auto geom = shape->GetGeomData();
		}
		const auto indexBufferID = api->pushData(dataPointer.size(), dataPointer.data(), sizes.data(), DataType::VertexIndexData);

		size_t current = 0;
		for (const auto shape : shapes) {
			auto& info = renderInfos[current];
			const auto geom = shape->GetGeomData();
			info.indexCount = geom->GetNumTriangles() == 0 ? geom->GetNumVertices() : geom->GetNumTriangles();
			info.indexSize = geom->GetNumTriangles() == 0 ? IndexSize::NONE:IndexSize::UINT16;
			info.indexBuffer += indexBufferID;
			info.vertexBuffer.push_back(current);
			current++;
		}
		api->pushRender(renderInfos.size(), renderInfos.data());
		return -1;
	}
}