#pragma once
#include "ConstantVehicleData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:VehicleFactory Class --- */
//
// 【?】ビークルの生成を行う
//
// ***************************************************************************************
class VehicleFactory
{
public:
    // 生成データ
    struct VehicleGenerationData
    {
        std::string materialTag;    // マテリアルタグ
        VECTOR3::VEC3 position;     // 位置
        VECTOR3::VEC3 rotation;     // 回転
        float hp = 0.0f;            // HP
        int createStateID = -1;     // 生成するステートID
        int startStateID = -1;      // 開始時のステートID
    };

	static bool SpawnVehicle(const VehicleData::VehicleSpawnData& spawnData);

private:
	static std::shared_ptr<GameObject> CreateDurga(const VehicleGenerationData& generationData);

};

