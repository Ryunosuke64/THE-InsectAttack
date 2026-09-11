#pragma once
#include "EnemyFactory.h"
#include "ConstantEnemyData.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BuildingManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】建物の管理
//
// ***************************************************************************************
class EnemyManager
{
private:
    std::vector<EnemyData::EnemySlot> m_Enemies;                        // 全てのエネミーがここに登録される
    std::unordered_map<                                                 // グループ
        EnemyData::EnemyGroupID, EnemyData::EnemyGroup> m_Groups;
    EnemyData::EnemyGroupID m_NextGroupID = 0;                          // 次のグループ番号

public:
    EnemyManager();
    ~EnemyManager();

    EnemyData::EnemyID RegisterEnemy(std::weak_ptr<GameObject> pEnemy);                                     // エネミー登録
    void UnregisterEnemy(EnemyData::EnemyID id);                                                            // エネミー登録解除
    EnemyData::EnemyGroupID RegisterEnemyGroup(std::vector<std::weak_ptr<GameObject>> pEnemies);            // グループ登録
    void UnregisterEnemyGroup(EnemyData::EnemyGroupID id);                                                  // グループ登録解除
    int GetAllEnemyCount()const;                                                                            // 全エネミー数取得
    int GetGroupAliveCount(EnemyData::EnemyGroupID group)const;                                             // グループ内エネミーの生存数
    bool IsDead(uint32_t id);                                                                               // 指定IDのエネミーが倒されたか
    bool IsAllDead()const;                                                                                  // 全てのエネミーが倒されたか
    std::weak_ptr<GameObject> FindNearestEnemy(const VECTOR3::VEC3& _pos);                                  // 指定位置から近いエネミーを取得 
    std::weak_ptr<GameObject> FindEnemiesInRange(  const VECTOR3::VEC3& _pos, float radius);                // 指定位置を中心とした半径内のエネミーを取得
    void AllKill();                                                                                         // 全てのエネミーを倒す
    void SetAllActive(bool active);                                                                         // 全てのエネミーのアクティブ状態変更
    bool IsGroupDestroyed(EnemyData::EnemyGroupID group)const;                                              // グループが消滅しているか


private:
    // コピー禁止
    EnemyManager(const EnemyManager&) = delete;
    EnemyManager& operator=(const EnemyManager&) = delete;

    bool IsValidEnemy(const EnemyData::EnemyID& enemyID)const;
};
