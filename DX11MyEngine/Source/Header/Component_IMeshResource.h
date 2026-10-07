#pragma once
#include "IComponent.h"
#include "MeshInfoFactory.h"

//* =========================================================================
//* - @:UTILITY_MESH_TYPE列挙体 - */
//* 【?】自作メッシュの種類
//* =========================================================================
enum class UTILITY_MESH_TYPE
{
	NONE = 0,
	CUBE,
	QUAD,
	SPHERE,
	PLANE,
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:IMeshResource Class --- */
//
//  ★継承：IComponent ★
//
// 【?】メッシュ構造（データ）の管理
//		頂点情報など
//	※　UnityのMeshFilter的な役割
//
// ***************************************************************************************
class IMeshResource : public IComponent
{
private:
	friend class MeshRenderer;
	friend class SkyRenderer;
	friend class DecalRenderer;
	friend class WaterRenderer;
	friend class MeshResourceEditor;

	std::shared_ptr<MeshResourceData> m_pMeshData; // メッシュ情報

	//CB_TRANSFORM_SET *m_pCBTransformSet;		   // 定数バッファ(ワールド行列用)
	//CB_MATERIAL_SET *m_pCBMaterialDataSet;		   // 定数バッファ(マテリアル用)
	SHADER_TYPE m_ShaderType;	

public:
	IMeshResource(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	virtual~IMeshResource();

	bool Setup(RendererEngine& renderer, SHADER_TYPE shaderType, UTILITY_MESH_TYPE type, std::shared_ptr<Material> materials, UINT materialNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale);	// 初期化


	/// <summary>
	/// テクスチャマップ設定 Setup後に呼ぶ
	/// </summary>
	/// <param name="mapType"></param>
	/// <param name="matIndex"></param>
	/// <param name="path"></param>
	/// <returns></returns>
	virtual bool set_TextureMap(TEXTURE_MAP mapType, UINT matIndex, const std::wstring& path);

	/// <summary>
	/// 定数バッファの作成
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <returns></returns>
	virtual bool CreateCBuffer(ID3D11Device* pDevice);

	UINT get_NumVertex()const { return m_pMeshData->NumVertex; };		// 頂点数取得
	UINT get_NumIndex()const { return m_pMeshData->NumIndex; };			// インデックス数取得
	UINT get_NumMaterial()const { return m_pMeshData->NumMaterial; };	// マテリアル数取得
	SHADER_TYPE get_ShaderType()const { return m_ShaderType; }			// 仕様シェーダ取得
	const MeshLocalBounds& get_LocalBounds() const { return m_pMeshData->LocalBounds; }	// ローカル境界データ取得

};

