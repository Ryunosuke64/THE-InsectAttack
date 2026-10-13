#include "pch.h"
#include "MapObjectFactory.h"
#include "Component_Health.h"
#include "Component_ModelMeshRenderer.h"
#include "Component_ModelMeshResource.h"
#include "Component_MapObject.h"
#include "Component_RigidBody.h"
#include "Component_CapsuleCollider.h"
#include "Component_SphereCollider.h"
#include "Component_MeshCollider.h"
#include "Component_BoxCollider.h"
#include "Component_CylinderCollider.h"
#include "RendererEngine.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "MeshFactory.h"

using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;

using namespace Map;
using namespace UtilityData;
using namespace PhysicsData;


//*---------------------------------------------------------------------------------------
//*【?】マップオブジェクトのの生成
//* [引数]
//* data : 生成用データ
//*
//* [返値]
//* 生成できたか 
//*----------------------------------------------------------------------------------------
bool MapObjectFactory::CreateMapObject(const Map::MapObjectData& data)
{
    // マップオブジェクトの定義データを探す
    auto assetData = Master::m_pMapManager->FindMapObjectAssetData(data.assetID);

    switch (assetData.objectType)
    {
    case MAP_OBJECT_TYPE::PROP : 
        break;    
    case MAP_OBJECT_TYPE::BUILDING : 
        break;  
    case MAP_OBJECT_TYPE::EXPLOSION_BUILDING : 
        break;
    }

    std::shared_ptr<GameObject>generatedObject;

    auto renderer = Master::m_pDataManager->get_RendererEngine();
    VEC3 pos = data.transform.position;
    VEC3 rot = data.transform.rotation;
    VEC3 scl = data.transform.scale;


    //SetupMaterialInfo *matInfo = nullptr;
    //// マテリアル取得
    //matInfo.Index = assetData.materials[i].Index;
    //matInfo.pMaterialData = assetData.materials[i].pMaterialData;

    CreateModelInfo model;
    model.pRenderer = renderer;
    model.ObjTag = data.assetID;
    model.IsAnim = false;
    model.MatNum = assetData.materials.size();
    model.SetupMaterial = assetData.materials.data();
    model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;

    // LOD
    model.LODModels[0] = { assetData.modelPath, 0.0f };

    generatedObject = MeshFactory::CreateModel(model);

    // 静的オブジェクト
    generatedObject->set_IsStatic(true);

    // ポーズ中は停止
    generatedObject->set_IsUpdateAllowedDuringPause(false);

    // 
    // トランスフォーム設定
    //
    auto transform = generatedObject->get_Component<MyTransform>();
    transform->set_Pos(pos);
    transform->set_RotateToRad(rot);
    transform->set_Scale(scl);

    //
    // 体力コンポーネント追加
    //
    auto health = generatedObject->add_Component<Health>();
    float hp = assetData.health;
    health->set_MaxHP(hp);
    health->set_CrntHP(hp);

    auto modelResource = generatedObject->get_Component<ModelMeshResource>();


    //
    // コライダーの装着
    //
    for (const auto& colliderData : assetData.colliders)
    {
        auto rb = generatedObject->add_Component<RigidBody>();
        std::shared_ptr<Collider> collider;

        switch (colliderData.type)
        {
        case COLLIDER_TYPE::NONE:
            continue;

        case COLLIDER_TYPE::BOX:
            collider = SetupColliderBox(colliderData, generatedObject);
            break;

        case COLLIDER_TYPE::SPHERE:
            collider = SetupColliderSphere(colliderData, generatedObject);
            break;

        case COLLIDER_TYPE::CAPSULE:
            collider = SetupColliderCapsule(colliderData, generatedObject);
            break;

        case COLLIDER_TYPE::CYLINDER:
            collider = SetupColliderCylinder(colliderData, generatedObject);
            break;

        case COLLIDER_TYPE::CONVEX_HULL:
            collider = SetupColliderConvexHull(colliderData, generatedObject, modelResource->get_ModelData());
            break;

        case COLLIDER_TYPE::CONCAVE_TRIANGLE:
            collider = SetupColliderConcave(colliderData, generatedObject, modelResource->get_ModelData());
            break;

        default:
            continue;

        }

        // コライダーの装着が失敗した場合
        if (!collider)
        {
            assert(false);
            continue;
        }

        //*****************************************************************************************
        //						衝突マスクの設定
        //*****************************************************************************************
        // 
        // 押し出しあり
        //
        collider->set_CollisionResponse(COLLISION_CATEGORY::BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);              // 建物
        collider->set_CollisionResponse(COLLISION_CATEGORY::DESTRUCTION_BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);  // 破壊可能建物
        collider->set_CollisionResponse(COLLISION_CATEGORY::PLAYER, COLLISION_RESPONSE::RESPONSE_BLOCK);                // プレイヤー
        collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY, COLLISION_RESPONSE::RESPONSE_BLOCK);                 // エネミー
        collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY_BULLET, COLLISION_RESPONSE::RESPONSE_BLOCK);          // エネミー弾
        collider->set_CollisionResponse(COLLISION_CATEGORY::ITEM, COLLISION_RESPONSE::RESPONSE_BLOCK);                  // アイテム

        //
        // 判定なし
        //


        PhysicsData::RigidBodyDesc rbDesc;
        rbDesc.mass = 0.0f;
        rbDesc.centerOfMassOffset = collider->get_Center();
        rbDesc.pos = pos;
        rbDesc.restitution = 0.0f;          // 跳ねない
        rbDesc.friction = 1.0f;             // 摩擦なし
        rbDesc.owner = generatedObject;     // オーナーオブジェクトの設定
        rbDesc.collider = collider;         // コライダーの設定
        rbDesc.gravity = VEC3(0.0f, -9.8f, 0.0f);

        rb->Setup(*Master::m_pPhysicsEngine, rbDesc);
    }

    return true;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderBox(const Map::ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<BoxCollider>();
    collider->set_Size(data.size);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    collider->set_SurfaceType(SURFACE_TYPE::CONCRETE);

    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderSphere(const Map::ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<SphereCollider>();
    collider->set_Radius(data.radius);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    collider->set_SurfaceType(SURFACE_TYPE::CONCRETE);
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderCapsule(const Map::ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<CapsuleCollider>();
    collider->set_Radius(data.radius);
    collider->set_Height(data.height);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    collider->set_SurfaceType(SURFACE_TYPE::CONCRETE);
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderCylinder(const Map::ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<CylinderCollider>();
    collider->set_Radius(data.radius);
    collider->set_Height(data.height);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    collider->set_SurfaceType(SURFACE_TYPE::CONCRETE);
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderConvexHull(
    const Map::ColliderData& data, 
    std::shared_ptr<GameObject> obj,
    std::weak_ptr<class ModelData> modelData)
{
    auto collider = obj->add_Component<MeshCollider>();
    collider->SetupModelData(modelData);
    collider->set_IsConvex(true);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    collider->set_SurfaceType(SURFACE_TYPE::CONCRETE);
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderConcave(
    const Map::ColliderData& data, 
    std::shared_ptr<GameObject> obj,
    std::weak_ptr<class ModelData> modelData)
{
    auto collider = obj->add_Component<MeshCollider>();
    collider->SetupModelData(modelData);
    collider->set_IsConvex(false);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    collider->set_SurfaceType(SURFACE_TYPE::CONCRETE);
    return collider;
}