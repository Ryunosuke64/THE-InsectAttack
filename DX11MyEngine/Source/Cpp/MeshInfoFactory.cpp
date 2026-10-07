#include "pch.h"
#include "MeshInfoFactory.h"
#include "RendererEngine.h"
#include "Window.h"

using namespace VERTEX;
using namespace Tool::UV;
using namespace RenderData;

// 頂点数 ***************************************
const UINT g_QuadVertexNum		   = 4;
const UINT g_CubeVertexNum		   = 24;
const UINT g_PlaneVertexNum		   = 4;
const UINT g_SpriteQuadVertexNum   = 4;
const UINT g_RTSpriteQuadVertexNum = 4;
// インデックス数 *******************************
const UINT g_QuadIndexNum			= 6;
const UINT g_CubeIndexNum			= 36;
const UINT g_PlaneIndexNum			= 6;
const UINT g_SpriteQuadIndexNum		= 6;
const UINT g_RTSpriteQuadIndexNum	= 6;

 
//* =========================================================================
//* - @:クアッド（板ポリ） - */
//* =========================================================================
const VERTEX::VERTEX_Static g_QuadVertices[] = {
	{ VEC3(-1.0f,  1.0f,  0.0f), VEC2(0.0f, 0.0f),  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f) }, // 8 左上
	{ VEC3( 1.0f,  1.0f,  0.0f), VEC2(1.0f, 0.0f),  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f) }, // 9 右上
	{ VEC3(-1.0f, -1.0f,  0.0f), VEC2(0.0f, 1.0f),  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f) }, // 10左下
	{ VEC3( 1.0f, -1.0f,  0.0f), VEC2(1.0f, 1.0f),  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f) }, // 11右下
};
const WORD g_QuadIndices[] ={
	 0, 1, 2,
	 1, 3, 2
};

//* =========================================================================
//* - @:キューブ（箱） - */
//* =========================================================================
const VERTEX::VERTEX_Static g_CubeVertices[] = {
	// 座標                      //UV				// カラー 					// 法線						
	// 正面 1
	{ VEC3(-0.5f,  0.5f, -0.5f), VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)},  // 0 左上
	{ VEC3( 0.5f,  0.5f, -0.5f), VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)},  // 1 右上
	{ VEC3(-0.5f, -0.5f, -0.5f), VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)},  // 2 左下
	{ VEC3( 0.5f, -0.5f, -0.5f), VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)},  // 3 右下
	
	// 天井ポリゴン  3                   
	{ VEC3(-0.5f,  0.5f,  0.5f), VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 4 左上
	{ VEC3( 0.5f,  0.5f,  0.5f), VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 5 右上
	{ VEC3(-0.5f,  0.5f, -0.5f), VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 6 左下
	{ VEC3( 0.5f,  0.5f, -0.5f), VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 7 右下

	// 地面ポリゴン   4                                                                      
	{ VEC3( 0.5f,  -0.5f,  0.5f),VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f)}, // 8 左上
	{ VEC3(-0.5f,  -0.5f,  0.5f),VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f)}, // 9 右上
	{ VEC3( 0.5f,  -0.5f, -0.5f),VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f)}, // 10左下
	{ VEC3(-0.5f,  -0.5f, -0.5f),VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f)}, // 11右下

	// 裏ポリゴン 6
	{ VEC3( 0.5f,  0.5f,  0.5f), VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f) }, // 12 左上
	{ VEC3(-0.5f,  0.5f,  0.5f), VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f) }, // 13 右上
	{ VEC3( 0.5f, -0.5f,  0.5f), VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f) }, // 14 左下
	{ VEC3(-0.5f, -0.5f,  0.5f), VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f) }, // 15 右下

	// 右ポリゴン  2                                                                         
	{ VEC3(0.5f,   0.5f, -0.5f), VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f) }, // 16 左上
	{ VEC3(0.5f,   0.5f,  0.5f), VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f) }, // 17 右上
	{ VEC3(0.5f,  -0.5f, -0.5f), VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f) }, // 18 左下
	{ VEC3(0.5f,  -0.5f,  0.5f), VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f) }, // 19 右下

	// 左ポリゴン 5
	{ VEC3(-0.5f,   0.5f,  0.5f),VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f) },  // 20 左上
	{ VEC3(-0.5f,   0.5f, -0.5f),VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f) },  // 21 右上
	{ VEC3(-0.5f,  -0.5f,  0.5f),VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f) },  // 22 左下
	{ VEC3(-0.5f,  -0.5f, -0.5f),VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f) },  // 23 右下
};
const WORD g_CubeIndices[] = {
		0,1,2,
		1,3,2,  // 時計回りなら順番は何でもいい

		// 天井
		4,5,6,
		5,7,6,

		// 地面
		8,9,10,
		9,11,10,

		// 後ろ
		12,13,14,
		13,15,14,

		// 右
		16,17,18,
		17,19,18,

		// 左
		20,21,22,
		21,23,22,
};

//* =========================================================================
//* - @:プレーン（平面） - */
//* =========================================================================
const VERTEX::VERTEX_Static g_PlaneVertices[] = {
	// 座標   // uv  // カラー					   // 法線                          
	{VEC3(-1.0f, 0.0f,  1.0f), VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 8 左上
	{VEC3( 1.0f, 0.0f,  1.0f), VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 9 右上
	{VEC3(-1.0f, 0.0f, -1.0f), VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 10左下
	{VEC3( 1.0f, 0.0f, -1.0f), VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f)}, // 11右下
};
const WORD g_PlaneIndices[] = {
	 0, 1, 2,
	 1, 3, 2
};


MeshInfoFactory::MeshInfoFactory()
{
}

MeshInfoFactory::~MeshInfoFactory()
{
}

// ----------------------------------------------------------------------------------------------------------------------
//       * MeshFactory Class - 板ポリの作成- *
// 引数 1.マテリアル
// 		2.マテリアル数
// ----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MeshResourceData> MeshInfoFactory::CreateQuadInfo(RendererEngine& renderer, std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale)
{
	auto meshData = std::make_shared<MeshResourceData>();
	WORD indices[] = { 0, 1, 2, 1, 3, 2 };
	auto pDevice = renderer.get_Device();
	// 法線マップあり
	if (isNormalMap)
	{
		VERTEX::VERTEX_Static_N vertices[] = {
			// 座標						    // uv,			// カラー                   // 法線                  
			{ VEC3(-1.0f,  1.0f,  0.0f), VEC2(0.0f, 0.0f) * _tilingScale,  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC3(),VEC3() }, // 8 左上
			{ VEC3( 1.0f,  1.0f,  0.0f), VEC2(1.0f, 0.0f) * _tilingScale,  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC3(),VEC3() }, // 9 右上
			{ VEC3(-1.0f, -1.0f,  0.0f), VEC2(0.0f, 1.0f) * _tilingScale,  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC3(),VEC3() }, // 10左下
			{ VEC3( 1.0f, -1.0f,  0.0f), VEC2(1.0f, 1.0f) * _tilingScale,  VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC3(),VEC3() }, // 11右下
		};
		// 接線・副接線を求める
		Tool::CalcTangentAndBitangent(vertices, g_QuadVertexNum, g_QuadIndices, g_QuadIndexNum);

		*meshData = CreateMesh(pDevice,vertices, g_QuadVertexNum, g_QuadIndices, g_QuadIndexNum);
	}
	// 法線マップなし
	else
	{
		*meshData = CreateMesh(pDevice, g_QuadVertices, g_QuadVertexNum, g_QuadIndices, g_QuadIndexNum);
	}

	// マテリアル情報設定
	meshData->pMaterials = materials;

	// 両面描画
	if(!meshData->pMaterials.expired())
	{
		meshData->pMaterials.lock()->m_CullMode = CULL_MODE::NONE;
	}

	return meshData;

	{
		//// 頂点情報 （上向き）
		//meshInfo->pVertices = new BASE_VERTEX::VERTEX[meshInfo->NumVertex]{
		//	// 座標                       // 法線                  // カラー                      // uv
		//	{ VEC3(-1.0f, 0.0f,  1.0f),  VEC3(0.0f, 1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(0.0f, 0.0f)}, // 8 左上
		//	{ VEC3( 1.0f, 0.0f,  1.0f),  VEC3(0.0f, 1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(1.0f, 0.0f)}, // 9 右上
		//	{ VEC3(-1.0f, 0.0f, -1.0f),  VEC3(0.0f, 1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(0.0f, 1.0f)}, // 10左下
		//	{ VEC3( 1.0f, 0.0f, -1.0f),  VEC3(0.0f, 1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(1.0f, 1.0f)}, // 11右下
		//};	
		//
		//// 頂点情報 （前向き）
		//meshInfo->pVertices = new VERTEX::VERTEX_Static[meshInfo->NumVertex]{
		//	// 座標                       // 法線                  // カラー                      // uv
		//	{ VEC3(-1.0f,  1.0f,  0.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(0.0f, 0.0f)}, // 8 左上
		//	{ VEC3( 1.0f,  1.0f,  0.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(1.0f, 0.0f)}, // 9 右上
		//	{ VEC3(-1.0f, -1.0f,  0.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(0.0f, 1.0f)}, // 10左下
		//	{ VEC3( 1.0f, -1.0f,  0.0f),  VEC3(0.0f, 0.0f, -1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(1.0f, 1.0f)}, // 11右下
		//};

		//// インデックス数
		//meshInfo->NumIndex = 6;

		//// インデックス情報
		//meshInfo->pIndices = new WORD[meshInfo->NumIndex]{
		//	0,1,2,
		//	1,3,2
		//};
	}
}


// ----------------------------------------------------------------------------------------------------------------------
//       * MeshFactory Class - キューブの作成- *
// 引数 1.マテリアル
// 		2.マテリアル数
// ----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MeshResourceData> MeshInfoFactory::CreateCubeInfo(RendererEngine& renderer, std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale)
{
	auto pDevice = renderer.get_Device();
	auto meshData = std::make_shared<MeshResourceData>();
	//WORD indices[] = {	
	//	0,1,2,
	//	1,3,2,  // 時計回りなら順番は何でもいい

	//	// 天井
	//	4,5,6,
	//	5,7,6,

	//	// 地面
	//	8,9,10,
	//	9,11,10,

	//	// 後ろ
	//	12,13,14,
	//	13,15,14,

	//	// 右
	//	16,17,18,
	//	17,19,18,

	//	// 左
	//	20,21,22,
	//	21,23,22,
	//};

	VEC3 tan;
	VEC3 biTan;


	// 法線マップあり
	if (isNormalMap)
	{
		VERTEX::VERTEX_Static_N vertices[] = {
			// 座標                       // uv            // カラー					// 法線                
			// 頂点フォーマット
			// 正面 1
			{ VEC3(-1.0f,  1.0f, -1.0f), VEC2(0.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f), tan, biTan},  // 0 左上
			{ VEC3( 1.0f,  1.0f, -1.0f), VEC2(1.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f), tan, biTan},  // 1 右上
			{ VEC3(-1.0f, -1.0f, -1.0f), VEC2(0.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f), tan, biTan},  // 2 左下
			{ VEC3( 1.0f, -1.0f, -1.0f), VEC2(1.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f), tan, biTan},  // 3 右下
										 
			// 天井ポリゴン  3                   
			{ VEC3(-1.0f,  1.0f,  1.0f), VEC2(0.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), tan, biTan }, // 4 左上
			{ VEC3( 1.0f,  1.0f,  1.0f), VEC2(1.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), tan, biTan }, // 5 右上
			{ VEC3(-1.0f,  1.0f, -1.0f), VEC2(0.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), tan, biTan }, // 6 左下
			{ VEC3( 1.0f,  1.0f, -1.0f), VEC2(1.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), tan, biTan }, // 7 右下

			// 地面ポリゴン   4                                                                      
			{ VEC3( 1.0f,  -1.0f,  1.0f),VEC2(0.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f), tan, biTan }, // 8 左上
			{ VEC3(-1.0f,  -1.0f,  1.0f),VEC2(1.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f), tan, biTan }, // 9 右上
			{ VEC3( 1.0f,  -1.0f, -1.0f),VEC2(0.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f), tan, biTan }, // 10左下
			{ VEC3(-1.0f,  -1.0f, -1.0f),VEC2(1.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f) ,VEC3(0.0f, -1.0f, 0.0f), tan, biTan }, // 11右下

			// 裏ポリゴン *  6
			{ VEC3( 1.0f,  1.0f,  1.0f), VEC2(0.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f), tan, biTan }, // 12 左上
			{ VEC3(-1.0f,  1.0f,  1.0f), VEC2(1.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f), tan, biTan }, // 13 右上
			{ VEC3( 1.0f, -1.0f,  1.0f), VEC2(0.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f), tan, biTan }, // 14 左下
			{ VEC3(-1.0f, -1.0f,  1.0f), VEC2(1.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, 1.0f), tan, biTan }, // 15 右下

			// 右ポリゴン  2                                                                         
			{ VEC3(1.0f,   1.0f, -1.0f), VEC2(0.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f), tan, biTan }, // 16 左上
			{ VEC3(1.0f,   1.0f,  1.0f), VEC2(1.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f), tan, biTan }, // 17 右上
			{ VEC3(1.0f,  -1.0f, -1.0f), VEC2(0.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f), tan, biTan }, // 18 左下
			{ VEC3(1.0f,  -1.0f,  1.0f), VEC2(1.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(1.0f, 0.0f, 0.0f), tan, biTan }, // 19 右下

			// 左ポリゴン 5
			{ VEC3(-1.0f,   1.0f,  1.0f),VEC2(0.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f), tan, biTan },  // 20 左上
			{ VEC3(-1.0f,   1.0f, -1.0f),VEC2(1.0f, 0.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f), tan, biTan },  // 21 右上
			{ VEC3(-1.0f,  -1.0f,  1.0f),VEC2(0.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f), tan, biTan },  // 22 左下
			{ VEC3(-1.0f,  -1.0f, -1.0f),VEC2(1.0f, 1.0f) * _tilingScale, VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(-1.0f, 0.0f, 0.0f), tan, biTan },  // 23 右下
		};

		// 接線・副接線を求める
		Tool::CalcTangentAndBitangent(vertices, g_CubeVertexNum, g_CubeIndices, g_CubeIndexNum);

		*meshData = CreateMesh(pDevice, vertices, g_CubeVertexNum, g_CubeIndices, g_CubeIndexNum);
	}
	else
	{
		*meshData = CreateMesh(pDevice, g_CubeVertices, g_CubeVertexNum, g_CubeIndices, g_CubeIndexNum);
	}

	// マテリアルの設定
	meshData->pMaterials = materials;

	return meshData;
}


// ----------------------------------------------------------------------------------------------------------------------
//       * MeshFactory Class - 球の作成- *
// 引数 1.マテリアル
// 		2.マテリアル数
// ----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MeshResourceData> MeshInfoFactory::CreateSphereInfo(RendererEngine& renderer, std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale)
{
	auto pDevice = renderer.get_Device();
	auto meshData = std::make_shared<MeshResourceData>();

	// 分割数（増やすと滑らかになる）
	const UINT stackCount = 24; // 緯度分割
	const UINT sliceCount = 24; // 経度分割
	const float radius = 1.0f;

	// 頂点
	const int numVertex = (stackCount + 1) * (sliceCount + 1);
	VERTEX::VERTEX_Static vertices[numVertex];

	// インデックス
	const int numIndex = stackCount * sliceCount * 6;
	WORD indices[numIndex];


	// 頂点生成
	UINT vertexIndex = 0;
	for (UINT i = 0; i <= stackCount; ++i)
	{
		float phi = static_cast<float>(M_PI) * i / stackCount; // 緯度 [0, π]

		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = (static_cast<float>(M_PI) * 2.0f) * j / sliceCount; // 経度 [0, 2π]

			float x = radius * sinf(phi) * cosf(theta);
			float y = radius * cosf(phi);
			float z = radius * sinf(phi) * sinf(theta);

			VEC3 pos = VEC3(x, y, z);
			VEC3 normal = VEC3::FromXMVECTOR(DirectX::XMVector3Normalize(pos)); // 単位化
			VEC4 color = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
			VEC2 uv = VEC2(
				(float)j / sliceCount,
				(float)i / stackCount
			);

			vertices[vertexIndex++] = { pos, uv,color,normal };
		}
	}

	// インデックス生成
	UINT index = 0;
	for (UINT i = 0; i < stackCount; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			UINT i0 = i * (sliceCount + 1) + j;
			UINT i1 = i0 + 1;
			UINT i2 = (i + 1) * (sliceCount + 1) + j;
			UINT i3 = i2 + 1;

			// 三角形2枚で四角形を構成
			indices[index++] = (WORD)i0;
			indices[index++] = (WORD)i2;
			indices[index++] = (WORD)i1;

			indices[index++] = (WORD)i1;
			indices[index++] = (WORD)i2;
			indices[index++] = (WORD)i3;
		}
	}

	// 作成
	*meshData = CreateMesh(pDevice, vertices, numVertex, indices, numIndex);

	// マテリアル設定
	meshData->pMaterials = materials;

	return meshData;
}

// ----------------------------------------------------------------------------------------------------------------------
//       * MeshFactory Class - 平面の作成- *
// 引数 1.描画エンジン
// 		2.マテリアル
// 		3.マテリアル数
// 		4.ノーマルマップを使用するか
// 		5.タイルリングスケール
// ----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MeshResourceData> MeshInfoFactory::CreatePlaneInfo(RendererEngine& renderer, std::weak_ptr<Material> materials, UINT matNum, bool isNormalMap, const VECTOR2::VEC2& _tilingScale)
{
	auto pDevice = renderer.get_Device();
	auto meshData = std::make_shared<MeshResourceData>();

	const VEC3 pos[]
	{
		VEC3(-1.0f, 0.0f,  1.0f),
		VEC3( 1.0f, 0.0f,  1.0f),
		VEC3(-1.0f, 0.0f, -1.0f),
		VEC3( 1.0f, 0.0f, -1.0f),
	};

	const VEC2 uv[]
	{
		VEC2(0.0f, 0.0f) * _tilingScale,
		VEC2(1.0f, 0.0f) * _tilingScale,
		VEC2(0.0f, 1.0f) * _tilingScale,
		VEC2(1.0f, 1.0f) * _tilingScale,
	};

	// 法線マップあり
	if (isNormalMap)
	{
		VERTEX::VERTEX_Static_N vertices[] = {
			// 座標   // uv  // カラー					   // 法線                          
			{ pos[0], uv[0], VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), VEC3(),VEC3() }, // 8 左上
			{ pos[1], uv[1], VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), VEC3(),VEC3() }, // 9 右上
			{ pos[2], uv[2], VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), VEC3(),VEC3() }, // 10左下
			{ pos[3], uv[3], VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 1.0f, 0.0f), VEC3(),VEC3() }, // 11右下
		};
		
		// 接線・副接線を求める
		Tool::CalcTangentAndBitangent(vertices, g_PlaneVertexNum, g_PlaneIndices, g_PlaneIndexNum);

		*meshData = CreateMesh(pDevice, vertices, g_PlaneVertexNum, g_PlaneIndices, g_PlaneIndexNum);
	}
	// 法線マップなし
	else
	{
		*meshData = CreateMesh(pDevice, g_PlaneVertices, g_PlaneVertexNum, g_PlaneIndices, g_PlaneIndexNum);
	}

	// マテリアル情報設定
	meshData->pMaterials = materials;

	return meshData;
}

// ----------------------------------------------------------------------------------------------------------------------
//       * MeshFactory Class - スプライト用クアッドの作成- *
// ----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MeshResourceData> MeshInfoFactory::CreateSpriteQuadInfo(RendererEngine& renderer, float w, float h)
{
	auto pDevice = renderer.get_Device();
	auto meshData = std::make_shared<MeshResourceData>();

	// 中心位置
	VEC2 centerVec = VEC2(0.0f, 0.0f);
	float hw  = w * 0.5f;
	float hh  = h * 0.5f;

	// 頂点情報
	VERTEX::VERTEX_Static vertices[]{
		// 座標                    // uv             // カラー                      // 法線                 
		{ VEC3(0.0f, 0.0f, 0.0f),  VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 左上
		{ VEC3(w,    0.0f, 0.0f),  VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 右上
		{ VEC3(0.0f, h,    0.0f),  VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 左下
		{ VEC3(w,    h,    0.0f),  VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 右下
	};

	*meshData = CreateMesh(pDevice, vertices, g_SpriteQuadVertexNum, g_QuadIndices, g_SpriteQuadIndexNum, true);

	// マテリアル情報設定
	meshData->pMaterials;

	return meshData;
}


// ----------------------------------------------------------------------------------------------------------------------
//       * MeshFactory Class - レンダーターゲット用スプライトクアッドの作成- *
// ----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MeshResourceData> MeshInfoFactory::CreateRTSpriteInfo(RendererEngine& renderer, float w, float h)
{
	auto pDevice = renderer.get_Device();
	auto meshData = std::make_shared<MeshResourceData>() ;

	// 中心位置
	VEC2 centerVec = VEC2(0.0f, 0.0f);
	float hw = 0.5f;
	float hh = 0.5f;

	/* テクスチャ座標では
	* 上：0.0
	* 下：1.0
	* だけど、 3D座標系では反対の為、レンダーターゲットにする場合はV座標を反転させる
	*/

	// 画面分割的なのができた
	
	//int bunkatu = 2;
	//// 頂点情報
	//meshInfo->pVertices = new BASE_VERTEX::VERTEX[meshInfo->NumVertex]{
	//	// 座標												// 法線                  // カラー                      // uv
	//	{ VEC3(centerVec.x - hw, centerVec.y - hh,  0.0f), VEC3(0.0f, 0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(0.0f - bunkatu + 1,	1.0f + bunkatu)}, // 8 左上
	//	{ VEC3(centerVec.x + hw, centerVec.y - hh,  0.0f), VEC3(0.0f, 0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(1.0f + bunkatu,		1.0f + bunkatu)}, // 9 右上
	//	{ VEC3(centerVec.x - hw, centerVec.y + hh,  0.0f), VEC3(0.0f, 0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(0.0f - bunkatu + 1,	0.0f - bunkatu + 1)}, // 10左下
	//	{ VEC3(centerVec.x + hw, centerVec.y + hh,  0.0f), VEC3(0.0f, 0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f),  VEC2(1.0f + bunkatu,		0.0f - bunkatu + 1)}, // 11右下
	//};
	 
	// 頂点情報
	VERTEX::VERTEX_Static vertices[]{
		// 座標                   // uv             // カラー                      // 法線                 
		{ VEC3(0.0f, 0.0f, 0.0f), VEC2(0.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 左上
		{ VEC3(w,    0.0f, 0.0f), VEC2(1.0f, 0.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 右上
		{ VEC3(0.0f, h,    0.0f), VEC2(0.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 左下
		{ VEC3(w,    h,    0.0f), VEC2(1.0f, 1.0f), VEC4(1.0f, 1.0f, 1.0f, 1.0f), VEC3(0.0f, 0.0f, -1.0f)}, // 右下
	};


	*meshData = CreateMesh(pDevice, vertices, g_RTSpriteQuadVertexNum, g_QuadIndices, g_RTSpriteQuadIndexNum);

	// マテリアル情報設定
	meshData->pMaterials;

	return meshData;
}
