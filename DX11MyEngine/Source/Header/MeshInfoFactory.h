#pragma once
// 頂点数 ***************************************
extern const UINT g_QuadVertexNum;
extern const UINT g_CubeVertexNum;
extern const UINT g_PlaneVertexNum;
extern const UINT g_SpriteQuadVertexNum;
extern const UINT g_RTSpriteQuadVertexNum;
// インデックス数 *******************************
extern const UINT g_QuadIndexNum;
extern const UINT g_CubeIndexNum;
extern const UINT g_PlaneIndexNum;
extern const UINT g_SpriteQuadIndexNum;
extern const UINT g_RTSpriteQuadIndexNum;

extern const VERTEX::VERTEX_Static g_QuadVertices[];
extern const VERTEX::VERTEX_Static g_CubeVertices[];
extern const VERTEX::VERTEX_Static g_PlaneVertices[];
extern const WORD g_QuadIndices[];
extern const WORD g_CubeIndices[];
extern const WORD g_PlaneIndices[];

/// <summary>
/// 静的メッシュ用
/// 法線マップなし
/// </summary>
//struct Static_MeshInfo
//{
//	TVertex *pVertices;					// 頂点情報
//	UINT NumVertex;						// 頂点数
//	WORD *pIndices;						// インデックス情報
//	UINT NumIndex;						// インデックス数
//	MATERIAL* pMaterials;				// マテリアル
//	UINT NumMaterial;					// マテリアル数
//};

struct MeshLocalBounds
{
	DirectX::BoundingBox Box{};
	DirectX::BoundingSphere Sphere{};
	bool IsValid = false;
};

/// <summary>
/// メッシュの情報（主にユーティリティメッシュ用）
/// </summary>
struct MeshResourceData 
{
	ID3D11Buffer* pVertexBuffer;		// 頂点バッファ
	UINT VertexStride;					// 頂点一つのサイズ（sizeof(VERTEX_Static)など）
	UINT NumVertex;						// 頂点数
	ID3D11Buffer* pIndexBuffer;			// インデックスバッファ
	UINT NumIndex;						// インデックス数
	MeshLocalBounds LocalBounds;		// ローカル境界
	std::weak_ptr<Material> pMaterials;	// マテリアル（基本一つだけ）
	UINT NumMaterial;					// マテリアル数
	bool IsDynamic;						// 動的メッシュか（バッファの切り替え）

	MeshResourceData() :
		pVertexBuffer(nullptr),
		VertexStride(0),
		NumVertex(0),
		pIndexBuffer(nullptr),
		NumIndex(0),
		pMaterials(),
		NumMaterial(0),
		IsDynamic(false)
	{};
};

class MeshInfoFactory
{
public:
	MeshInfoFactory();
	~MeshInfoFactory();

	template<typename TVertex>
	static MeshResourceData CreateMesh(ID3D11Device* pDevice,TVertex* pVertices, const UINT vNum,const WORD* pIndices, UINT iNum, bool isDynamic = false)
	{
		MeshResourceData meshData = {};
		meshData.VertexStride = sizeof(TVertex); // ここで型のサイズを固定値にする
		meshData.NumVertex = vNum;
		meshData.NumIndex = iNum;
		meshData.IsDynamic = isDynamic;
		
		/////////////////////////////////////////////////////////////////////
		// ローカル境界の生成		
		/////////////////////////////////////////////////////////////////////
		if (pVertices && vNum > 0)
		{
			VECTOR3::VEC3 minPos = pVertices[0].pos;
			VECTOR3::VEC3 maxPos = pVertices[0].pos;

			for (UINT i = 1; i < vNum; i++)
			{
				const VECTOR3::VEC3& pos = pVertices[i].pos;

				minPos.x = std::min(minPos.x, pos.x);
				minPos.y = std::min(minPos.y, pos.y);
				minPos.z = std::min(minPos.z, pos.z);

				maxPos.x = std::max(maxPos.x, pos.x);
				maxPos.y = std::max(maxPos.y, pos.y);
				maxPos.z = std::max(maxPos.z, pos.z);
			}

			DirectX::XMFLOAT3 center{
				(minPos.x + maxPos.x) * 0.5f,
				(minPos.y + maxPos.y) * 0.5f,
				(minPos.z + maxPos.z) * 0.5f,
			};


			DirectX::XMFLOAT3 extents{
				(maxPos.x - minPos.x) * 0.5f,
				(maxPos.y - minPos.y) * 0.5f,
				(maxPos.z - minPos.z) * 0.5f,
			};

			// ローカル境界設定
			meshData.LocalBounds.Box =
				DirectX::BoundingBox(center, extents);

			// AABB全体を覆うSphere
			const float radius = std::sqrt(
				extents.x * extents.x +
				extents.y * extents.y +
				extents.z * extents.z
			);
			meshData.LocalBounds.Sphere =
				DirectX::BoundingSphere(center, radius);

			meshData.LocalBounds.IsValid = true;
		}


		/////////////////////////////////////////////////////////////////////
		// 頂点バッファの作成
		/////////////////////////////////////////////////////////////////////
		D3D11_BUFFER_DESC bd{};
		if (isDynamic) {
			bd.Usage = D3D11_USAGE_DYNAMIC;					// 動的バッファ
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// CPU書き込み
		}
		else {
			bd.Usage = D3D11_USAGE_DEFAULT;					// 標準設定
			bd.CPUAccessFlags = 0;							// CPUから書き込みしない
		}
		bd.ByteWidth = meshData.VertexStride * vNum;		// バッファのサイズ
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// 頂点バッファとして使う

		// 頂点バッファのデータ初期化構造体
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = pVertices;

		// 頂点バッファの生成
		HRESULT hr = pDevice->CreateBuffer(&bd, &initData, &meshData.pVertexBuffer);
		if (FAILED(hr)) {
			MessageBoxA(NULL, "頂点バッファの作成に失敗しました。", "Error", MB_OK);
			return meshData;
		}

		/////////////////////////////////////////////////////////////////////
		// インデックスバッファの作成
		/////////////////////////////////////////////////////////////////////
		bd.Usage = D3D11_USAGE_DEFAULT;						// 標準設定
		bd.ByteWidth = sizeof(WORD) * iNum;					// バッファのサイズ
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;				// インデックスバッファとして使う
		bd.CPUAccessFlags = 0;								// CPUから書き込みしない

		// インデックスバッファのデータ初期化構造体
		initData.pSysMem = pIndices;

		// インデックスバッファの生成
		hr = pDevice->CreateBuffer(&bd, &initData, &meshData.pIndexBuffer);
		if (FAILED(hr)) {
			// インデックスバッファに失敗したら、作った頂点バッファを消す
			if (meshData.pVertexBuffer) {
				meshData.pVertexBuffer->Release();
				meshData.pVertexBuffer = nullptr;
			}
			MessageBoxA(NULL, "インデックスバッファの作成に失敗しました。", "Error", MB_OK);
			return meshData;
		}

		return meshData;
	}

	static std::shared_ptr<MeshResourceData> CreateQuadInfo(RendererEngine& renderer,	std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale);
	static std::shared_ptr<MeshResourceData> CreateCubeInfo(RendererEngine& renderer,	std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale);
	static std::shared_ptr<MeshResourceData> CreateSphereInfo(RendererEngine& renderer,	std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale);
	static std::shared_ptr<MeshResourceData> CreatePlaneInfo(RendererEngine& renderer,	std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2 & _tilingScale);
	//static std::shared_ptr<MeshResourceData> CreateTriangleInfo(RendererEngine& renderer);
	//static std::shared_ptr<MeshResourceData> CreateCylinderInfo(RendererEngine& renderer);
	static std::shared_ptr<MeshResourceData> CreateSpriteQuadInfo(RendererEngine& renderer, float w, float h);
	static std::shared_ptr<MeshResourceData> CreateRTSpriteInfo(RendererEngine& renderer, float w, float h);
};
