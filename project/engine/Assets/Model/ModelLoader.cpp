#include "ModelLoader.h"
#include "engine/Math/Types/Transform.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Runtime/GraphicsCore.h"
#include "ModelSaver.h"

#include "engine/Utilities/FileUtilities.h"

namespace NoEngine {

using namespace Component;
using namespace Asset;

ModelAsset ModelLoader::Load(const std::string& filePath) {
	ModelAsset modelAsset;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	if (scene == nullptr) return modelAsset;
	assert(scene->HasMeshes());
	std::string directoryPath = Utilities::GetBasePath(filePath);

	Mesh newMesh;
	std::vector<Material> newMaterial;
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));
		auto vertexCount = mesh->mNumVertices;
		auto indexCount = mesh->mNumFaces * 3;



		uint32_t vertexBase = static_cast<uint32_t>(newMesh.vertices.size());
		uint32_t indexBase = static_cast<uint32_t>(newMesh.indices.size());

		newMesh.vertices.reserve(vertexBase + vertexCount);
		newMesh.indices.reserve(indexBase + indexCount);

		// Vertex読み込み
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];

			Vertex vertex;
			vertex.position = { -position.x, position.y, position.z };
			vertex.texcoord = { texcoord.x, texcoord.y };
			vertex.normal = { normal.x, normal.y, normal.z };
			newMesh.vertices.push_back(vertex);
		}

		// Index読み込み
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				newMesh.indices.push_back(vertexIndex);
			}
		}

		if (mesh->HasBones()) {

			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
				aiBone* bone = mesh->mBones[boneIndex];
				std::string jointName = bone->mName.C_Str();
				JointWeightData& jointWeightData = newMesh.skinClusterData[jointName];

				aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
				aiVector3D scale, translate;
				aiQuaternion rotate;
				bindPoseMatrixAssimp.Decompose(scale, rotate, translate);

				jointWeightData.inverseBindPoseMatrix.MakeAffine({ scale.x,scale.y,scale.z }, { rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
				jointWeightData.inverseBindPoseMatrix.Inverse();

				for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
					uint32_t localId = bone->mWeights[weightIndex].mVertexId;
					float weight = bone->mWeights[weightIndex].mWeight;
					// ローカル頂点IDをグローバル頂点IDへ変換
					uint32_t globalId = vertexBase + localId;
					jointWeightData.vertexWeights.push_back({ weight, globalId });
				}
			}
		}

		SubMesh sub{};
		sub.indexStart = indexBase;
		sub.indexCount = indexCount;
		sub.vertexStart = vertexBase;
		sub.vertexCount = vertexCount;


		if (scene->HasMaterials()) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString texPath;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
				Material m{};

				m.textureHandle = NoEngine::TextureManager::LoadCovertTexture(directoryPath + "/" + texPath.C_Str());
				sub.materialIndex = static_cast<uint32_t>(newMaterial.size());
				newMaterial.push_back(m);
			} else {
				// ToDo : テクスチャがないとwhite1x1.pngをテクスチャとして利用するようになっていますが、
				// テクスチャのないマテリアルはテクスチャがない状態で管理できるようにすべきです。
				Material m{};
				m.textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
				sub.materialIndex = static_cast<uint32_t>(newMaterial.size());
				newMaterial.push_back(m);
			}
		}


		newMesh.subMeshes.push_back(sub);
	}

	newMesh.rootNode = ReadNode(scene->mRootNode);

	if (scene->HasAnimations()) {
		modelAsset.animations = ProcessAnimation(scene);
		modelAsset.skeleton = ProcessSkeleton(newMesh.rootNode, newMesh);
	}

	// Vertex
	{
		if (newMesh.skinClusterData.empty()) {
			size_t vertexBufferSize = newMesh.vertices.size() * sizeof(Vertex);

			UploadBuffer vertexUpload;
			vertexUpload.Create(L"VertexUpload", vertexBufferSize);
			memcpy(vertexUpload.Map(), newMesh.vertices.data(), vertexBufferSize);
			vertexUpload.Unmap();

			newMesh.useVertexBuffer.Create(
				L"Model vertex",
				static_cast<uint32_t>(newMesh.vertices.size()),
				sizeof(Vertex),
				vertexUpload
			);
		} else {
			std::vector<JointVertex> sortedJointData(newMesh.vertices.size());

			auto& skeleton = *ModelSaver::Get().GetSkeleton(modelAsset.skeleton);
			for (const auto& [jointName, skinInfo] : newMesh.skinClusterData) {
				auto it = skeleton.jointMap.find(jointName);
				if (it == skeleton.jointMap.end()) continue;

				int32_t jointIndex = it->second;

				for (const auto& weightInfo : skinInfo.vertexWeights) {
					uint32_t vertexIdx = weightInfo.vertexIndex;
					float weightVal = weightInfo.weight;

					if (vertexIdx < sortedJointData.size()) {
						// 既存のjointIndices/weights配列の空きスロットを探して代入
						for (int k = 0; k < 4; ++k) {
							if (sortedJointData[vertexIdx].weights[k] == 0.0f) {
								sortedJointData[vertexIdx].jointIndices[k] = jointIndex;
								sortedJointData[vertexIdx].weights[k] = weightVal;
								break;
							}
						}
					}
				}
			}

			size_t vertexSize = sizeof(Vertex);
			size_t skinSize = sizeof(JointVertex);
			size_t totalSize = newMesh.vertices.size() * (vertexSize + skinSize);

			UploadBuffer vertexUpload;
			vertexUpload.Create(L"VertexUpload", totalSize);

			uint8_t* dst = reinterpret_cast<uint8_t*>(vertexUpload.Map());
			for (size_t i = 0; i < newMesh.vertices.size(); ++i) {
				memcpy(dst, &newMesh.vertices[i], vertexSize);
				dst += vertexSize;

				memcpy(dst, &sortedJointData[i], skinSize);
				dst += skinSize;
			}

			vertexUpload.Unmap();

			newMesh.baseVertexBuffer.Create(
				L"Skin model vertex",
				static_cast<uint32_t>(newMesh.vertices.size()),
				static_cast<uint32_t>(vertexSize + skinSize),
				vertexUpload
			);


			size_t vertexBufferSize = newMesh.vertices.size() * sizeof(Vertex);
			UploadBuffer useVertexUpload;
			useVertexUpload.Create(L"VertexUpload", vertexBufferSize);
			memcpy(useVertexUpload.Map(), newMesh.vertices.data(), vertexBufferSize);
			useVertexUpload.Unmap();

			newMesh.useVertexBuffer.Create(
				L"Skinned model vertex",
				static_cast<uint32_t>(newMesh.vertices.size()),
				static_cast<uint32_t>(vertexSize),
				useVertexUpload
			);
		}

	}

	// Index
	{
		size_t indexBufferSize = newMesh.indices.size() * sizeof(uint32_t);

		UploadBuffer indexUpload;
		indexUpload.Create(L"IndexUpload", indexBufferSize);
		memcpy(indexUpload.Map(), newMesh.indices.data(), indexBufferSize);
		indexUpload.Unmap();

		newMesh.indexBuffer.Create(
			L"Model index",
			static_cast<uint32_t>(newMesh.indices.size()),
			sizeof(uint32_t),
			indexUpload
		);
	}

	// RaytracingMesh
	if (GraphicsCore::IsEnableRaytracing()) {
		LogDebug("RaytracingMesh create start");
		newMesh.raytracingMesh = std::make_unique<RaytracingMesh>(ProcessRaytracingMesh(newMesh));
	}

	
	ModelSaver& modelSaver = ModelSaver::Get();
	modelAsset.mesh = modelSaver.AddMesh(std::move(newMesh));
	for (auto& m : newMaterial) {
		modelAsset.materials.push_back(modelSaver.AddMaterial(std::move(m)));
	}

	return modelAsset;
}



Node ModelLoader::ReadNode(aiNode* node) {
	Node result;
	Transform transform;
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate); // assimpの行列からSRTを抽出する間数を利用します。
	transform.scale = { scale.x, scale.y, scale.z }; // Scaleはそのまま
	transform.rotation = { rotate.x, -rotate.y, -rotate.z, rotate.w };// x軸を反転、さらに回転方向が逆なので軸を反転させます。
	transform.translate = { -translate.x, translate.y, translate.z };// x軸を反転
	result.localMatrix = transform.MakeAffineMatrix4x4();
	result.name = node->mName.C_Str(); // Node名を格納
	result.children.resize(node->mNumChildren); // 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に読んで階層構造を作っていきます。
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

std::vector<AnimationHandle> ModelLoader::ProcessAnimation(const aiScene* scene) {
	std::vector<Animation> animations;
	std::vector<AnimationHandle> result;
	animations.resize(scene->mNumAnimations);
	result.resize(scene->mNumAnimations);
	for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* animationAssimp = scene->mAnimations[i]; // 最初のアニメーションだけ採用
		Animation& animation = animations[i];
		animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間単位を秒へ変換

		// assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報をとってくる
		for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
			aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
			NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
				KeyframeVector3 keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
				keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };//右手->左手
				nodeAnimation.translate.keyframes.push_back(keyframe);
			}

			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
				aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
				KeyframeQuaternion keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
				keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };// 右手->左手
				nodeAnimation.rotation.keyframes.push_back(keyframe);
			}

			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
				KeyframeVector3 keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
				keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
				nodeAnimation.scale.keyframes.push_back(keyframe);
			}
		}

		result[i] = ModelSaver::Get().AddAnimation(std::move(animation));
	}
	return result;
}


SkeletonHandle ModelLoader::ProcessSkeleton(const Node& rootNode,Mesh& mesh) {
	Skeleton skeleton{};
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	//名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	// スキニング
	{
		mesh.numJoints = static_cast<uint32_t>(skeleton.joints.size());
		mesh.mappedPalette.resize(skeleton.joints.size());
		mesh.paletteUpload.Create(L"SkinningUpload", sizeof(SkeletonWell) * mesh.mappedPalette.size());
		memcpy(mesh.paletteUpload.Map(), mesh.mappedPalette.data(), sizeof(SkeletonWell) * mesh.mappedPalette.size());

		mesh.paletteResource.Create(
			L"Skinning Joints",
			static_cast<uint32_t>(mesh.mappedPalette.size()),
			sizeof(SkeletonWell),
			mesh.paletteUpload
		);


		skeleton.inverseBindPoseMatrices.assign(skeleton.joints.size(), Math::Matrix4x4::IDENTITY);

		for (const auto& jointWeight : mesh.skinClusterData) {
			auto it = skeleton.jointMap.find(jointWeight.first);
			if (it == skeleton.jointMap.end()) continue;

			skeleton.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		}
	}
	return ModelSaver::Get().AddSkeleton(std::move(skeleton));
}

RaytracingMesh ModelLoader::ProcessRaytracingMesh(const Mesh& mesh) {
	RaytracingMesh rtMesh;
	rtMesh.geometries.reserve(mesh.subMeshes.size());

	for (const SubMesh& sm : mesh.subMeshes) {
		rtMesh.geometries.push_back(MakeGeometryDesc(mesh, sm));
	}

	// BLAS inputs
	rtMesh.inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	rtMesh.inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	rtMesh.inputs.NumDescs = (UINT)rtMesh.geometries.size();
	rtMesh.inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	rtMesh.inputs.pGeometryDescs = &rtMesh.geometries[0].geometryDesc;

	// PrebuildInfoの取得
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
	GraphicsCore::sGraphicsDevice->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(
		&rtMesh.inputs,
		&prebuildInfo
	);

	// BLAS用のGPUリソース作成
	auto resultDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuildInfo.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	auto scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuildInfo.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	auto updateScratchDesc = CD3DX12_RESOURCE_DESC::Buffer(
		prebuildInfo.UpdateScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_HEAP_PROPERTIES defaultHeapProps{};
	defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProps.CreationNodeMask = 1;
	defaultHeapProps.VisibleNodeMask = 1;

	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&resultDesc,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr,
		IID_PPV_ARGS(&rtMesh.blas)
	);

	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&scratchDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&rtMesh.scratch)
	);

	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&updateScratchDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&rtMesh.updateScratch)
	);

	// BLASのビルドコマンド発行
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
	buildDesc.Inputs = rtMesh.inputs;
	buildDesc.ScratchAccelerationStructureData = rtMesh.scratch->GetGPUVirtualAddress();
	buildDesc.DestAccelerationStructureData = rtMesh.blas->GetGPUVirtualAddress();

	CommandContext::BuildRaytracingAccelerationStructure(buildDesc, rtMesh.scratch.Get());

	return rtMesh;
}

int32_t ModelLoader::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = { Math::Matrix4x4::IDENTITY };
	joint.transform = node.transform;
	joint.index = int32_t(joints.size()); // 現在登録されてる数をIndexに
	joint.parent = parent;
	joints.push_back(joint); // SkeletonのJoint列に追加します。
	for (const Node& child : node.children) {
		// 子Jointを作成し、そのIndexを登録します。
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	// 自身のIndexを返します。
	return joint.index;
}

RaytracingGeometry ModelLoader::MakeGeometryDesc(const Mesh& mesh, const SubMesh& sm) {
	D3D12_RAYTRACING_GEOMETRY_DESC geom = {};
	geom.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geom.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	// Index
	geom.Triangles.IndexBuffer =
		mesh.indexBuffer.GetGpuVirtualAddress() +
		sizeof(uint32_t) * sm.indexStart;

	geom.Triangles.IndexCount = sm.indexCount;
	geom.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

	// Vertex
	geom.Triangles.VertexBuffer.StartAddress =
		mesh.useVertexBuffer.GetGpuVirtualAddress() +
		sizeof(Vertex) * sm.vertexStart;

	geom.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
	geom.Triangles.VertexCount = sm.vertexCount;

	geom.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

	RaytracingGeometry result;
	result.geometryDesc = geom;

	return result;
}

}
