#include "NifLoader.hpp"
#include <NifFile.hpp>
#include <TGEngine.hpp>
#include <Util.hpp>

namespace tge::nif {

	using namespace tge::graphics;
	using namespace tge::main;
	using namespace tge::shader;

	Error NifModule::init()
	{
		vertexFile = util::wholeFile("assets/testNif.vert");
		fragmentsFile = util::wholeFile("assets/testNif.frag");
		return Error::NONE;
	}

	size_t NifModule::load(const std::string& name, void* shaderPipe) const {
		const auto api = getAPILayer();
		const auto ggm = getGameGraphicsModule();
		const auto sha = api->getShaderAPI();
		nifly::NifFile file(name);
		const auto& shapes = file.GetShapes();

		std::vector<RenderInfo> renderInfos;
		renderInfos.resize(shapes.size());

		const auto pipe = sha->compile({ {ShaderType::VERTEX, vertexFile}, {ShaderType::FRAGMENT, fragmentsFile} });
		Material material(pipe);
		const auto materialId = api->pushMaterials(1, &material);

		std::vector<const void*> dataPointer;
		std::vector<size_t> sizes;
		size_t current = 0;
		dataPointer.reserve(shapes.size() * 5);
		sizes.reserve(shapes.size() * 5);
		std::vector<std::vector<std::Triangle>> triangleLists;
		triangleLists.resize(shapes.size());
		std::vector<std::string> textureNames;
		textureNames.reserve(shapes.size());
		for (auto shape : shapes) {
			nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
			if (!bishape) {
				printf("[WARN]: No BSTriShape!\n");
				continue;
			}
			auto& info = renderInfos[current];
			info.vertexBuffer.push_back(dataPointer.size());
			
			const auto& verticies = bishape->UpdateRawVertices();
			dataPointer.push_back(verticies.data());
			sizes.push_back(verticies.size() * sizeof(nifly::Vector3));

			auto & triangles = triangleLists[current];
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
			const auto textures = file.GetTexturePathRefs(shape);
			for (const auto texture : textures) {
				const auto& tex = texture.get();
				if(!tex.empty())
					textureNames.push_back(tex);
			}
			current++;
		}
		const auto texturesLoaded = ggm->loadTextures(textureNames, tge::graphics::LoadType::DDSPP);
		const auto indexBufferID = api->pushData(dataPointer.size(), dataPointer.data(), sizes.data(), DataType::VertexIndexData);

		std::vector<tge::graphics::NodeInfo> nodeInfos;
		nodeInfos.resize(shapes.size() + 1);
		const auto startID = sha->createBindings(pipe, shapes.size());
		current = 0;
		for (const auto shape : shapes) {
			auto& info = renderInfos[current];
			info.materialId = materialId;
			info.bindingID = startID + current;
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