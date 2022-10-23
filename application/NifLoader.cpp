#include "NifLoader.hpp"
#include <NifFile.hpp>
#include <TGEngine.hpp>
#include <Util.hpp>
#include <graphics/vulkan/VulkanShaderPipe.hpp>

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

		std::vector<const void*> dataPointer;
		std::vector<size_t> sizes;
		size_t current = 0;
		dataPointer.reserve(shapes.size() * 5);
		sizes.reserve(shapes.size() * 5);
		std::vector<std::vector<std::Triangle>> triangleLists;
		triangleLists.resize(shapes.size());

		std::vector<std::string> textureNames;
		std::unordered_map<std::string, size_t> textureNamesToID;
		textureNames.reserve(shapes.size());

		std::vector<Material> materials;
		materials.reserve(shapes.size());

		for (auto shape : shapes) {
			nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
			if (!bishape) {
				printf("[WARN]: No BSTriShape!\n");
				continue;
			}
			auto& info = renderInfos[current];

			const auto& verticies = bishape->UpdateRawVertices();
			info.vertexBuffer.push_back(dataPointer.size());
			dataPointer.push_back(verticies.data());
			sizes.push_back(verticies.size() * sizeof(nifly::Vector3));

			std::vector<std::string> cacheString;
			cacheString.reserve(10);
			if (bishape->HasUVs()) {
				cacheString.push_back("UV");
				const auto& uvData = bishape->UpdateRawUvs();
				info.vertexBuffer.push_back(dataPointer.size());
				dataPointer.push_back(uvData.data());
				sizes.push_back(uvData.size() * sizeof(nifly::Vector2));
			}
			if (bishape->HasNormals()) {
				cacheString.push_back("NORMAL");
				const auto& uvData = bishape->UpdateRawNormals();
				info.vertexBuffer.push_back(dataPointer.size());
				dataPointer.push_back(uvData.data());
				sizes.push_back(uvData.size() * sizeof(nifly::Vector3));
			}
			if (bishape->HasVertexColors()) {
				cacheString.push_back("COLOR");
				const auto& uvData = bishape->UpdateRawColors();
				info.vertexBuffer.push_back(dataPointer.size());
				dataPointer.push_back(uvData.data());
				sizes.push_back(uvData.size() * sizeof(nifly::Vector4));
			}

			auto foundItr = shaderCache.find(cacheString);
			if (foundItr == end(shaderCache)) {
				const auto pipe = sha->compile({ {ShaderType::VERTEX, vertexFile, cacheString}, {ShaderType::FRAGMENT, fragmentsFile, cacheString} });
				shaderCache[cacheString] = pipe;
				foundItr = shaderCache.find(cacheString);
				tge::shader::VulkanShaderPipe* ptr = (tge::shader::VulkanShaderPipe*)foundItr->second;
				ptr->vertexInputBindings.clear();
				ptr->vertexInputBindings.resize(cacheString.size() + 1);
				ptr->vertexInputBindings[0] = vk::VertexInputBindingDescription(0, 12);
				for (auto& attribute : ptr->vertexInputAttributes) {
					attribute.binding = attribute.location;
					attribute.offset = 0;
					ptr->vertexInputBindings[attribute.location] = vk::VertexInputBindingDescription(attribute.binding, tge::shader::getSizeFromFormat(attribute.format));
				}
				ptr->inputStateCreateInfo.pVertexBindingDescriptions = ptr->vertexInputBindings.data();
				ptr->inputStateCreateInfo.vertexBindingDescriptionCount = ptr->vertexInputBindings.size();
			}
			void* ptr = foundItr->second;
			materials.push_back(Material(ptr));

			auto& triangles = triangleLists[current];
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
				if (!tex.empty())
					textureNames.push_back("assets\\" + tex);
			}
			current++;
		}
		const auto materialId = api->pushMaterials(materials.size(), materials.data());
		const auto texturesLoaded = ggm->loadTextures(textureNames, tge::graphics::LoadType::DDSPP);
		const auto indexBufferID = api->pushData(dataPointer.size(), dataPointer.data(), sizes.data(), DataType::VertexIndexData);
		
		SamplerInfo samplerInfo{ FilterSetting::LINEAR, FilterSetting::LINEAR, AddressMode::REPEAT, AddressMode::REPEAT };
		const auto samplerID = api->pushSampler(samplerInfo);

		size_t id = texturesLoaded;
		for (const auto& name : textureNames) {
			textureNamesToID[name] = id++;
		}

		std::vector<BindingInfo> bindingInfos;
		bindingInfos.reserve(shapes.size() * 3);
		std::vector<tge::graphics::NodeInfo> nodeInfos;
		nodeInfos.resize(shapes.size() + 1);
		current = 0;
		for (const auto shape : shapes) {
			auto& info = renderInfos[current];
			info.materialId = materialId + current;
			info.bindingID = sha->createBindings(materials[current].costumShaderData, 1);
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

			nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
			if (bishape) {
				auto shaderData = file.GetShader(shape);
				if (shaderData) {
					const auto indexTexData = shaderData->TextureSetRef();
					const auto ref = file.GetHeader().GetBlock(indexTexData);
					const auto& base = ref->textures[0].get();
					const auto& normal = ref->textures[1].get();
					const auto albedoID = textureNamesToID["assets\\" + base];
					const auto normalID = textureNamesToID["assets\\" + normal];
					const BindingInfo samplerBinding{
						0,
						nodeInfo.bindingID,
						BindingType::Sampler,
						{ UINT64_MAX, samplerID }
					};
					const BindingInfo albedoBinding{
						1,
						nodeInfo.bindingID,
						BindingType::Texture,
						{ albedoID, samplerID }
					};
					const BindingInfo normalBinding{
						4,
						nodeInfo.bindingID,
						BindingType::Texture,
						{ normalID, samplerID }
					};
					bindingInfos.push_back(albedoBinding);
					bindingInfos.push_back(normalBinding);
					bindingInfos.push_back(samplerBinding);
				}
			}
		}

		const auto nodes = ggm->addNode(nodeInfos.data(), nodeInfos.size());

		sha->bindData(bindingInfos.data(), bindingInfos.size());

		api->pushRender(renderInfos.size(), renderInfos.data());

		return nodes;
	}
}