#include "ModelLoader.h"
#include "engine/Math/Types/Transform.h"

namespace NoEngine {
namespace Asset {
namespace {
std::unordered_map<std::string, Mesh> sMeshes;
}


Mesh& ModelLoader::LoadModel(const std::string& name, const std::string& filePath) {
	if (sMeshes.contains(name)) {
		return sMeshes[name];
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				// ToDo : ライトがないため、ノーマル読み込み処理がありません。ライトを実装し次第早急に読み込み処理を追加すべきです。
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				Vertex vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.texcoord = { texcoord.x, texcoord.y };
				// aiProcess_MakeLeft Handedはz*=-1で、右手->左手に変換するので手動で対処
				vertex.position.x *= -1.0f;
				sMeshes[name].vertices.push_back(vertex);
			}
		}
	}

	sMeshes[name].rootNode =  ReadNode(scene->mRootNode);

	// ToDo : Material読み込みもできるようにすべきです。

	return sMeshes[name];
}

Node ModelLoader::ReadNode(aiNode* node) {
	Node result;
	Transform transform;
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate); // assimpの行列からSRTを抽出する間数を利用
	transform.scale = { scale.x, scale.y, scale.z }; // Scaleはそのまま
	transform.rotation = { rotate.x, -rotate.y, -rotate.z, rotate.w };//x軸を反転、さらに回転方向が逆なので軸を反転させる
	transform.translate = { -translate.x, translate.y, translate.z };//x軸を反転
	result.localMatrix = transform.MakeAffineMatrix4x4();
	result.name = node->mName.C_Str(); // Node名を格納
	result.children.resize(node->mNumChildren); // 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

}
}