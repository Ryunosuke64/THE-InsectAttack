#include "pch.h"
#include "LightManager.h"
#include "RendererEngine.h"
#include "Component_Transform.h"

using namespace VECTOR3;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* なし 
//*----------------------------------------------------------------------------------------
LightManager::LightManager():
	m_pContext(nullptr)
{};


//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
LightManager::~LightManager()
{
	m_TemporaryPointLightData.clear();
	m_TemporaryDirectionLightData.clear();
	m_TransientPointLightPool.Clear();

	m_pContext = nullptr;
};

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* std::shared_ptr<RendererEngine> : 描画エンジンのポインタを持つ
//*
//* [返値]
//* true  : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool LightManager::Init(std::shared_ptr<RendererEngine> pRenderer)
{
	m_pRenderer = pRenderer;

	auto pDevice = m_pRenderer.lock()->get_Device();
	m_pContext = m_pRenderer.lock()->get_DeviceContext();

	// バッファ作成
	if(!CreatePointLightCBuffer(pDevice))return false;
	if (!CreateDirectionLightCBuffer(pDevice))return false;

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* なし
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void LightManager::Update()
{
	const bool isPause = Master::m_pDataManager->get_IsPause();
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	if (isPause)
	{
		deltaTime = 0.0f;
	}

	m_TransientPointLightPool.Update(
		deltaTime,
		m_TemporaryPointLightData,
		POINTLIGHT_MAX_NUM);

	// ライトデータを定数バッファへ送る
	LightDataSetupAndBindToConstantBuffer();

	//DirectionLight_SetCBuffer();

	// ポーズ中は直前のポイントライト用バッファを維持する
	if (isPause)
	{
		m_TemporaryPointLightData.clear();
		return;
	}

	//PointLight_SetCBuffer();
}

void LightManager::Term()
{
	m_TemporaryPointLightData.clear();
	m_TemporaryDirectionLightData.clear();
	m_TransientPointLightPool.Clear();

}


//*---------------------------------------------------------------------------------------
//*【?】ライト情報を構築し、定数バッファにセット
//*
//* [引数]
//* なし
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void LightManager::LightDataSetupAndBindToConstantBuffer()
{

	VEC3 cameraPos = Master::m_pDataManager->get_CameraPos();
	CB_LIGHTS cbLights{};
	cbLights.EyePos = cameraPos;	// カメラ位置

	// ----------------------------------------------------------
	// ディレクションライト用データの更新
	// ----------------------------------------------------------
	memcpy(
		&cbLights.DirLights,
		m_TemporaryDirectionLightData.data(),
		m_TemporaryDirectionLightData.size() * sizeof(CB_DirectionLightData)
	);

	// ----------------------------------------------------------
	// ポイントライト用データの更新
	// ----------------------------------------------------------
	// サイズがオーバーしてないか
	uint32_t pLightCount = static_cast<uint32_t>(m_TemporaryPointLightData.size());
	if (pLightCount > POINTLIGHT_MAX_NUM) {
		pLightCount = POINTLIGHT_MAX_NUM;
	}
	// ポイントライトの数をセット
	cbLights.PointLightCount = pLightCount;

	// ポイントライトが1個以上あればメモリコピー
	if (pLightCount > 0)
	{
		memcpy(
			cbLights.PointLights,
			m_TemporaryPointLightData.data(),
			pLightCount * sizeof(CB_PointLightData)
		);
	}

	// ----------------------------------------------------------
	// スポットライト用データの更新
	// ----------------------------------------------------------
	// サイズがオーバーしてないか
	uint32_t spLightCount = static_cast<uint32_t>(m_TemporarySpotLightData.size());
	if (spLightCount > SPOTLIGHT_MAX_NUM) {
		spLightCount = SPOTLIGHT_MAX_NUM;
	}
	// スポットライトの数をセット
	cbLights.SpotLightCount = spLightCount;

	// スポットライト数が1個以上あればメモリコピー
	if (spLightCount > 0)
	{
		memcpy(
			cbLights.SpotLights,
			m_TemporarySpotLightData.data(),
			spLightCount * sizeof(CB_SpotLightData)
		);
	}


	/* 定数バッファにセット */
	Master::m_pShaderManager->BindConstantBuffer(
		CONSTANT_BUFFER_TYPE::LIGHTS,
		(void*)&cbLights, sizeof(CB_LIGHTS)
	);

	// 配列のクリア
	m_TemporaryPointLightData.clear();
	m_TemporaryDirectionLightData.clear();
	m_TemporarySpotLightData.clear();

}


//*---------------------------------------------------------------------------------------
//*【?】ディレクションライトを定数バッファにセット
//*
//* [引数]
//* なし
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void LightManager::DirectionLight_SetCBuffer()
{
	//VEC3 cameraPos = Master::m_pDataManager->get_CameraPos();

	//CB_DIRECTION_LIGHT cbDirectionLight{};

	//// バッファの更新
	//memcpy(
	//	&cbDirectionLight.Lights,
	//	m_TemporaryDirectionLightData.data(),
	//	m_TemporaryDirectionLightData.size() * sizeof(CB_DirectionLightData)
	//);

	//// 一旦直接セット
	//// TODO: ライトをまとめた構造体を作り、ポイント、ディレクション問わず一気に送ってしまい、その時に一緒にカメラも送るか、
	////		 カメラを完全に分離するか…
	//cbDirectionLight.EyePos = cameraPos;

	///* 定数バッファにセット */
	//Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::DIRECTIONAL_LIGHT, (void*)&cbDirectionLight, sizeof(CB_DIRECTION_LIGHT));


	//D3D11_MAPPED_SUBRESOURCE mappedResource;
	//m_pCBDirectionLightSet->Data[0].EyePos = cameraPos;

	//// GPUメモリにアクセス
	//m_pContext->Map(m_pCBDirectionLightSet->pBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	//// データのコピー 
	//memcpy(mappedResource.pData, &m_pCBDirectionLightSet->Data, sizeof(CB_DIRECTION_LIGHT) * DIRECTIONLIGHT_MAX_NUM);

	//// アクセス終了
	//m_pContext->Unmap(m_pCBDirectionLightSet->pBuff, 0);

	//// ピクセルシェーダにセット
	//m_pContext->VSSetConstantBuffers(5, 1, &m_pCBDirectionLightSet->pBuff);
	//m_pContext->PSSetConstantBuffers(5, 1, &m_pCBDirectionLightSet->pBuff);

	m_TemporaryDirectionLightData.clear();
}


//*---------------------------------------------------------------------------------------
//*【?】ポイントライトを定数バッファにセット
//*
//* [引数]
//* なし
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void LightManager::PointLight_SetCBuffer()
{
	//// サイズがオーバーしてないか
	//uint32_t lightCount = static_cast<uint32_t>(m_TemporaryPointLightData.size());
	//if (lightCount > POINTLIGHT_MAX_NUM) lightCount = POINTLIGHT_MAX_NUM;

	//CB_POINT_LIGHT cbPointLight {};

	//// ライトの個数セット
	//cbPointLight.LightCount = lightCount;

	//if (lightCount > 0)
	//{
	//	memcpy(cbPointLight.Lights, m_TemporaryPointLightData.data(), lightCount * sizeof(CB_PointLightData));
	//}

	//// 0灯の場合も送信し、前フレームのライト情報が残らないようにする
	//Master::m_pShaderManager->BindConstantBuffer(
	//	CONSTANT_BUFFER_TYPE::POINT_LIGHT,
	//	(void*)&cbPointLight,
	//	sizeof(CB_POINT_LIGHT));

	//m_TemporaryPointLightData.clear();
}


//*---------------------------------------------------------------------------------------
//*【?】ポイントライト用の定数バッファ作成
//*
//* [引数]
//* ID3D11Device* : 作成に必要なデバイス
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool LightManager::CreatePointLightCBuffer(ID3D11Device* pDevice)
{
	//// ライト定数バッファの生成
	//m_pCBPointLightSet = new CB_POINT_LIGHT_SET;
	//if (m_pCBPointLightSet == nullptr) {
	//	return false;
	//}

	//// 定数バッファの設定
	//D3D11_BUFFER_DESC bd{};
	//ZeroMemory(&bd, sizeof(bd));
	//bd.Usage = D3D11_USAGE_DYNAMIC;						// 頻繁更新
	//bd.ByteWidth = sizeof(CB_POINT_LIGHT);				// バッファのサイズ
	//bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;			// 定数バッファとして使う
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;			// CPUから書き込み
	//bd.MiscFlags = 0;
	//bd.StructureByteStride = 0;

	//// 定数バッファの生成
	//HRESULT hr = pDevice->CreateBuffer(&bd, nullptr, &m_pCBPointLightSet->pBuff);
	//if (FAILED(hr)) {
	//	return false;
	//}

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】ディレクションライト用の定数バッファ作成
//*
//* [引数]
//* ID3D11Device* : 作成に必要なデバイス
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool LightManager::CreateDirectionLightCBuffer(ID3D11Device* pDevice)
{
	//// ライト定数バッファの生成
	//m_pCBDirectionLightSet = new CB_DIRECTION_LIGHT_SET;
	//if (m_pCBDirectionLightSet == nullptr) {
	//	return false;
	//}

	//// 定数バッファの設定
	//D3D11_BUFFER_DESC bd{};
	//ZeroMemory(&bd, sizeof(bd));
	//bd.Usage = D3D11_USAGE_DYNAMIC;						// 頻繁更新
	//bd.ByteWidth = sizeof(CB_DIRECTION_LIGHT) * DIRECTIONLIGHT_MAX_NUM;			// バッファのサイズ
	//bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;			// 定数バッファとして使う
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;			// CPUから書き込み
	//bd.MiscFlags = 0;
	//bd.StructureByteStride = 0;


	//// 定数バッファの生成
	//HRESULT hr = pDevice->CreateBuffer(&bd, nullptr, &m_pCBDirectionLightSet->pBuff);
	//if (FAILED(hr)) {
	//	return false;
	//}

	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】ライト計算に必要なカメラのトランスフォーム設定
//*
//* [引数]
//* std::weak_ptr<MyTransform>* : カメラのTransformコンポーネント
//*
//* [返値]
//* void 
//*----------------------------------------------------------------------------------------
void LightManager::set_CameraTransform(std::weak_ptr<class MyTransform> pCamTransform)
{
	m_pCameraTransform = pCamTransform;
}


//*---------------------------------------------------------------------------------------
//*【?】ポイントライトの情報をセット
//*
//* [引数]
//* const CB_PointLightData& : ポイントライト情報
//*
//* [返値]
//* void 
//*----------------------------------------------------------------------------------------
void LightManager::set_PointLightData(const CB_PointLightData& data)
{
	// ポイントライトの最大数より少なければ追加
	if (m_TemporaryPointLightData.size() < POINTLIGHT_MAX_NUM){
		m_TemporaryPointLightData.push_back(data);
	}
}

//*---------------------------------------------------------------------------------------
//*【?】スポットライトの情報をセット
//*
//* [引数]
//* const CB_PointLightData& : ポイントライト情報
//*
//* [返値]
//* void 
//*----------------------------------------------------------------------------------------
void LightManager::set_SpotLightData(const CB_SpotLightData& data)
{
	// ポイントライトの最大数より少なければ追加
	if (m_TemporarySpotLightData.size() < SPOTLIGHT_MAX_NUM){
		m_TemporarySpotLightData.push_back(data);
	}
}

//*---------------------------------------------------------------------------------------
//*【?】ディレクションライト情報をセット
//*
//* [引数]
//* const CB_DIRECTION_LIGHT& : ディレクションライト情報
//*
//* [返値]
//* void 
//*----------------------------------------------------------------------------------------
void LightManager::set_DirectionLightData(const CB_DirectionLightData& data)
{
	// ディレクションライトの最大数より少なければ追加
	if (m_TemporaryDirectionLightData.size() < DIRECTIONLIGHT_MAX_NUM){
		m_TemporaryDirectionLightData.push_back(data);
	}
}

TransientLightHandle LightManager::PlayTransientPointLight(
	const VEC3& position,
	const TransientPointLightDesc& desc)
{
	return m_TransientPointLightPool.Play(position, desc);
}

void LightManager::StopTransientPointLight(TransientLightHandle handle)
{
	m_TransientPointLightPool.Stop(handle);
}

void LightManager::ClearTransientPointLights()
{
	m_TransientPointLightPool.Clear();
}
