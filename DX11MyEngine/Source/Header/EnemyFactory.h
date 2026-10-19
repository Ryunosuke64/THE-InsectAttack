#pragma once


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:EnemyFactory Class --- */
//
// 【?】エネミーの生成を行う
//
// ***************************************************************************************
class EnemyFactory
{
public:
    // エネミーの生成データ
    struct EnemyGenerationData
    {
        std::string materialTag;    // マテリアルタグ
        VECTOR3::VEC3 position;     // 位置
        VECTOR3::VEC3 rotation;     // 回転
        float hp = 0.0f;            // HP
        int createStateID = -1;     // 生成するステートID
        int startStateID = -1;      // 開始時のステートID
        bool isAggro = false;       // 攻撃状態か
    };

    static uint32_t SpawnEnemy(const EnemyData::EnemySpawnData& spawnData);
    static uint32_t SpawnEnemyGroup(const EnemyData::EnemyGroupSpawnData& spawnData);

private:
    static std::shared_ptr<GameObject> CreateAnt01(const EnemyGenerationData& generationData);
    static std::shared_ptr<GameObject> CreateOctahedron(const EnemyGenerationData& generationData);
    static std::shared_ptr<GameObject> CreateDroneTypeA(const EnemyGenerationData& generationData);
};

