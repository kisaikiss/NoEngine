#include "ModelLoader.h"
#include "engine/Math/Types/Transform.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"

namespace NoEngine {
namespace {
std::unordered_map<std::string, Mesh> sMeshes;
}


Mesh* ModelLoader::LoadModel(const std::string& name, const std::string& filePath) {
	if (sMeshes.contains(name)) {
		return &sMeshes[name];
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

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
			// ToDo : ライト未実装のためノーマル読み込み部分がありません。ライト実装次第早急に読み込むべきです。

			Vertex vertex;
			vertex.position = { -position.x, position.y, position.z, 1.0f };
			vertex.texcoord = { texcoord.x, texcoord.y };
			sMeshes[name].vertices.push_back(vertex);
		}
		// Index読み込み
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				// 各メッシュの頂点配列を結合しているため、インデックスには現在の頂点ベースを加算する
				sMeshes[name].indices.push_back(vertexBase + vertexIndex);
			}

		}
	}

	sMeshes[name].rootNode =  ReadNode(scene->mRootNode);



	//vertex
	{
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
	}
	 
	//index
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

	// ToDo : Material読み込みもできるようにすべきです。

	return &sMeshes[name];
}

Mesh* ModelLoader::GetModel(const std::string& name) {
	return &sMeshes[name];
}

void ModelLoader::DeleteAll() {
	sMeshes.clear();
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

Animation ModelLoader::LoadAnimation(const std::string& filePath) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);

	aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初のアニメーションだけ採用
	Animation animation;
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
	return animation;
}

Skeleton ModelLoader::CreateSkeleton(const Node& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	//名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
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
