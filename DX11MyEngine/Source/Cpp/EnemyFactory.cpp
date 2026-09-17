//* =========================================================================
//* - @:インクルード - */
//* =========================================================================
#include "pch.h"
#include "EnemyFactory.h"
#include "RendererEngine.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "MeshFactory.h"
#include "Component_BoxCollider.h"
#include "Component_SphereCollider.h"
#include "Component_EnemyController.h"
#include "Component_Health.h"
#include "Component_LineRenderer.h"
#include "Component_WeaponController.h"
#include "Component_BuildingController.h"
#include "Component_MoveLogic.h"
#include "Component_Faction.h"
#include "Component_Physics.h"
#include "Component_SkinnedMeshAnimator.h"


using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;

using namespace UtilityData;
using namespace EnemyData;

constexpr float ANT_THRESHOLD_RATE = 0.05f;         // 最大HPの5%
constexpr float OCTAHEDRON_THRESHOLD_RATE = 0.30f;  // 最大HPの30%

//*---------------------------------------------------------------------------------------
//*【?】エネミーの生成
//* [引数]
//* setupData : セットアップ用データ
//*
//* [返値]
//* 生成したエネミーのID
//*----------------------------------------------------------------------------------------
uint32_t EnemyFactory::SpawnEnemy(const EnemyData::EnemySpawnData& spawnData)
{
    std::shared_ptr<GameObject>generatedObject;
    EnemyGenerationData generationData;
    generationData.isAggro = spawnData.isAggro;
    generationData.position = spawnData.position;
    generationData.rotation = spawnData.rotation;
    generationData.hp = spawnData.hp;

    // 指定タイプのエネミーを生成
    switch (spawnData.enemyType)
    {
    //======================================
    // アリ
    //======================================
    case ENEMY_TYPE::GIANT_ANT_Normal:
    {
        generationData.createStateID = (int)ENEMY_TYPE::GIANT_ANT_Normal;
        generationData.startStateID = ANT_STATE::ANT_STATE_PATROL_IDLE;
        generatedObject = CreateAnt01(generationData);
        break;
    }
    //======================================
    // 八面体
    //======================================
    case ENEMY_TYPE::OCTAHEDRON:
    {
        generationData.createStateID = (int)ENEMY_TYPE::OCTAHEDRON;
        generationData.startStateID = OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_IDLE;
        generatedObject = CreateOctahedron(generationData);
        break;
    }
    default:
        break;
    };

    // エネミーマネージャーに登録し、IDを取得
    EnemyID id = Master::m_pEnemyManager->RegisterEnemy(generatedObject);

    // インデックスを外部に渡す
    return id.index;
}

//*---------------------------------------------------------------------------------------
//*【?】エネミーグループの生成
//* [引数]
//* setupData : セットアップ用データ
//*
//* [返値]
//* 生成したグループのID
//*----------------------------------------------------------------------------------------
uint32_t EnemyFactory::SpawnEnemyGroup(const EnemyData::EnemyGroupSpawnData& spawnData)
{
    std::shared_ptr<GameObject>generatedObject;
    EnemyGenerationData generationData;
    generationData.isAggro = spawnData.isAggro;
    generationData.hp = spawnData.hp;
    std::vector<std::weak_ptr<GameObject>>enemies;  // マネージャーに渡す用

    
    for (int i = 0; i < spawnData.count; i++)
    {
        // ランダムな位置を求める
        generationData.position = 
            spawnData.position + Tool::RandRange(
                -spawnData.spawnRadius, spawnData.spawnRadius
        );
        generationData.rotation = spawnData.rotation;

        // 指定タイプのエネミーを生成
        switch (spawnData.enemyType)
        {
            //======================================
            // アリ
            //======================================
        case ENEMY_TYPE::GIANT_ANT_Normal:
        {
            generationData.createStateID = (int)ENEMY_TYPE::GIANT_ANT_Normal;

            // 攻撃状態かどうかで開始時のステートを決める
            generationData.startStateID = 
                spawnData.isAggro ? ANT_STATE ::ANT_STATE_ACTIVE_MOVE : ANT_STATE::ANT_STATE_PATROL_IDLE;
            
            // 生成
            generatedObject = CreateAnt01(generationData);
            break;
        }
        //======================================
        // 八面体
        //======================================
        case ENEMY_TYPE::OCTAHEDRON:
        {
            generationData.createStateID = (int)ENEMY_TYPE::OCTAHEDRON;

            // 攻撃状態かどうかで開始時のステートを決める
            generationData.startStateID =
                spawnData.isAggro ? OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_IDLE : OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_IDLE;
            
            // 生成
            generatedObject = CreateOctahedron(generationData);
            break;
        }
        default:
            break;
        };

        // 配列に追加
        enemies.push_back(generatedObject);
    }

    // エネミーマネージャーにグループを登録し、IDを取得
    EnemyGroupID id = Master::m_pEnemyManager->RegisterEnemyGroup(enemies);

    return id;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						敵ごとの生成メソッド
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//=========================================================================================
//						アリ - 01
//=========================================================================================
std::shared_ptr<GameObject> EnemyFactory::CreateAnt01(const EnemyGenerationData& generationData)
{
    std::shared_ptr<GameObject>generatedObject;
    auto renderer = Master::m_pDataManager->get_RendererEngine();
    VEC3 pos = generationData.position;
    VEC3 rot = generationData.rotation;


    // マテリアル取得
    auto matPtr = Master::m_pResourceManager->FindMaterial("Ant");
    SetupMaterialInfo matInfo[1];
    matInfo[0].Index = 0;
    matInfo[0].pMaterialData = matPtr;
    CreateModelInfo model;
    model.pRenderer = renderer;
    model.ObjTag = "Ant";
    model.IsAnim = true;
    model.MatNum = 1;
    model.SetupMaterial = matInfo;
    model.ShaderType = SHADER_TYPE::DEFERRED_STD_SKINNED_N;

    // LOD
    model.LODModels[0] = {"Resource/Model/Enemy/GiantAnt01/GiantAnt_LOD0.fbx", 0.0f };
    model.LODModels[1] = {"Resource/Model/Enemy/GiantAnt01/GiantAnt_LOD1.fbx", 100.0f};
    model.LODModels[2] = {"Resource/Model/Enemy/GiantAnt01/GiantAnt_LOD2.fbx", 150.0f};

    generatedObject = MeshFactory::CreateModel(model);

    // 動的オブジェクト
    generatedObject->set_IsStatic(false);

    // ポーズ中は停止
    generatedObject->set_IsUpdateAllowedDuringPause(false);

    // アニメーター
    generatedObject->get_Component<SkinnedMeshAnimator>()->set_IsAnim(true);
    generatedObject->get_Component<SkinnedMeshAnimator>()->set_AnimIndex(0);

    // 
    // トランスフォーム設定
    //
    auto transform = generatedObject->get_Component<MyTransform>();
    transform->set_Pos(pos);
    transform->set_RotateToRad(rot);
    transform->set_Scale(1.0f);

    
    //
    // エネミーコントローラー追加
    //
    auto enemyController = generatedObject->add_Component<EnemyController>();

    //
    // 移動コンポーネントの追加
    //
    generatedObject->add_Component<MoveLogic>();
    
    //
    // 派閥コンポーネント追加
    //
    auto faction = generatedObject->add_Component<Faction>();
    faction->set_Faction(FACTION::ENEMY);
    
    //
    // 体力コンポーネント追加
    //
    auto health = generatedObject->add_Component<Health>();
    float hp = generationData.hp * Master::m_pDataManager->get_EnemyDifficultyFactor()._hpRate;
    health->set_MaxHP(hp);
    health->set_CrntHP(hp);

    // コントローラー側に怯み耐久を設定する
    enemyController->set_StaggerThreshold(hp * ANT_THRESHOLD_RATE);

    //
    // 物理コンポーネント追加
    //
    auto physics = generatedObject->add_Component<Physics>();
    physics->set_AirDrag(1.0f);
    physics->set_Restitution(0.5f); // 跳ねない
    physics->set_AngularDrag(0.98f);
    //
    // コライダーの追加
    //
    auto collider = generatedObject->add_Component<BoxCollider>();
    collider->set_Size(VEC3(2.0f, 2.0f, 2.0f));
    collider->set_Center(VEC3(0.0f, 2.0f, 0.0f));
    // 衝突カテゴリ
    collider->set_CollisionCategory(COLLISION_CATEGORY::ENEMY);
    // 衝突マスクの設定
    collider->set_CollisionResponse(COLLISION_CATEGORY::BUILDING,               COLLISION_RESPONSE::RESPONSE_BLOCK);          // 建物
    collider->set_CollisionResponse(COLLISION_CATEGORY::DESTRUCTION_BUILDING,   COLLISION_RESPONSE::RESPONSE_BLOCK);          // 破壊可能建物
    collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY,                  COLLISION_RESPONSE::RESPONSE_IGNORE);         // エネミー
    collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY_BULLET,           COLLISION_RESPONSE::RESPONSE_IGNORE);         // エネミー弾
    collider->set_CollisionResponse(COLLISION_CATEGORY::ITEM,                   COLLISION_RESPONSE::RESPONSE_IGNORE);         // アイテム
    collider->set_CollisionResponse(COLLISION_CATEGORY::PLAYER_BULLET,          COLLISION_RESPONSE::RESPONSE_OVERLAP);        // アイテム
    // コライダーの登録
    Master::m_pCollisionManager->RegisterCollider(collider);
    
    //
    // ステートの登録
    //
    enemyController->Start(*renderer);
    StateMachine<EnemyController> stateMachine_Ant(enemyController.get());
    EnemyStateFactory::Create(stateMachine_Ant, generationData.createStateID, *renderer);
    stateMachine_Ant.SetStartState(generationData.startStateID);
    // 登録
    enemyController->RegisterStateMachine(stateMachine_Ant);


    // 生成したエネミーオブジェクトを返す
    return generatedObject;
}

//=========================================================================================
//						八面体
//=========================================================================================
std::shared_ptr<GameObject> EnemyFactory::CreateOctahedron(const EnemyGenerationData& generationData)
{
    auto renderer = Master::m_pDataManager->get_RendererEngine();
    std::shared_ptr<GameObject>generatedObject;
    VEC3 pos = generationData.position;
    VEC3 rot = generationData.rotation;

    // マテリアル取得
    auto matPtr1 = Master::m_pResourceManager->FindMaterial("Objector_body");
    auto matPtr2 = Master::m_pResourceManager->FindMaterial("Objector_shield");

    SetupMaterialInfo matInfo[2];
    matInfo[0].Index = 0;
    matInfo[0].pMaterialData = matPtr1;
    matInfo[1].Index = 1;
    matInfo[1].pMaterialData = matPtr2;

    CreateModelInfo model;
    model.pRenderer = renderer;
    model.LODModels[0] = { "Resource/Model/Enemy/Octahedron/Octahedron.fbx", 0.0f };
    model.ObjTag = "Octahedron";
    model.IsAnim = false;
    model.MatNum = 2;
    model.IsActive = true;
    model.SetupMaterial = matInfo;
    model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;


    generatedObject = MeshFactory::CreateModel(model);
    generatedObject->set_IsStatic(false);

    // 動的オブジェクト
    generatedObject->set_IsStatic(false);

    // ポーズ中は停止
    generatedObject->set_IsUpdateAllowedDuringPause(false);
    
    // 
    // トランスフォーム設定
    //
    auto transform = generatedObject->get_Component<MyTransform>();
    transform->set_Pos(pos);
    transform->set_RotateToRad(rot);
    transform->set_Scale(1.0f);


    //
    // エネミーコントローラー追加
    //
    auto enemyController = generatedObject->add_Component<EnemyController>();

    //
    // 移動コンポーネントの追加
    //
    generatedObject->add_Component<MoveLogic>();

    //
    // 派閥コンポーネント追加
    //
    auto faction = generatedObject->add_Component<Faction>();
    faction->set_Faction(FACTION::ENEMY);

    //
    // 体力コンポーネント追加
    //
    auto health = generatedObject->add_Component<Health>();
    float hp = generationData.hp * Master::m_pDataManager->get_EnemyDifficultyFactor()._hpRate;
    health->set_MaxHP(hp);
    health->set_CrntHP(hp);

    // コントローラー側に怯み耐久を設定する
    enemyController->set_StaggerThreshold(hp * OCTAHEDRON_THRESHOLD_RATE);

    //
    // 物理コンポーネント追加
    //
    auto physics = generatedObject->add_Component<Physics>();
    physics->set_Restitution(0.0f);     // 跳ねない
    physics->set_GravityScale(0.0f);    // 無重力  
    physics->set_Mass(1000.0f);         // 重量
    physics->set_MoveDrag(1.0f);        // 移動抵抗
    physics->set_AirDrag(1.0f);         // 空中抵抗

    //
    // コライダーの追加
    //
    auto collider = generatedObject->add_Component<BoxCollider>();
    collider->set_Size(VEC3(20.0f, 20.0f, 20.0f));
    collider->set_Center(VEC3(0.0f, 10.0f, 0.0f));
    // 衝突カテゴリ
    collider->set_CollisionCategory(COLLISION_CATEGORY::ENEMY);

    // 衝突マスクの設定
    collider->set_CollisionResponse(COLLISION_CATEGORY::BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);              // 建物
    collider->set_CollisionResponse(COLLISION_CATEGORY::DESTRUCTION_BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);  // 破壊可能建物
    collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY, COLLISION_RESPONSE::RESPONSE_IGNORE);                // エネミー
    collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY_BULLET, COLLISION_RESPONSE::RESPONSE_IGNORE);         // エネミー弾
    collider->set_CollisionResponse(COLLISION_CATEGORY::ITEM, COLLISION_RESPONSE::RESPONSE_IGNORE);                 // アイテム

    // コライダーの登録
    Master::m_pCollisionManager->RegisterCollider(collider);

    //
    // ステートの登録
    //
    enemyController->Start(*renderer);
    StateMachine<EnemyController> stateMachine_Ant(enemyController.get());
    EnemyStateFactory::Create(stateMachine_Ant, generationData.createStateID, *renderer);
    stateMachine_Ant.SetStartState(generationData.startStateID);
    // 登録
    enemyController->RegisterStateMachine(stateMachine_Ant);

    return generatedObject;
}