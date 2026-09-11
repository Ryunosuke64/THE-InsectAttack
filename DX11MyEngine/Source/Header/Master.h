#pragma once
#include "ResourceManager.h"
#include "BlendManager.h"
#include "ShaderManager.h"
#include "LightManager.h"
#include "DirectWriteManager.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "Debugger.h"
#include "EditorManager.h"
#include "CollisionManager.h"
#include "EffectManager.h"
#include "SoundManager.h"
#include "TimeManager.h"
#include "BulletManager.h"
#include "DataManager.h"
#include "UIManager.h"
#include "RandomManager.h"
#include "WeaponDataManager.h"
#include "ItemManager.h"
#include "TweenManager.h"
#include "BuildingManager.h"
#include "ScriptManager.h"
#include "EnemyManager.h"
#include "MissionDirector.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Master Class --- */
//
// 【?】シングルトンクラスをまとめたもの
//		
//		基本的に生成、初期化、更新などはDXAppクラスで行っている。
// 
//	 ・その他
//		UIManager : タイトルロードシーン
//		BulletManager : ゲームロードシーン
//		ItemManager : ゲームロードシーン
//		
// ***************************************************************************************
class Master
{
public:
	static Debugger				* m_pDebugger;			// ImGui機能ラップ
	static ShaderManager		* m_pShaderManager;		// シェーダ管理
	static LightManager			* m_pLightManager;		// ライト管理
	static DirectWriteManager	* m_pDirectWriteManager;// 文字管理
	static BlendManager			* m_pBlendManager;		// ブレンド管理
	static TweenManager			* m_pTweenManager;		// Tween管理
	static GameObjectManager	* m_pGameObjectManager;	// オブジェクト管理
	static ResourceManager		* m_pResourceManager;	// リソース管理
	static EditorManager		* m_pEditorManager;		// エディタ管理
	static InputManager			* m_pInputManager;		// 入力管理
	static CollisionManager		* m_pCollisionManager;	// 衝突管理
	static SoundManager			* m_pSoundManager;		// サウンド管理
	static EffectManager		* m_pEffectManager;		// エフェクト管理
	static TimeManager			* m_pTimeManager;		// 時間管理
	static DataManager			* m_pDataManager;		// データ管理
	static BulletManager		* m_pBulletManager;		// 弾管理
	static UIManager			* m_pUIManager;			// UI管理
	static RandomManager		* m_pRandomManager;		// 乱数管理
	static WeaponDataManager	* m_pWeaponDataManager;	// 武器データ管理
	static ItemManager			* m_pItemManager;		// アイテム管理
	static BuildingManager		* m_pBuildingManager;	// 建物管理
	static ScriptManager		* m_pScriptManager;		// AngelScript管理
	static EnemyManager			* m_pEnemyManager;		// エネミー管理
	static MissionDirector		* m_pMissionDirector;	// ミッション管理
};