#pragma once

namespace EnemyData
{
	/// <summary>
	/// 敵の種類
	/// </summary>
	enum class ENEMY_TYPE
	{
		GIANT_ANT_Normal,	// アリ
		OCTAHEDRON,			// 八面体
		DRONE_TYPE_A,		// ドローンタイプA

		NUM,
	};


	/*
	* 
	* PT : 巡回中
	* AT : 戦闘態勢
	* 
	*/

	/// <summary>
	/// アリのステート
	/// </summary>
	enum ANT_STATE
	{
		// 非アクティブ
		ANT_STATE_PATROL_IDLE,	// 待機
		ANT_STATE_PATROL_MOVE,	// 移動
		

		// アクティブ
		ANT_STATE_ACTIVE_MOVE,			// 移動
		ANT_STATE_ACTIVE_TRACKING,		// 追従
		ANT_STATE_ACTIVE_ATTACK_BITE,	// 噛みつき攻撃
		ANT_STATE_ACTIVE_ATTACK_ACID,	// 酸攻撃
		ANT_STATE_ACTIVE_DAMAGED,		// ダメージ
		ANT_STATE_ACTIVE_BLOWN_AWAY,	// 爆発による吹き飛び
		ANT_STATE_ACTIVE_FALLING,		// 落下中
		ANT_STATE_ACTIVE_HIT_STUN,		// 被弾
		ANT_STATE_ACTIVE_DEAD,			// 死亡
	};

	/// <summary>
	/// 八面体のステート
	/// </summary>
	enum OCTAHEDRON_STATE
	{
		// 非アクティブ

		// アクティブ
		OCTAHEDRON_STATE_ACTIVE_MOVE,			// 移動
		OCTAHEDRON_STATE_ACTIVE_TRACKING,		// 移動
		OCTAHEDRON_STATE_ACTIVE_IDLE,			// 待機
		OCTAHEDRON_STATE_ACTIVE_ATTACK_LASER01,	// レーザー攻撃
		OCTAHEDRON_STATE_ACTIVE_HIT_STUN,		// 被弾
		OCTAHEDRON_STATE_ACTIVE_DEAD,			// 死亡
	};
	
	/// <summary>
	/// ドローン タイプAのステート
	/// </summary>
	enum DRONE_TYPE_A_STATE
	{
		// 非アクティブ

		// アクティブ
		DRONE_TYPE_A_STATE_ACTIVE_MOVE,				// 移動
		DRONE_TYPE_A_STATE_ACTIVE_TRACKING,			// 追跡移動
		DRONE_TYPE_A_STATE_ACTIVE_IDLE,				// 待機
		DRONE_TYPE_A_STATE_ACTIVE_ATTACK,			// 攻撃
		DRONE_TYPE_A_STATE_ACTIVE_HIT_STUN,			// 被弾
		DRONE_TYPE_A_STATE_ACTIVE_DEAD,				// 死亡
	};

	inline const char* g_AntStateNames[] =
	{
		"PATROL_IDLE",
		"PATROL_MOVE",
		"ACTIVE_MOVE",
		"ACTIVE_TRACKING",
		"ACTIVE_ATTACK_BITE",
		"ACTIVE_ATTACK_ACID",
		"ACTIVE_DAMAGED",
		"ACTIVE_BLOWN_AWAY",
		"ACTIVE_FALLING",
		"ACTIVE_HIT_STUN",
		"ACTIVE_DEAD",
	};

	
	using EnemyGroupID = uint32_t;  // グループ用ID
	
	// エネミーID
	struct EnemyID
	{
	    uint32_t index = UINT32_MAX;
	    uint32_t generation = 0;        // 古いEnemyIDと区別するために使う
	};
	
	// エネミースロット
	struct EnemySlot
	{
	    std::weak_ptr<GameObject> enemy;
	
	    uint32_t generation = 0;
	    bool active = false;
	};
	
	// エネミーグループ
	struct EnemyGroup
	{
	    EnemyGroupID id;
	    std::vector<EnemyID> enemies;   // グループに所属するエネミーのIDを持つ（m_Enemiesのインデックス）
	};
	
	// エネミー出現時のセットアップデータ
	struct EnemySpawnData
	{
	    EnemyData::ENEMY_TYPE enemyType;    // エネミーの種類
	    VECTOR3::VEC3 position;     // 位置
	    VECTOR3::VEC3 rotation;     // 回転
		float hp = 0.0f;			// HP
		bool isAggro = false;       // 攻撃状態か
	};	

	// エネミーグループ出現時のセットアップデータ
	struct EnemyGroupSpawnData
	{
		EnemyData::ENEMY_TYPE enemyType;    // エネミーの種類
		VECTOR3::VEC3 position;             // 位置
		VECTOR3::VEC3 rotation;             // 回転
		float hp = 0.0f;					// HP
		float spawnRadius = 0.0f;           // 出現範囲
		int count = 0;                      // 出現数
		bool isAggro = 0.0f;                // 攻撃状態か
	};

	const float ENEMY_ANT01_BASE_HP = 200.0f;	// アリの基本HP
	const float ENEMY_OCTAHEDRON_BASE_HP = 1000.0f;	// 八面体の基本HP

	/// <summary>
	/// 敵の基本データ
	/// </summary>
	struct BaseEnemyData
	{
		ENEMY_TYPE _enmyType = ENEMY_TYPE::GIANT_ANT_Normal;

		float _hp = 0.0f;
		float _attack = 0.0f;
		float _moveSpeed = 0.0f;
	};


	/// <summary>
	/// 難易度係数
	/// </summary>
	struct EnemyDifficultyFactor
	{
		float _hpRate = 0.0f;
		float _attackRate = 0.0f;
		float _moveSpeedRate = 0.0f;
	};
}