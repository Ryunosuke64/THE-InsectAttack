#include "pch.h"
#include "Component_TrailRenderer.h"
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
TrailRenderer::TrailRenderer(std::weak_ptr<GameObject> pOwner, int updateRank) 
	: IComponent(pOwner, updateRank),
	m_MinVertexDistance(10.0f),
	m_DrawTime(60),
	m_Width(10.0f),
	m_CrntTrailPos(VEC3()),
	m_pCBMaterialDataSet(nullptr),
	//m_IsView(true),
	m_IsPosRand(false),
	m_PosRandVec(VEC3()),
	m_EmissivePower(1.0f),
	m_Color(VEC4(1.0f, 1.0f, 1.0f,1.0f))
{
	this->set_Tag("TrailRenderer");


	m_pTex = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Particle/Gradient/gradient-horizontal-mirror.png");
}


//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
TrailRenderer::~TrailRenderer()
{
	if (m_pCBMaterialDataSet) {
		if (m_pCBMaterialDataSet->pBuff) {
			m_pCBMaterialDataSet->pBuff->Release();
		}
		delete m_pCBMaterialDataSet;
		m_pCBMaterialDataSet = nullptr;
	}

	m_pVertesBuffer.Reset();
}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void TrailRenderer::Start(RendererEngine &renderer)
{
	if (!Setup(renderer))
	{
		MessageBoxA(NULL,"セットアップができませんでした","TrailRenderer Error",MB_OK);
		return;
	}
	//m_pTex = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/rust_coarse_01_arm_1k.jpg");
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void TrailRenderer::LateUpdate(RendererEngine &renderer)
{
	// 表示時間が0以下なら問答無用で返す
	if (m_DrawTime <= 0)
	{
		return;
	}

	auto transform = m_pOwner.lock()->get_Transform().lock();
	m_CrntTrailPos = transform->get_VEC3ToPos();	// 現在位置を末端位置とする

	// 距離判定（最後にリストに入れた地点と比較）
	float dist = 0.0f;
	if (!m_TrailInfoList.empty()) {
		dist = VEC3::Distance(m_CrntTrailPos, m_TrailInfoList.back()._pos);
	}
	else {	// 初回用
		dist = m_MinVertexDistance;
	}

	if (dist >= m_MinVertexDistance)
	{
		TrailInfo trail;
		trail._time = m_DrawTime;
		trail._pos = m_CrntTrailPos;// 現在位置を追加

		if (m_IsPosRand)
		{
			trail._pos.x += Tool::RandRange(-m_PosRandVec.x, m_PosRandVec.x);
			trail._pos.y += Tool::RandRange(-m_PosRandVec.y, m_PosRandVec.y);
			trail._pos.z += Tool::RandRange(-m_PosRandVec.z, m_PosRandVec.z);
		}

		// 追加
		m_TrailInfoList.push_back(trail);
	}

	// 存在時間を過ぎたら消していく
	// 順番は変わらないはずなので、前の方から調べていく
	if (m_TrailInfoList.front()._time <= 0.0f)
	{
		m_TrailInfoList.pop_front();
	}

	// 頂点数が最大になったら後ろから消していく
	if ((m_TrailInfoList.size() * 2) >= MAX_TRAIL_VERTEX_NUM)
	{
		m_TrailInfoList.pop_front();
	}
}

//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void TrailRenderer::Draw(RendererEngine &renderer)
{
	// 表示時間が0以下なら問答無用で返す
	if (m_DrawTime <= 0)
	{
		return;
	}

	// シャドウパス時には返す
	if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW){
		return;
	}

	auto pContext = renderer.get_DeviceContext();

	// ポーズ中じゃないなら、更新
	if (!Master::m_pDataManager->get_IsPause())
	{
		// 頂点更新
		VertexUpdate(renderer);
	}

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
	
	Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::ALPHA);

	// 描画（頂点数は一つの座標につき左右で2つあるので2倍）
	pContext->Draw(static_cast<UINT>(m_TrailInfoList.size() * 2), 0);
	Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::NONE);
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
void TrailRenderer::VertexUpdate(RendererEngine& renderer)
{
	auto pContext = renderer.get_DeviceContext();
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	if (m_TrailInfoList.empty() || m_TrailInfoList.size() == 1)return;

	// GPUメモリにアクセス
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pContext->Map(m_pVertesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// カメラ座標
	VEC3 cameraPos = Master::m_pDataManager->get_CameraPos();

	VERTEX_Static *pVertices = (VERTEX_Static *)mappedResource.pData;
	for (int i = 0; i < m_TrailInfoList.size(); i++)
	{
		VEC3 tail = m_TrailInfoList[i]._pos;// 末尾（今作るのはこれ）

		m_TrailInfoList[i]._time -= deltaTime;

		// 時間の比率を求める
		float w_t = m_TrailInfoList[i]._time / m_DrawTime;

		VEC3 headDir;

		// 先頭位置への方向を求める
		if (i < (m_TrailInfoList.size() - 1)){
			headDir = (m_TrailInfoList[i + 1]._pos - tail).Normalize();
		}
		else{
			// 先頭位置がない場合、前の情報から仮想の位置を作る
			VEC3 prev = m_TrailInfoList[i - 1]._pos;
			headDir = (tail - prev).Normalize();
		}

		// カメラへの方向
		VEC3 viewDir = (tail - cameraPos).Normalize();
		

		// メッシュの広がる方向ベクトルを作る
		VEC3 dir = VEC3::Cross(headDir, viewDir);

		VEC3 r_NewPos = tail + (dir * ( m_Width * w_t));
		VEC3 l_NewPos = tail + (dir * (-m_Width * w_t));


		VERTEX_Static r_V = VERTEX_Static();
		VERTEX_Static l_V = VERTEX_Static();
		r_V = pVertices[i * 2 + 0];	// 左側
		l_V = pVertices[i * 2 + 1];	// 右側

		// 座標
		r_V.pos = r_NewPos;
		l_V.pos = l_NewPos;

		// カラー
		r_V.color.AllOne();
		l_V.color.AllOne();

		r_V.uv = VEC2(1.0f, (float)i / (float)(m_TrailInfoList.size() - 1));
		l_V.uv = VEC2(0.0f, (float)i / (float)(m_TrailInfoList.size() - 1));

		r_V.normal.AllOne();
		l_V.normal.AllOne();
		
		pVertices[i * 2 + 0] = r_V;
		pVertices[i * 2 + 1] = l_V;

	}

	// データのコピー 
	//memcpy(mappedResource.pData, pVertices, sizeof(VERTEX_Static) * m_TrailInfoList.size() * 2);

	// アクセス終了
	pContext->Unmap(m_pVertesBuffer.Get(), 0);
}


void CalcTrailSegmentToViewDirection(RendererEngine& renderer)
{

}

void CalcTrailSegment(RendererEngine& renderer)
{

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
void TrailRenderer::ConstantBufferUpdate(RendererEngine& renderer)
{
	auto pContext = renderer.get_DeviceContext();

	CB_MATERIAL cbMaterial{};
	cbMaterial.Diffuse = m_Color;
	cbMaterial.EmissivePower = m_EmissivePower;
	cbMaterial.EmissiveColor = VEC3(m_Color.x, m_Color.y, m_Color.z);
	cbMaterial.Specular = VEC4(1.0f, 1.0f, 1.0f, 0.1f);
	cbMaterial.SpecularPower = 100.0f;
	cbMaterial.EnvironmentReflectionStrength = 0.0f;
	Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL, (void*)&cbMaterial, sizeof(CB_MATERIAL));


	//if (m_pCBMaterialDataSet == nullptr)return;

	//// GPUメモリにアクセス
	//D3D11_MAPPED_SUBRESOURCE mappedResource;
	//pContext->Map(m_pCBMaterialDataSet->pBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	//m_pCBMaterialDataSet->Data.Diffuse		 = m_Color;
	//m_pCBMaterialDataSet->Data.EmissivePower = m_EmissivePower;
	//m_pCBMaterialDataSet->Data.EmissiveColor = VEC3(m_Color.x, m_Color.y, m_Color.z);
	//m_pCBMaterialDataSet->Data.Specular		 = VEC4(1.0f, 1.0f, 1.0f, 0.1f);
	//m_pCBMaterialDataSet->Data.SpecularPower = 100.0f;

	//// データのコピー 
	//memcpy(mappedResource.pData, &m_pCBMaterialDataSet->Data, sizeof(CB_MATERIAL));

	//// アクセス終了
	//pContext->Unmap(m_pCBMaterialDataSet->pBuff, 0);

	//// 定数バッファへ送信
	//pContext->PSSetConstantBuffers(4,1, &m_pCBMaterialDataSet->pBuff);

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
bool TrailRenderer::Setup(RendererEngine &renderer)
{
	// 頂点バッファの作成
	if (!CreateVertexBuffer(renderer))
	{
		MessageBoxA(NULL, "頂点バッファの作成が出来ませんでした", "TrailRenderer Error", MB_OK);
		return false;
	}
	// 定数バッファの作成
	if (!CreateConstantBuffer(renderer))
	{
		MessageBoxA(NULL, "定数バッファの作成が出来ませんでした", "TrailRenderer Error", MB_OK);
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
bool TrailRenderer::CreateVertexBuffer(RendererEngine &renderer)
{
	auto pDevice = renderer.get_Device();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DYNAMIC;								// 動的書き込み
	bd.ByteWidth = sizeof(VERTEX_Static) * MAX_TRAIL_VERTEX_NUM;// バッファのサイズ
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
bool TrailRenderer::CreateConstantBuffer(RendererEngine &renderer)
{
	auto pDevice = renderer.get_Device();

	// マテリアル --------------------------------------------------------------
	m_pCBMaterialDataSet = new CB_MATERIAL_SET();

	D3D11_BUFFER_DESC mat_BufferDesc;
	ZeroMemory(&mat_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	mat_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	mat_BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mat_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mat_BufferDesc.ByteWidth = sizeof(CB_MATERIAL);

	// バッファの作成
	HRESULT hr = pDevice->CreateBuffer(&mat_BufferDesc, nullptr, &m_pCBMaterialDataSet->pBuff);
	if (FAILED(hr))
	{
		delete m_pCBMaterialDataSet;
		return false;
	}

	return true;
}


void TrailRenderer::set_TrailPos(VECTOR3::VEC3 _head, VECTOR3::VEC3 _tail)
{
}