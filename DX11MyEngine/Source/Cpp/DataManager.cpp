#include "pch.h"
#include "DataManager.h"
#include "RendererEngine.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "Component_SpriteRenderer.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_PlayerController.h"
#include "Component_BoxCollider.h"
#include "Component_3DCamera.h"
#include "Component_Transform.h"
#include "Component_TrailRenderer.h"
#include "Component_LineRenderer.h"
#include "Component_DirectionalLight.h"
#include "Component_AssultRifle.h"
#include "Component_Health.h"
#include "Component_PointLight.h"
#include "Component_WeaponController.h"
#include "GameObject.h"
#include "MeshFactory.h"
#include "InputFactory.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace VECTOR4;
using namespace UtilityData;
using namespace EnemyData;
constexpr float VOLUME_SCALE = 0.01f;  // 音量のスケール（0.0f～1.0f）
constexpr float PLAYER_HP = 1000.0f;	// プレイヤーのHP（今後、成長要素が増えたら、保存できるように）
//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
DataManager::DataManager():
m_ScreenWidth(0),
m_ScreenHeight(0),
m_DefaultFov(0.0f),
m_IsDebugMode(false),
m_IsUseWeapon(true),
m_IsCameraControl(true),
m_pRenderer(nullptr),
m_SelectWeaponID(),
m_IsPlayerDead(false),
m_IsMissionCleared(false),
m_PlayerHP(PLAYER_HP),
m_SelectDifficultyLevel(DIFFICULTY_LEVEL::NORMAL),
m_IsPause(false),
m_IsTitle(false)
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
DataManager::~DataManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]なし
//* [返値]
//* true  : 成功 
//* false : 失敗 
//*----------------------------------------------------------------------------------------
bool DataManager::Init(class RendererEngine* pRenderer)
{
	m_pRenderer = pRenderer;

	// 初期武器
	m_SelectWeaponID[0] = 0;
	m_SelectWeaponID[1] = 1;

	// ユーザー設定データの初期化
	m_UserConfigData._BGMVolume = 0;
	m_UserConfigData._SEVolume = 0;
	m_UserConfigData._mouseSensitivity = 40.0f;
	m_UserConfigData._isInvertY = false;
	m_UserConfigData._isCameraShake = true;
	m_UserConfigData._isShadowEnabled = true;

	Master::m_pSoundManager->set_Volume(SOUND_TYPE::BGM, m_UserConfigData._BGMVolume * VOLUME_SCALE);
	Master::m_pSoundManager->set_Volume(SOUND_TYPE::SE, m_UserConfigData._SEVolume * VOLUME_SCALE);

	// TODO:敵の難易度係数一旦ここに書く
	// 攻撃を難易度ごとにjsonで分けてしまったので、攻撃力に関しては今のところ反映してない
	//																			hp	  atk   speed
	m_EnemyDifficultyFactorArray[UINT_CAST(DIFFICULTY_LEVEL::EASY)]			= { 0.6f, 0.5f, 1.0f };
	m_EnemyDifficultyFactorArray[UINT_CAST(DIFFICULTY_LEVEL::NORMAL)]		= { 1.0f, 1.0f, 1.0f };
	m_EnemyDifficultyFactorArray[UINT_CAST(DIFFICULTY_LEVEL::HARD)]			= { 1.0f, 1.5f, 1.0f };
	m_EnemyDifficultyFactorArray[UINT_CAST(DIFFICULTY_LEVEL::DISASTER)]		= { 1.0f, 3.5f, 1.0f };
	m_EnemyDifficultyFactorArray[UINT_CAST(DIFFICULTY_LEVEL::IMPOSSIBLE)]	= { 1.0f, 4.0f, 1.0f };

	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】タイトルロードの開始時にセットするデータ
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]
//* true  : 成功 
//* false : 失敗 
//*----------------------------------------------------------------------------------------
bool DataManager::SettingsData_TitleLoadOnEnter(RendererEngine& renderer)
{
	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】タイトルロードの終了時にセットするデータ
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]
//* true  : 成功 
//* false : 失敗 
//*----------------------------------------------------------------------------------------
bool DataManager::SettingsData_TitleLoadOnExit(RendererEngine& renderer)
{
	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】ミッション開始時のデータセット
//*
//* [引数]
//* &renderer : 描画エンジンの参照 
//* _missionNumber : ミッション番号（0から）
//* [返値]
//* true  : 成功 
//* false : 失敗 
//*----------------------------------------------------------------------------------------
bool DataManager::SettingsData_MissionLoad(RendererEngine& renderer, UINT _missionNumber)
{
	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】ミッション終了時のデータセット
//*
//* [引数]
//* &renderer : 描画エンジンの参照 
//* _missionNumber : ミッション番号（0から）
//* [返値]
//* true  : 成功 
//* false : 失敗 
//*----------------------------------------------------------------------------------------
bool DataManager::SettingsData_MissionTermination(RendererEngine& renderer, UINT _missionNumber)
{
	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】ゲーム中に使用したリソースをクリアする
//*		リザルトや、再出撃時に呼ぶ 
//* 
//* [引数]
//* &renderer : 描画エンジンの参照 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void DataManager::ClearGameSceneResource(RendererEngine& renderer)
{
	auto player = Master::m_pGameObjectManager->get_ObjectByTag("Player");
	// プレイヤーの武器をクリアする
	if (player){
		player->get_Component<WeaponController>()->ClearWeapon();
		player->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	}

	// 弾をクリア
	Master::m_pBulletManager->clear_CrntActiveBullet();

	// アイテムをクリア	
	Master::m_pItemManager->clear_AllItem();

	// 削除禁止でないオブジェクトを全てクリア
	Master::m_pGameObjectManager->clear_NotIsDontDestroyObject();

	// 全てのエフェクトを停止する
	Master::m_pEffectManager->StopAllEffects();

	// パラメータを戻す
	Master::m_pDataManager->set_IsMissionCleared(false);
	Master::m_pDataManager->set_IsPlayerDead(false);
}

//*---------------------------------------------------------------------------------------
//*【?】カメラにFovをセットする
//*
//* [引数]
//* _fov : fov
//* [返値]
//* true  : 成功 
//* false : 失敗 
//*----------------------------------------------------------------------------------------
void DataManager::set_Fov(float _fov)
{
	// 描画エンジンにFOVをセット
	if (auto camera = m_pCameraComponent.lock()) {
		camera->set_Fov(_fov);
	}
}

//*---------------------------------------------------------------------------------------
//*【?】カメラにFovをセットする
//*
//* [引数]
//* _fov : fov
//* [返値]
//* true  : 成功 
//* false : 失敗 
//*----------------------------------------------------------------------------------------
float DataManager::get_Fov()
{
	if (auto camera = m_pCameraComponent.lock()) {
		return camera->get_Fov();
	}
	return 0.0f;
}


//*----------------------------------------------------------------------------------------
//*【?】カメラ座標の取得
//*----------------------------------------------------------------------------------------
VECTOR3::VEC3 DataManager::get_CameraPos()const
{
	if (auto camera = m_pCameraComponent.lock()) {
		return camera->get_CameraPos();
	}

	return VEC3();
}


//*----------------------------------------------------------------------------------------
//*【?】BGM音量の設定
//*----------------------------------------------------------------------------------------
void DataManager::set_BGMVolume(int _vol)
{
	m_UserConfigData._BGMVolume = _vol;
	Master::m_pSoundManager->set_Volume(SOUND_TYPE::BGM, m_UserConfigData._BGMVolume * VOLUME_SCALE);
}

//*----------------------------------------------------------------------------------------
//*【?】SE音量の設定
//*----------------------------------------------------------------------------------------
void DataManager::set_SEVolume(int _vol)
{
	m_UserConfigData._SEVolume = _vol;
	Master::m_pSoundManager->set_Volume(SOUND_TYPE::SE, m_UserConfigData._SEVolume * VOLUME_SCALE);
}


//*----------------------------------------------------------------------------------------
//*【?】マウス感度の設定
//*----------------------------------------------------------------------------------------
void DataManager::set_MouseSensitivity(float _sens)
{
	m_UserConfigData._mouseSensitivity = _sens;
}

//*----------------------------------------------------------------------------------------
//*【?】Y反転の設定
//*----------------------------------------------------------------------------------------
void DataManager::set_IsInvertY(bool _isInv)
{
	m_UserConfigData._isInvertY = _isInv;
}

//*----------------------------------------------------------------------------------------
//*【?】カメラシェイクの有無の設定
//*----------------------------------------------------------------------------------------
void DataManager::set_IsCameraShake(bool _isShake)
{
	m_UserConfigData._isCameraShake = _isShake;
}

//*----------------------------------------------------------------------------------------
//*【?】シャドウの有無の設定
//*----------------------------------------------------------------------------------------
void DataManager::set_IsShadowEnabled(bool _isShadow)
{
	m_UserConfigData._isShadowEnabled = _isShadow;
}

//*----------------------------------------------------------------------------------------
//*【?】エネミーの難易度係数取得
//*----------------------------------------------------------------------------------------
const EnemyData::EnemyDifficultyFactor& DataManager::get_EnemyDifficultyFactor()const
{
	return m_EnemyDifficultyFactorArray[UINT_CAST(m_SelectDifficultyLevel)];
}

//*----------------------------------------------------------------------------------------
//*【?】スクリーンのサイズを設定＆定数バッファも書き換える
//*----------------------------------------------------------------------------------------
void DataManager::set_ScreenSize(UINT _w, UINT _h)
{
	m_ScreenWidth = _w;
	m_ScreenHeight = _h;

	// 定数バッファも更新
	CB_WINDOW cbWindow{};
	cbWindow.WindowWidth = static_cast<float>(m_ScreenWidth);
	cbWindow.WindowHeight = static_cast<float>(m_ScreenHeight);
	Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::WINDOW, (void*)&cbWindow, sizeof(CB_WINDOW));
}
