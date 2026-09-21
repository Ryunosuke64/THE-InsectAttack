#pragma once
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <queue>
#include "ModelMesh.h"

constexpr int MAX_REGISTER_LOD = 3; // LODモデル登録は3つまで

/// <summary>
/// LOD品質
/// </summary>
enum class LOD_QUALITY
{
	ULTRA = 0,	// 最高品質
	MIDDLE = 1,	// まあまあ品質
	LOW = 2		// 低品質
};


//* =========================================================================
//* 【?】ボーンに含まれる影響度まとめ
//* =========================================================================
struct VertexBoneWeightData
{
	unsigned int VertexID; // ボーンの影響を受ける頂点ID
	float Weight; // 重み (0.0～1.0)
};

//* =========================================================================
//* 【?】ボーン情報
//* =========================================================================
class BoneInfo {
public:
	std::string Name;				// ボーン名
	DirectX::XMMATRIX OffsetMtx;	// オフセット行列
	DirectX::XMFLOAT4X4  FinalTransformation;	// 最終行列
	std::vector<VertexBoneWeightData> WeightList;	// 影響を与える頂点リスト

	DirectX::XMMATRIX DebugWorldMatrix;	// ボーン表示のデバッグ用

	BoneInfo() :
		OffsetMtx(DirectX::XMMatrixIdentity()),
		DebugWorldMatrix(DirectX::XMMatrixIdentity()),
		FinalTransformation(DirectX::XMFLOAT4X4())
	{
	}
};

//* =========================================================================
//* 【?】ノード情報
//* =========================================================================
struct NodeInfo {
	std::string Name; // ノード名
	int ParentIndex;				  // 親ノードのインデックス (-1 = root)
	DirectX::XMMATRIX Transformation; // ローカル変換行列
	std::vector<int> ChildrenIndices; // 子ノードのインデックスリスト

	NodeInfo() :
		ParentIndex(-1),
		Transformation(DirectX::XMMatrixIdentity())
	{
	};
};

/*********** アニメーション用のキーフレーム構造体 ***********/
struct PositionKey { double Time; DirectX::XMFLOAT3 Value; };
struct RotationKey { double Time; aiQuaternion Value; };
struct ScalingKey { double Time; DirectX::XMFLOAT3 Value; };

//* =========================================================================
//* 【?】ノードに含まれるアニメーションのキーフレーム情報
//* =========================================================================
struct NodeAnimChannel {
	std::string NodeName; // ノード名
	std::vector<PositionKey> PositionKeys; // 座標キーフレーム
	std::vector<RotationKey> RotationKeys; // 回転キーフレーム
	std::vector<ScalingKey>	 ScalingKeys;  // スケールキーフレーム
};

//* =========================================================================
//* 【?】アニメーション情報
//* =========================================================================
struct AnimationData {
	std::string Name; // アニメーション名
	double Duration;  // アニメーションの長さ

	/* 1秒あたりのティック数
	* フレームレート的なやつ。なので高ければ高いほど滑らかに動く（多分）*/
	double TicksPerSecond;

	std::vector<NodeAnimChannel> Channels; // アニメーションチャネル

	AnimationData() :
		Duration(0.0),
		TicksPerSecond(0.0)
	{
	};
};



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ModelData Class --- */
//
// 【?】モデル情報の格納
//      ResourceManagerが実態を持ち、IMeshResourceやSkineedMeshAnimatorに情報を渡す
//
// ***************************************************************************************
class ModelData
{
private:
    uint32_t m_VertexNum;		// 頂点数
    uint32_t m_IndexNum;		// インデックス数
    uint32_t m_MeshNum;			// メッシュ数
    uint32_t m_BoneNum;			// ボーン数
    uint32_t m_AnimNum;			// アニメーション数
    uint32_t m_MaterialNum;		// マテリアル数

	SHADER_TYPE m_ShaderType;		// 使用するシェーダタイプ
	SHADER_TYPE m_ShadowShaderType; // シャドウ用シェーダタイプ

    Assimp::Importer m_Importer;			// インポーター
    const aiScene *m_pScene;				// シーン情報
	ModelMesh *m_pMeshes;					// メッシュ
    std::vector<BoneInfo> m_BoneList;		// ボーンリスト
    std::vector<std::weak_ptr<Material>> m_pMaterialList;	// マテリアルリスト

	DirectX::BoundingBox m_LocalBounds;		// モデルのローカル座標における境界
	bool m_HasLocalBounds;					// ローカル境界を保持しているか

	// ボーン変換用定数バッファ
	CB_BONES_DATA m_BonesData;						// 定数バッファ用ボーン変換データ

	std::vector<VERTEX::CollisionVertex> m_CollisionVertices;
	std::vector<uint32_t> m_CollisionIndices;


	//CB_BONES_DATA_SET *m_pConstanrBufferBonesData;// ボーン用
	//CB_TRANSFORM_SET  *m_pCBTransformSet;			// ワールド行列用
	//CB_MATERIAL_SET   *m_pCBMaterialDataSet;		// マテリアル用

	/***********アニメーション関連***********/
	std::vector<NodeInfo *> m_pNodeList;					// Nodeツリー情報
	std::unordered_map<std::string, uint32_t> m_BoneIndexMap; // Node名からBoneListのインデックスを引くためのマップ
	std::vector<AnimationData *>m_pAnimations;			 // 読み取ったアニメーション

public:
	ModelData();
	~ModelData();

	bool Setup(RendererEngine &render, const char *filePath);					// モデルデータのセットアップ
	bool SetupTextureMap(std::shared_ptr<Material> matData, int matIndex);		// テクスチャマップ設定 Setup後に呼ぶ
	void set_ShaderType(SHADER_TYPE type) { m_ShaderType = type; }				// シェーダタイプ設定
	void set_ShadowShaderType(SHADER_TYPE type) { m_ShadowShaderType = type; }	// シャドウ用シェーダタイプ設定


	// ****************************************************************************************************************************************
	/* アクセスメソッド */
	// ****************************************************************************************************************************************
	uint32_t get_VertexNum() const { return m_VertexNum; }
	uint32_t get_IndexNum() const { return m_IndexNum; }
	uint32_t get_MeshNum() const { return m_MeshNum; }
	uint32_t get_BoneNum() const { return m_BoneNum; }
	uint32_t get_AnimNum() const { return m_AnimNum; }
	uint32_t get_MaterialNum() const { return m_MaterialNum; }

	/* メッシュ・頂点データ */
	ModelMesh *get_Meshes() const { return m_pMeshes; }
	const DirectX::BoundingBox& get_LocalBounds() const { return m_LocalBounds; }
	bool get_HasLocalBounds() const { return m_HasLocalBounds; }


	/* ボーン・マテリアル */
	const std::vector<BoneInfo> &get_BoneList() const { return m_BoneList; }
	const std::vector<std::weak_ptr<Material>> &get_MaterialList() const { return m_pMaterialList; }


	/* コリジョンメッシュ  */
	const std::vector<VERTEX::CollisionVertex>& get_CollisionVertices() const { return m_CollisionVertices; }
	const std::vector<uint32_t>& get_CollisionIndices() const { return m_CollisionIndices; }

	/* Assimp関連 */
	const Assimp::Importer &get_Importer() const { return m_Importer; }
	const aiScene *get_Scene() const { return m_pScene; }


	/* アニメーション関連 */
	const std::vector<NodeInfo *> &get_NodeList() const { return m_pNodeList; }
	const std::unordered_map<std::string, uint32_t> &get_BoneIndexMap() const { return m_BoneIndexMap; }
	const std::vector<AnimationData *> &get_Animations() const { return m_pAnimations; }

	// シェーダタイプ取得
	const SHADER_TYPE &get_ShaderType() const { return m_ShaderType; }	
	const SHADER_TYPE &get_ShadowShaderType() const { return m_ShadowShaderType; }	

	/* 定数バッファ */
	const CB_BONES_DATA* get_BonesData() const { return &m_BonesData; }
	//CB_BONES_DATA_SET *GetConstantBufferBonesData() const { return m_pConstanrBufferBonesData; }
	//CB_TRANSFORM_SET *GetConstantBufferTransformSet() const { return m_pCBTransformSet; }
	//CB_MATERIAL_SET *GetConstantBufferMaterialDataSet() const { return m_pCBMaterialDataSet; }

private:
	void AnimationExtraction(const aiScene *scene);				// アニメーション情報抽出
	void NodeExtraction(const aiNode *pNode, int parentIdx);	// ノード情報を再帰的に抽出
	void BoneExtraction(aiMesh *mesh);							// ボーン情報抽出
	void MatrialExtraction(const aiScene *scene);				// マテリアル情報抽出 ※上のset_TextureMapで直接指定させているので要らないかも
	void CollisionMeshSetup(aiMesh* mesh);

	bool CreateBonesCBuffer(RendererEngine &renderer);			// ボーン変換用定数バッファの作成
	bool CreateMaterialCBuffer(RendererEngine &renderer);		// マテリアル用定数バッファの作成
	bool CreateTransformCBuffer(RendererEngine &renderer);		// トランスフォーム用定数バッファの作成
};

//* =========================================================================
//* 【?】LOD用のモデルデータ
//* =========================================================================
struct LODModelData
{
	float _distance = 0.0f;				  // LODの切り替え距離
	std::weak_ptr<ModelData> _pModelData; // モデルデータ
};