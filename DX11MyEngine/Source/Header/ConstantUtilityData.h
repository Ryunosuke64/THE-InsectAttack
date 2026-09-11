#pragma once
#include "SceneStateEnums.h"

/// <summary>
/// 汎用データ
/// </summary>
namespace UtilityData
{
	/// <summary>
	/// 派閥
	/// </summary>
	enum class FACTION
	{
		PLAYER,		// プレイヤー
		ALLY,		// 味方
		ENEMY,		// 敵
		NEUTRAL,	// 中立
		VEHICLE,	// 乗り物
		ITEM,		// アイテム
	};

	/// <summary>
	/// コライダー種類
	/// </summary>
	enum class COLLIDER_TYPE
	{
		NONE,
		BOX,
		BOX_OBB,
		SPHERE,
		RAY,
	};

	/// <summary>
	/// 衝突した際の処理
	/// </summary>
	enum class COLLISION_RESPONSE : unsigned
	{
		RESPONSE_IGNORE		= 1 << 0,	// 判定しない
		RESPONSE_OVERLAP	= 1 << 1,	// 判定するが押し出さない
		RESPONSE_BLOCK		= 1 << 2,	// 判定して押し出す
	};

	/// <summary>
	/// 衝突判定のビット分け
	/// </summary>
	enum class COLLISION_CATEGORY : unsigned
	{
		NONE					= 0,
		PLAYER					= 1 << 0,	// プレイヤー
		PLAYER_BULLET			= 1 << 1,	// プレイヤーの弾
		ENEMY					= 1 << 2,	// 敵
		ENEMY_BULLET			= 1 << 3,	// 敵の弾
		DESTRUCTION_BUILDING	= 1 << 4,	// 破壊可能な建物
		BUILDING				= 1 << 5,	// 破壊不可能な建物
		ITEM					= 1 << 6,	// アイテム

		EVERY = 0xFFFFFFFF	// 全てに衝突
	};

	// 衝突判定 文字列をEnumに変換するマップ
	static std::map<std::string, COLLISION_CATEGORY> g_CollisionCategoryMap = {
		{"PLAYER",				 COLLISION_CATEGORY::PLAYER},
		{"PLAYER_BULLET",		 COLLISION_CATEGORY::PLAYER_BULLET},
		{"ENEMY",				 COLLISION_CATEGORY::ENEMY},
		{"ENEMY_BULLET",		 COLLISION_CATEGORY::ENEMY_BULLET},
		{"DESTRUCTION_BUILDING", COLLISION_CATEGORY::DESTRUCTION_BUILDING},
		{"BUILDING",			 COLLISION_CATEGORY::BUILDING},
		{"EVERY",				 COLLISION_CATEGORY::EVERY},
		{"ITEM",				 COLLISION_CATEGORY::ITEM},
	};
	// 衝突判定 Enumを文字列に変換するマップ
	static std::map<COLLISION_CATEGORY, std::string > g_CollisionCategoryStringMap = {
		{COLLISION_CATEGORY::PLAYER				  ,"PLAYER",			  },
		{COLLISION_CATEGORY::PLAYER_BULLET		  ,"PLAYER_BULLET",		  },
		{COLLISION_CATEGORY::ENEMY				  ,"ENEMY",				  },
		{COLLISION_CATEGORY::ENEMY_BULLET		  ,"ENEMY_BULLET",		  },
		{COLLISION_CATEGORY::DESTRUCTION_BUILDING ,"DESTRUCTION_BUILDING",},
		{COLLISION_CATEGORY::BUILDING			  ,"BUILDING",			  },
		{COLLISION_CATEGORY::EVERY				  ,"EVERY",				  },
		{COLLISION_CATEGORY::ITEM				  ,"ITEM",				  },
	};

	/// <summary>
	/// 難易度
	/// </summary>
	enum class DIFFICULTY_LEVEL
	{
		EASY,
		NORMAL,
		HARD,
		DISASTER,
		//APOCALYPSE,
		IMPOSSIBLE,

		NUM
	};


	/// <summary>
	/// タイトル項目
	/// </summary>
	enum class TITLEMENU_ITEM
	{
		MISSION_SELECT,	// ミッション選択
		SOLDER_SELECT,	// 兵科選択
		CONFIG,			// 設定
		EXIT,			// 終了

		NUM,
	};

	/// <summary>
	/// 兵科の種類
	/// </summary>
	enum class SOLDIER_TYPE
	{
		RANGER,	// 歩兵
		RAPID,
		SCOUT,
		HEAVY,

		NUM,
	};

	/// <summary>
	/// 設定項目
	/// </summary>
	enum class CONFIG_ITEM
	{
		BGM_VOLUME,			// BGM
		SE_VOLUME,			// SE
		MOUSE_SENSITIVITY,	// マウス感度
		INVERT_Y,			// 上下反転
		CAMERA_SHAKE,		// カメラシェイクの有無
		SHADOW_ENABLED,		// シャドウの有無

		NUM
	};


	/// <summary>
	/// ポーズ画面の項目
	/// </summary>
	enum class PAUSE_ITEM
	{
		NONE = -1,

		RETURN_GAME,	// ゲームに戻る
		RESTART,		// 再出撃
		RETURN_TITLE,	// 退却
		CONFIG,			// 設定
		MANUAL,			// 操作説明表示
		//CONFIG,		// 設定

		NUM
	};




	/// <summary>
	/// アイテムの種類
	/// </summary>
	enum class ITEM_TYPE
	{
		RECOVERY_SMALL,	// 回復 - 小 15%
		RECOVERY_LARGE,	// 回復 - 大 30%
		ARMOR,			// アーマー
		WEAPON,			// 武器箱

		NUM
	};


	/// <summary>
	/// 移動挙動の種類
	/// </summary>
	enum class MOVE_BEHAVIOUR_TYPE
	{
		NONE = -1,

		LINEAR,     // 直線移動
		HOMING,     // ホーミング移動

		NUM
	};


	/// <summary>
	/// 共用体の値の種類
	/// </summary>
	enum class VALUE_TYPE
	{
		BOOL,
		INT,
		FLOAT,
	};


	struct FactionInfo
	{
		FACTION _faction;
	};

	/// <summary>
	/// メニューの項目
	/// </summary>
	struct MenuItemInfo
	{
		VECTOR2::VEC2 _pos;	// 項目の位置
		const char* _name;	// 項目名
		bool _isHovered;	// マウスが上に乗っているか

		MenuItemInfo() :
			_pos(VECTOR2::VEC2()),
			_name(),
			_isHovered(false)
		{
		};

		MenuItemInfo(VECTOR2::VEC2 pos, const char* name)
			: _pos(pos), _name(name), _isHovered(false) {
		}
	};

	/// <summary>
	/// メインメニュー項目
	/// </summary>
	struct MainMenuItemInfo : public MenuItemInfo
	{
		TITLEMENU_ITEM _type;
		SceneStateEnums::c_TITLE _nextState;// 選択した際の遷移先ステート
		
		MainMenuItemInfo() :
			_type(UtilityData::TITLEMENU_ITEM::NUM),
			_nextState(SceneStateEnums::c_TITLE::c_TITLE_MAIN_MENU)
		{
		};
	};

	/// <summary>
	/// 設定メニュー項目
	/// </summary>
	struct ConfigMenuItemInfo : public MenuItemInfo
	{
		CONFIG_ITEM _type;			// 設定項目の種類（BGM音量、SE音量、マウス感度、カメラのY反転の有無、シャドウの有無）
		VALUE_TYPE _valueType;		// 値の種類（整数、浮動小数点数、真偽値）
		std::variant<bool, int, float> _value;	// 設定の値（値の種類に応じてbool、int、floatのいずれかを使用）


		ConfigMenuItemInfo() :
			_type(UtilityData::CONFIG_ITEM::NUM),
			_valueType(VALUE_TYPE::INT)
		{
		};

		// 全ての値を設定するためのコンストラクタ（追加）
		ConfigMenuItemInfo(VECTOR2::VEC2 pos, const char* name, CONFIG_ITEM type, VALUE_TYPE vType, std::variant<bool, int, float> val)
			: MenuItemInfo(pos, name), // 親クラスを初期化
			_type(type),
			_valueType(vType),
			_value(val) {
		}

		/// <summary>
		/// 値を設定する
		/// </summary>
		/// <param name="val"></param>
		void SetValue(std::variant<bool, int, float> val)
		{
			_value = val;
		}

		/// <summary>
		/// 共用体の値を文字列として表示できるようにする
		/// </summary>
		/// <returns></returns>
		std::string ConvertValueAsString() const
		{
			switch (_valueType)
			{
			case VALUE_TYPE::BOOL:
				return std::get<bool>(_value) ? "ON" : "OFF";
			case VALUE_TYPE::INT:
				return std::to_string(std::get<int>(_value));
			case VALUE_TYPE::FLOAT:
				return Tool::WStringToString(Tool::FormatFloat(std::get<float>(_value)));
			default:
				return "";
			}
		}
	};



	constexpr int MAX_BGM_VOLUME = 100;			// BGM音量の最大値
	constexpr int MIN_BGM_VOLUME = 0;			// BGM音量の最小値
	constexpr int MAX_SE_VOLUME = 100;			// SE音量の最大値
	constexpr int MIN_SE_VOLUME = 0;			// SE音量の最小値
	constexpr float MAX_MOUSE_SENSITIVITY = 100.0f;	// マウス感度の最大値
	constexpr float MIN_MOUSE_SENSITIVITY = 0.0f;	// マウス感度の最小値

	/// <summary>
	/// ユーザーの設定データ
	/// </summary>
	struct UserConfigData
	{
		int _BGMVolume;				// BGMの音量 : 100.0が最大（実際はさらにスケールがかけられ、小さくなる）
		int _SEVolume;				// SEの音量 : 100.0が最大（実際はさらにスケールがかけられ、小さくなる）
		float _mouseSensitivity;	// マウス感度 : 100.0が最大（実際はさらにスケールがかけられ、小さくなる）
		bool _isInvertY;			// Y軸反転の有無
		bool _isCameraShake;		// カメラシェイクの有無
		bool _isShadowEnabled;		// シャドウの有効/無効

		UserConfigData() :
			_BGMVolume(50),
			_SEVolume(70),
			_mouseSensitivity(40.0f), 
			_isInvertY(false),
			_isCameraShake(true),
			_isShadowEnabled(true)
		{
		}
	};

	/// <summary>
	/// カメラシェイク用のパラメータ
	/// </summary>
	struct CameraShakeParam
	{
		float _duration;			// 揺れの長さ
		VECTOR3::VEC3 _strength;	// 揺れの強さ（X,Y,Z軸）
		VECTOR3::VEC3 _pos;			// 揺れの中心位置
		float _maxRange;			// 揺れの最大範囲（この範囲を超える揺れは発生しない）
		CameraShakeParam() :
			_duration(0.0f),
			_strength(VECTOR3::VEC3()),
			_pos(VECTOR3::VEC3()),
			_maxRange(0.0f)
		{
		}
	};

	/// <summary>
	/// 爆発コマンドのパラメータ
	/// </summary>
	struct Command_ExplosionParam
	{
		float _scale = 1.0f;	// エフェクトの大きさ
		VECTOR3::VEC3 _pos;		// エフェクトの位置
		VECTOR3::VEC3 _rot;		// エフェクトの回転（ラジアン）

		Command_ExplosionParam() :
			_scale(1.0f),
			_pos(VECTOR3::VEC3()),
			_rot(VECTOR3::VEC3())
		{
		}
	};

	/// <summary>
	/// ステージの環境パラメータ
	/// </summary>
	struct StageEnvironmentParam
	{
		VECTOR3::VEC3 dirLightColor;		// ディレクションライトカラー（スカイボックスの色になる）
		VECTOR3::VEC3 dirLightDirection;	// ディレクションライトの向き
		float dirLightIntensity;			// ディレクションライトインテンシティ
		VECTOR3::VEC3 fogColor;			// フォグカラー
		float fogStart;					// フォグの開始距離
		float fogEnd;						// フォグの最大距離（1.0以下ならフォグを設定しない）
		float dofStart;					// 被写界深度の開始距離
		float dofEnd;						// 被写界深度の最大距離

		StageEnvironmentParam() :
			dirLightColor(VECTOR3::VEC3(1.0f)),
			dirLightDirection(VECTOR3::VEC3(0.0f,1.0f,0.0f)),
			dirLightIntensity(2.0f),
			fogColor(VECTOR3::VEC3(1.0f)),
			dofStart(0.0f),
			dofEnd(0.0f),
			fogStart(0.0f),
			fogEnd(0.0f)

		{
		}
	};

	/// <summary>
	/// 条件の分岐
	/// </summary>
	enum class ConditionOperator
	{
        AND = 0,
        OR = 1
    };

	/// <summary>
	/// エネミーの出現位置のタイプ
	/// </summary>
	enum class ENEMY_SPAWN_POS_TYPE
	{
		RANDOM = 0,		// ランダムに出現
		POINT  = 1,		// 指定したポイントに出現
	};

	/// <summary>
	/// プレイヤーの出現情報
	/// </summary>
	struct SpawnPlayerData
	{
		VECTOR3::VEC3 _playerStartPos;				// プレイヤーの初期位置
		VECTOR3::VEC3 _playerStartRot;				// プレイヤーの初期回転角度
	};

	/// <summary>
	/// 出現するエネミーの情報
	/// </summary>
	struct SpawnEnemyData
	{
		std::string _enemyType;						// 出現する敵の種類
		int _enemyNum;								// 出現する敵の数
		float _healthScale;							// HPスケール
		ENEMY_SPAWN_POS_TYPE _spawnPosType;			// 出現位置のタイプ
		float _spawnRadius;							// 出現位置の半径（ランダム出現時に使用）
		VECTOR3::VEC3 _spawnPos;					// 出現位置
		VECTOR3::VEC3 _spawnRot;					// 出現時の回転角度
		bool _isAggro;								// 出現時の状態（アクティブ or 非アクティブ）
	};

	/// <summary>
	/// ウェーブごとのデータ
	/// </summary>
	struct WaveData
	{
		int _waveID;								// ウェーブID
		float _spawnDelay;							// ウェーブ開始から敵が出現するまでの遅延時間
		std::vector<SpawnEnemyData> _enemysList;	// 出現する敵のリスト
	};

	/// <summary>
	/// ミッションの情報
	/// </summary>
	struct MissionData
	{
		std::string _missionName;						// ミッション名
		std::string _missionDescription;				// ミッションの説明
		std::string _texturePath;						// ミッション用画像パス
		int _missionID;									// ミッションID
		StageEnvironmentParam _stageEnvironmentParam;	// ステージ環境パラメータ
		SpawnPlayerData _spawnPlayerData;				// プレイヤーの出現情報
		int _waveNum;									// ウェーブ数
		std::vector<WaveData>_waveDataList;				// ウェーブごとのデータ
	};

	/// <summary>
	/// タイトル項目名
	/// </summary>
	static const char* g_TitleMenuItemNames[static_cast<int>(TITLEMENU_ITEM::NUM)] =
	{
		"ミッション選択",
		//"兵科選択",
		"兵装選択",
		"設定",
		"終了",
	};

	/// <summary>
	/// ポーズ項目名
	/// </summary>
	static const char* g_PauseItemNames[static_cast<int>(PAUSE_ITEM::NUM)] =
	{
		"ゲームに戻る",
		"再出撃",
		"退却",
		"設定",
		"操作マニュアル",
		//"設定",
	};

	/// <summary>
	/// 設定の項目名
	/// </summary>
	static const char* g_ConfigItemNames[static_cast<int>(CONFIG_ITEM::NUM)] =
	{
		"BGM音量",
		"SE音量",
		"マウス感度",
		"カメラの上下反転",
		"画面振動",
		"シャドウの有無",
	};

	/// <summary>
	/// 兵科名
	/// </summary>
	static const char* g_SoldierNames[static_cast<int>(SOLDIER_TYPE::NUM)] =
	{
		//"陸戦歩兵",
		//"...",
		//"...",
		//"...",

		"スタンダード",
		"ラピッド",
		"スカウト",
		"ヘビー",
	};

	/// <summary>
	/// 兵装説明
	/// </summary>
	static const char* g_WeaponDescriptions[static_cast<int>(SOLDIER_TYPE::NUM)] =
	{
		"・バランスの取れた兵装\n特にクセもなく、初心者におすすめだ。",
		"・連射性能に優れた兵装\n威力は低めだが、弾数、連射性能に優れているぞ。",
		"・中/遠距離に特化した兵装\n連射性能は抑え目だが、中距離以上からの射撃に長けているぞ。",
		"・高火力兵装\n扱いは難しいが、火力は群を抜いて高い。まさにロマン砲！",
	};

	/// <summary>
	/// 難易度名
	/// </summary>
	static const char* g_DifficultyNames[static_cast<int>(DIFFICULTY_LEVEL::NUM)] =
	{
		"EASY",
		"NORMAL",
		"HARD",
		"DISASTER",
		//"APOCALYPSE",
		"IMPOSSIBLE",
	};

	/// <summary>
	/// 難易度説明
	/// </summary>
	static const char* g_DifficultyDescriptions[static_cast<int>(DIFFICULTY_LEVEL::NUM)] =
	{
		"初心者におすすめの難易度です。\n敵も接待してくれます。",
		"標準的な難易度です。\nまずはこの難易度でプレイされることを推奨します。",
		"かなり難しくなっています。\n一体ずつ確実に倒しましょう。",
		"敵が全力で襲い掛かってきます。\n一切の容赦はありません。",
		"...SystemError\nプレイは推奨されません。",
	};
};