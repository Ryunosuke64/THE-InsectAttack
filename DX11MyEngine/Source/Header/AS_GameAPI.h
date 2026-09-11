#pragma once

//=========================================================================================
//
//						AngelScript内で使用するゲームAPI群
//
//=========================================================================================
namespace GIGA_Engine::ScriptAPI::Game
{
	// ミッション開始からの時間を取得
	float GetMissionTime();

	// ステージ環境の設定
	void SetStageEnvironmentParam(const UtilityData::StageEnvironmentParam& param);

	// エネミーの出現（単体）
	// タイプ・位置・回転・HP・攻撃状態か
	uint32_t SpawnEnemy(EnemyData::ENEMY_TYPE type,const VECTOR3::VEC3& pos, const VECTOR3::VEC3& rot, float hp, bool isAggro);

	// エネミーグループ出現
	// タイプ・位置・出現範囲・出現数・HP・攻撃状態か
	uint32_t SpawnEnemyGroup(EnemyData::ENEMY_TYPE type, const VECTOR3::VEC3& pos, float spawnRadius, u_int count, float hp, bool isAggro);

	// 指定IDのエネミーが倒されたか
	bool IsEnemyDead(uint32_t enemyID);

	// 指定IDのエネミーグループがすべて倒されたか
	bool IsEnemyGroupDestroyed(uint32_t groupID);

	// 指定IDのエネミーグループの生存数を取得
	int GetEnemyGroupAliveCount(uint32_t groupID);

	// エネミーの生存数を取得
	int GetAliveEnemyCount();

	// プレイヤーが指定エリア内に入ったか
	bool IsPlayerInArea(const VECTOR3::VEC3& pos, float radius);

	// イベント：空爆
	void EventAirstrike();

	// 無線の再生
	void PlayMessage(const std::string& msg);


};