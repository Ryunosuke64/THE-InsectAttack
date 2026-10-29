//* =========================================================================
//* - @:インクルード - */
//* =========================================================================
#include "pch.h"
#include "VehicleFactory.h"
#include "RendererEngine.h"
#include "ResourceManager.h"
#include "VehicleStateFactory.h"
#include "GameObject.h"
#include "MeshFactory.h"
#include "Component_VehicleController.h"
#include "Component_Health.h"
#include "Component_WeaponController.h"
#include "Component_ModelMeshResource.h"
#include "Component_RigidBody.h"
#include "Component_Physics.h"
#include "Component_MoveLogic.h"
#include "Component_Faction.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_AI_GunWeapon.h"
#include "Component_3DCamera.h"

#include "Component_CapsuleCollider.h"
#include "Component_SphereCollider.h"
#include "Component_MeshCollider.h"
#include "Component_BoxCollider.h"

using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;

using namespace UtilityData;
using namespace VehicleData;

//*---------------------------------------------------------------------------------------
//*【?】乗り物の生成
//* [引数]
//* setupData : セットアップ用データ
//*
//* [返値]
//* 生成できたか
//*----------------------------------------------------------------------------------------
bool VehicleFactory::SpawnVehicle(const VehicleData::VehicleSpawnData& spawnData)
{
    std::shared_ptr<GameObject>generatedObject;
    VehicleGenerationData generationData;
    generationData.position = spawnData.position;
    generationData.rotation = spawnData.rotation;
    generationData.hp = spawnData.hp;

    // 指定タイプのビークルを生成
    switch (spawnData.type)
    {
        //======================================
        // ドルガ
        //======================================
    case VEHICLE_TYPE::DURGA:
    {
        generationData.createStateID = (int)VEHICLE_TYPE::DURGA;
        generationData.startStateID = ::DURGA_V_STATE::DURGA_V_STATE_MOVE;
        generatedObject = CreateDurga(generationData);
        break;
    }
    default:
        break;
    };

    return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						生成メソッド
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//=========================================================================================
//						ドルガ
//=========================================================================================
std::shared_ptr<GameObject> VehicleFactory::CreateDurga(const VehicleGenerationData& generationData)
{
    std::shared_ptr<GameObject>generatedObject;
    auto renderer = Master::m_pDataManager->get_RendererEngine();
    VEC3 pos = generationData.position;
    VEC3 rot = generationData.rotation;


    // マテリアル取得
    auto matPtr = Master::m_pResourceManager->FindMaterial("Durga");
    SetupMaterialInfo matInfo[1];
    matInfo[0].Index = 0;
    matInfo[0].pMaterialData = matPtr;
    CreateModelInfo model;
    model.pRenderer = renderer;
    model.ObjTag = "Durga";
    model.IsAnim = true;
    model.MatNum = 1;

    model.SetupMaterial = matInfo;
    model.ShaderType = SHADER_TYPE::DEFERRED_STD_SKINNED_N;

    // LOD
    model.LODModels[0] = { "Resource/Model/Vehicle/Durga/Durga.fbx", 0.0f };

    generatedObject = MeshFactory::CreateModel(model);

    // 動的オブジェクト
    generatedObject->set_IsStatic(false);

    // ポーズ中は停止
    generatedObject->set_IsUpdateAllowedDuringPause(false);

    // アニメーター
    generatedObject->get_Component<SkinnedMeshAnimator>()->set_IsAnim(true);
    generatedObject->get_Component<SkinnedMeshAnimator>()->set_AnimIndex(6);
    generatedObject->get_Component<SkinnedMeshAnimator>()->set_IsAnimationLODEnabled(false);

    // 
    // トランスフォーム設定
    //
    auto transform = generatedObject->get_Component<MyTransform>();
    transform->set_Pos(pos);
    transform->set_RotateToRad(rot);
    transform->set_Scale(1.0f);


    //
    // ビークル用コントローラー追加
    //
    auto vehicleController = generatedObject->add_Component<VehicleController>();
    vehicleController->set_AnimSpeed(1.0f);

    //
    // 移動コンポーネントの追加
    //
    generatedObject->add_Component<MoveLogic>();

    //
    // 派閥コンポーネント追加
    //
    auto faction = generatedObject->add_Component<Faction>();
    faction->set_Faction(FACTION::VEHICLE);

    //
    // 体力コンポーネント追加
    //
    auto health = generatedObject->add_Component<Health>();
    float hp = generationData.hp;
    health->set_MaxHP(hp);
    health->set_CrntHP(hp);

    //// コントローラー側に怯み耐久を設定する
    //enemyController->set_StaggerThreshold(hp * ANT_THRESHOLD_RATE);

    //enemyController->set_AlwaysSoundID("");

    auto modelResource = generatedObject->get_Component<ModelMeshResource>();

    auto rb = generatedObject->add_Component<RigidBody>();
    auto collider = generatedObject->add_Component<MeshCollider>();
    //auto collider = generatedObject->add_Component<BoxCollider>();
    //collider->set_Size(VEC3(15.0f, 20.0f, 10.0f));
    //collider->set_Center(VEC3(0.0f, 20.0f, 0.0f));
    collider->SetupModelData( modelResource->get_ModelData(LOD_QUALITY::ULTRA));
    collider->set_IsStatic(false);
    collider->set_IsConvex(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::BUILDING);// 衝突カテゴリ
    collider->set_SurfaceType(SURFACE_TYPE::METAL);             // 材質：メタル

    //*****************************************************************************************
    //						衝突マスクの設定
    //*****************************************************************************************
    // 
    // 押し出しあり
    //
    collider->set_CollisionResponse(COLLISION_CATEGORY::BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);              // 建物
    collider->set_CollisionResponse(COLLISION_CATEGORY::DESTRUCTION_BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);  // 破壊可能建物
    collider->set_CollisionResponse(COLLISION_CATEGORY::PLAYER, COLLISION_RESPONSE::RESPONSE_BLOCK);                // プレイヤー
    collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY, COLLISION_RESPONSE::RESPONSE_BLOCK);                // エネミー
    collider->set_CollisionResponse(COLLISION_CATEGORY::PROP, COLLISION_RESPONSE::RESPONSE_BLOCK);                // エネミー

    //
    // 判定なし
    //
    //collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY_BULLET, COLLISION_RESPONSE::RESPONSE_IGNORE);         // エネミー弾
    collider->set_CollisionResponse(COLLISION_CATEGORY::ITEM, COLLISION_RESPONSE::RESPONSE_IGNORE);                 // アイテム


    PhysicsData::RigidBodyDesc rbDesc;
    rbDesc.mass = 100000.0f;
    rbDesc.centerOfMassOffset = collider->get_Center();
    rbDesc.pos = pos;
    rbDesc.restitution = 0.0f;          // 跳ねない
    rbDesc.friction = 1.0f;             // 摩擦
    rbDesc.owner = generatedObject;     // オーナーオブジェクトの設定
    rbDesc.collider = collider;         // コライダーの設定
    rbDesc.gravity = VEC3(0.0f, -9.8f, 0.0f);
    rbDesc.angularDamping = 0.9f;
    rbDesc.linearDamping = 0.5f;
    rbDesc.angularFactor = VEC3(0.0f, 1.0f, 0.0f);
    rbDesc.localScaling = 0.175f;

    rb->Setup(*Master::m_pPhysicsEngine, rbDesc);

    //
    // ステートの登録
    //
    vehicleController->Start(*renderer);
    StateMachine<VehicleController> stateMachine(vehicleController.get());
    VehicleStateFactory::Create(stateMachine, generationData.createStateID, *renderer);
    stateMachine.SetStartState(generationData.startStateID);
    // 登録
    vehicleController->RegisterStateMachine(stateMachine);

    //Master::m_pDataManager->get_CameraComponent().lock()->set_FocusObject(generatedObject);

    // 生成したオブジェクトを返す
    return generatedObject;
}