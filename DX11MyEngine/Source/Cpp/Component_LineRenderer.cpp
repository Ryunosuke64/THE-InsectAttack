#include "pch.h"
#include "Component_LineRenderer.h"
#include "GameObject.h"
#include "RendererEngine.h"

using namespace VERTEX;
using namespace RenderData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
LineRenderer::LineRenderer(std::weak_ptr<GameObject> pOwner, int updateRank)
	: IComponent(pOwner, updateRank),
	m_DrawTime(60),
	m_Width(1.0f),
	m_Length(1000.0f),
	m_Dir(VEC3()),
	m_StartPos(VEC3()),
	m_Color(VEC4(1.0f, 1.0f, 1.0f, 1.0f)),
	m_IsView(true),
	m_EmissivePower(3.0f)
{
	this->set_Tag("LineRenderer");

	m_Dir = VEC3(0, 0, 1);
}


//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
LineRenderer::~LineRenderer()
{
	m_pVertesBuffer.Reset();
}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void LineRenderer::Start(RendererEngine& renderer)
{
	if (!Setup(renderer))
	{
		MessageBoxA(NULL, "セットアップができませんでした", "LineRenderer Error", MB_OK);
		return;
	}
	m_pTex = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/rust_coarse_01_arm_1k.jpg");
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void LineRenderer::LateUpdate(RendererEngine& renderer)
{


}

//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void LineRenderer::Draw(RendererEngine& renderer)
{
	// シャドウパス時には返す
	if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW) {
		return;
	}

	// 頂点更新
	VertexUpdate(renderer);

	auto pContext = renderer.get_DeviceContext();

	// 定数バッファ更新
	ConstantBufferUpdate(renderer);

	// 頂点バッファの設定
	UINT stride = sizeof(VERTEX_Static);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, m_pVertesBuffer.GetAddressOf(), &stride, &offset); // 頂点バッファをセット

	// シェーダの設定
	Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::DEFERRED_STD_TRAIL);

	// 三角形ストリップ指定
	pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::ALPHA);

	// 描画
	pContext->Draw(4, 0);
	//Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::NONE);
}


//*---------------------------------------------------------------------------------------
//*【?】頂点情報の更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
void LineRenderer::VertexUpdate(RendererEngine& renderer)
{
	auto pContext = renderer.get_DeviceContext();

	// GPUメモリにアクセス
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pContext->Map(m_pVertesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// カメラ座標
	VEC3 cameraPos = Master::m_pDataManager->get_CameraPos();

	VERTEX_Static* pVertices = (VERTEX_Static*)mappedResource.pData;

	/*
	* 頂点の更新
	*/
	{
		VEC3 tail = m_StartPos;// 末尾（始点部分から先に作る）
		VEC3 head = VEC3();	   // 先頭（終点）

		// カメラへの方向
		VEC3 viewDir = (tail - cameraPos).Normalize();

		// 先頭位置への方向
		VEC3 headDir = m_Dir;

		// メッシュの広がる方向ベクトルを作る
		VEC3 dir = VEC3::Cross(headDir, viewDir);
		VEC3 r_NewPos = tail + (dir * (m_Width));
		VEC3 l_NewPos = tail + (dir * (-m_Width));

		VERTEX_Static r_V = VERTEX_Static();
		VERTEX_Static l_V = VERTEX_Static();

		/* 始点部分 */
		r_V = pVertices[0];	// 左側
		l_V = pVertices[1];	// 右側

		r_V.pos = r_NewPos;			// 座標
		l_V.pos = l_NewPos;
		r_V.color.AllOne();			// カラー
		l_V.color.AllOne();
		r_V.uv = VEC2(1.0f, 0.0f);	// UV
		l_V.uv = VEC2(0.0f, 0.0f);
		r_V.normal.AllOne();		// 法線
		l_V.normal.AllOne();
		
		pVertices[0] = r_V;
		pVertices[1] = l_V;

		/* 終点部分 */
		r_V = pVertices[2];	// 左側
		l_V = pVertices[3];	// 右側
		
		// 始点から終点へ伸びる
		VEC3 r_EndPos = r_NewPos + (headDir * (m_Length));
		VEC3 l_EndPos = l_NewPos + (headDir * (m_Length));

		r_V.pos = r_EndPos;			// 座標
		l_V.pos = l_EndPos;
		r_V.uv = VEC2(1.0f, 1.0f);	// UV
		l_V.uv = VEC2(0.0f, 1.0f);
		pVertices[2] = r_V;
		pVertices[3] = l_V;
	}

	// データのコピー 
	memcpy(mappedResource.pData, pVertices, sizeof(VERTEX_Static) * 4);

	// アクセス終了
	pContext->Unmap(m_pVertesBuffer.Get(), 0);
}

//*---------------------------------------------------------------------------------------
//*【?】定数バッファの更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void LineRenderer::ConstantBufferUpdate(RendererEngine& renderer)
{
	auto pContext = renderer.get_DeviceContext();

	CB_MATERIAL cbMaterial{};
	cbMaterial.Diffuse = m_Color;
	cbMaterial.EmissivePower = m_EmissivePower;
	cbMaterial.EmissiveColor = VEC3(m_Color.x, m_Color.y, m_Color.z);
	cbMaterial.Specular = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
	cbMaterial.SpecularPower = 100.0f;
	cbMaterial.EnvironmentReflectionStrength = 0.0f;
	
	/* 定数バッファにセット */
	Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL, (void*)&cbMaterial, sizeof(CB_MATERIAL));

	ID3D11ShaderResourceView* tex = nullptr;
	tex = m_pTex->get_SRV();
	pContext->PSSetShaderResources(0, 1, &tex);
}

//*---------------------------------------------------------------------------------------
//*【?】セットアップ
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool LineRenderer::Setup(RendererEngine& renderer)
{
	// 頂点バッファの作成
	if (!CreateVertexBuffer(renderer))
	{
		MessageBoxA(NULL, "頂点バッファの作成が出来ませんでした", "LineRenderer Error", MB_OK);
		return false;
	}
	// 定数バッファの作成
	if (!CreateConstantBuffer(renderer))
	{
		MessageBoxA(NULL, "定数バッファの作成が出来ませんでした", "LineRenderer Error", MB_OK);
		return false;
	}

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】頂点バッファの作成
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool LineRenderer::CreateVertexBuffer(RendererEngine& renderer)
{
	auto pDevice = renderer.get_Device();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DYNAMIC;								// 動的書き込み
	bd.ByteWidth = sizeof(VERTEX_Static) * 4;					// バッファのサイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;					// 頂点バッファとして使う
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// CPUから書き込み

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = nullptr;

	// バッファの作成
	HRESULT hr = pDevice->CreateBuffer(&bd, nullptr, m_pVertesBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】定数バッファの作成
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool LineRenderer::CreateConstantBuffer(RendererEngine& renderer)
{
	//auto pDevice = renderer.get_Device();

	//// マテリアル --------------------------------------------------------------
	//m_pCBMaterialDataSet = new CB_MATERIAL_SET();

	//D3D11_BUFFER_DESC mat_BufferDesc;
	//ZeroMemory(&mat_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	//mat_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//mat_BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//mat_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//mat_BufferDesc.ByteWidth = sizeof(CB_MATERIAL);

	//// バッファの作成
	//HRESULT hr = pDevice->CreateBuffer(&mat_BufferDesc, nullptr, &m_pCBMaterialDataSet->pBuff);
	//if (FAILED(hr))
	//{
	//	delete m_pCBMaterialDataSet;
	//	return false;
	//}

	return true;
}
