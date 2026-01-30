#include "ModelLoader.h"
#include "engine/Math/Types/Transform.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"

#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Utilities/FileUtilities.h"

namespace NoEngine {

using namespace Component;

// ToDo : 現在は読み込んだモデル、アニメーション、マテリアルをここで保存していますが、別クラスで保存すべきです。
namespace {
std::unordered_map<std::string, Mesh> sMeshes;
std::unordered_map<std::string, std::vector<Animation>> sAnimation;
std::unordered_map<std::string, Skeleton> sSkeletons;
std::unordered_map<std::string, std::vector<Material>> sMaterials;
}

void ModelLoader::LoadModel(const std::string& name, const std::string& filePath, MeshComponent* model, AnimatorComponent* animator) {
	if (sMeshes.contains(name)) {
		if (model) model->mesh = &sMeshes[name];
		if (animator) {
			if (sAnimation.contains(name))animator->animation = sAnimation[name];
			if (sSkeletons.contains(name))animator->skeleton = &sSkeletons[name];
		}
		return;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());
	std::string directoryPath = Utilities::GetBasePath(filePath);

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));
		auto vertexCount = mesh->mNumVertices;
		auto indexCount = mesh->mNumFaces * 3;



		uint32_t vertexBase = static_cast<uint32_t>(sMeshes[name].vertices.size());
		uint32_t indexBase = static_cast<uint32_t>(sMeshes[name].indices.size());

		sMeshes[name].vertices.reserve(vertexBase + vertexCount);
		sMeshes[name].indices.reserve(indexBase + indexCount);

		// Vertex読み込み
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];

			Vertex vertex;
			vertex.position = { -position.x, position.y, position.z, 1.0f };
			vertex.texcoord = { texcoord.x, texcoord.y };
			vertex.normal = { normal.x, normal.y, normal.z };
			sMeshes[name].vertices.push_back(vertex);
		}

		// Index読み込み
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				sMeshes[name].indices.push_back(vertexIndex);
			}
		}

		if (mesh->HasBones()) {
			
			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
				aiBone* bone = mesh->mBones[boneIndex];
				std::string jointName = bone->mName.C_Str();
				JointWeightData& jointWeightData = sMeshes[name].skinClusterData[jointName];

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

				// ToDo : マテリアル読み込みが簡易的すぎます。同じ種類のマテリアルなら読み込まないようにすべきです。
				m.textureHandle = NoEngine::TextureManager::LoadCovertTexture(directoryPath + "/" + texPath.C_Str());
				sub.materialIndex = static_cast<uint32_t>(sMaterials[name].size());
				sMaterials[name].push_back(m);
			} else {
				// ToDo : テクスチャがないとwhite1x1.pngをテクスチャとして利用するようになっていますが、
				// テクスチャのないマテリアルはテクスチャがない状態で管理できるようにすべきです。
				Material m{};
				m.textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
				sub.materialIndex = static_cast<uint32_t>(sMaterials[name].size());
				sMaterials[name].push_back(m);
			}
		}

		sMeshes[name].subMeshes.push_back(sub);
	}
	
	sMeshes[name].rootNode = ReadNode(scene->mRootNode);

	if (scene->HasAnimations()) {
		ProcessAnimation(name, scene);
		ProcessSkeleton(name, sMeshes[name].rootNode);
	}
	if (animator) {
		if (sAnimation.contains(name))animator->animation = sAnimation[name];
		if (sSkeletons.contains(name))animator->skeleton = &sSkeletons[name];
	}

	// Vertex
	{
		if (sMeshes[name].skinClusterData.empty()) {
			size_t vertexBufferSize = sMeshes[name].vertices.size() * sizeof(Vertex);

			UploadBuffer vertexUpload;
			vertexUpload.Create(L"VertexUpload", vertexBufferSize);
			memcpy(vertexUpload.Map(), sMeshes[name].vertices.data(), vertexBufferSize);
			vertexUpload.Unmap();

			sMeshes[name].vertexBuffer.Create(
				L"Model vertex",
				static_cast<uint32_t>(sMeshes[name].vertices.size()),
				sizeof(Vertex),
				vertexUpload
			);
		} else {
			std::vector<JointVertex> sortedJointData(sMeshes[name].vertices.size());


			for (const auto& [jointName, skinInfo] : sMeshes[name].skinClusterData) {
				auto it = sSkeletons[name].jointMap.find(jointName);
				if (it == sSkeletons[name].jointMap.end()) continue;

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
			size_t totalSize = sMeshes[name].vertices.size() * (vertexSize + skinSize);

			UploadBuffer vertexUpload;
			vertexUpload.Create(L"VertexUpload", totalSize);

			uint8_t* dst = reinterpret_cast<uint8_t*>(vertexUpload.Map());
			for (size_t i = 0; i < sMeshes[name].vertices.size(); ++i) {
				memcpy(dst, &sMeshes[name].vertices[i], vertexSize);
				dst += vertexSize;

				memcpy(dst, &sortedJointData[i], skinSize);
				dst += skinSize;
			}

			vertexUpload.Unmap();

			sMeshes[name].vertexBuffer.Create(
				L"Skin model vertex",
				static_cast<uint32_t>(sMeshes[name].vertices.size()),
				static_cast<uint32_t>(vertexSize + skinSize),
				vertexUpload
			);
		}
		
	}

	// Index
	{
		size_t indexBufferSize = sMeshes[name].indices.size() * sizeof(uint32_t);

		UploadBuffer indexUpload;
		indexUpload.Create(L"IndexUpload", indexBufferSize);
		memcpy(indexUpload.Map(), sMeshes[name].indices.data(), indexBufferSize);
		indexUpload.Unmap();

		sMeshes[name].indexBuffer.Create(
			L"Model index",
			static_cast<uint32_t>(sMeshes[name].indices.size()),
			sizeof(uint32_t),
			indexUpload
		);
	}

	if (model) model->mesh = &sMeshes[name];
}

void ModelLoader::GetModel(const std::string& name, MeshComponent* model, AnimatorComponent* animator) {
	if (!model) return;

	if (sMeshes.contains(name)) {
		model->mesh = &sMeshes[name];
		if (animator) {
			if (sAnimation.contains(name))animator->animation = sAnimation[name];
			if (sSkeletons.contains(name))animator->skeleton = &sSkeletons[name];
		}
	}
}

std::span<Material> ModelLoader::GetMaterial(const std::string& name) {
	return sMaterials[name];
}

void ModelLoader::DeleteAll() {
	sMeshes.clear();
	sAnimation.clear();
	sSkeletons.clear();
	sMaterials.clear();
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

void ModelLoader::ProcessAnimation(const std::string& name, const aiScene* scene) {
	sAnimation[name].resize(scene->mNumAnimations);
	for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* animationAssimp = scene->mAnimations[i]; // 最初のアニメーションだけ採用
		Animation& animation = sAnimation[name][i];
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
	}
	
}

void ModelLoader::ProcessSkeleton(const std::string& name, const Node& rootNode) {
	Skeleton& skeleton = sSkeletons[name];
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	//名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}
	
	// スキニング
	{
		sMeshes[name].numJoints = static_cast<uint32_t>(skeleton.joints.size());
		sMeshes[name].mappedPalette.resize(skeleton.joints.size());
		sMeshes[name].paletteUpload.Create(L"SkinningUpload", sizeof(SkeletonWell) * sMeshes[name].mappedPalette.size());
		memcpy(sMeshes[name].paletteUpload.Map(), sMeshes[name].mappedPalette.data(), sizeof(SkeletonWell) * sMeshes[name].mappedPalette.size());

		sMeshes[name].paletteResource.Create(
			L"Skinning Joints",
			static_cast<uint32_t>(sMeshes[name].mappedPalette.size()),
			sizeof(SkeletonWell),
			sMeshes[name].paletteUpload
		);


		skeleton.inverseBindPoseMatrices.assign(skeleton.joints.size(), Matrix4x4::IDENTITY);

		for (const auto& jointWeight : sMeshes[name].skinClusterData) {
			auto it = skeleton.jointMap.find(jointWeight.first);
			if (it == skeleton.jointMap.end()) continue;

			skeleton.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		}
	}
	

}

int32_t ModelLoader::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = { Matrix4x4::IDENTITY };
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

}
