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
#include "Component_Destructible.h"
#include "Component_TimerDestruction.h"
#include "DestructionBehavior_Header.h"
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

    // 物理プリセットを探す
    MO_PhysicsData physicsData = 
        Master::m_pMapManager->FindPhysicsDataPreset(assetData.physicsPresetID);

    std::shared_ptr<GameObject>generatedObject;

    auto renderer = Master::m_pDataManager->get_RendererEngine();
    VEC3 pos = data.transform.position;
    VEC3 rot = data.transform.rotation;
    VEC3 scl = data.transform.scale;

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
    if (!generatedObject)
    {
        assert(false);
        return false;
    }

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
    // マップオブジェクトコンポーネント追加
    //
    auto mapObjectComp = generatedObject->add_Component<MapObject>();
    mapObjectComp->set_AssetID(assetData.assetID);  // アセットIDの設定

    //
    // 体力コンポーネント追加
    //
    auto health = generatedObject->add_Component<Health>();
    float hp = assetData.health;
    health->set_MaxHP(hp);
    health->set_CrntHP(hp);

    // 
    // 破壊コンポーネント追加
    // Setup内で体力コンポーネントを取得しているので、
    // 必ず体力コンポーネントをつけた後に追加する
    //
    if (assetData.isDestruction)
    {
        auto destructible = generatedObject->add_Component<Destructible>();
        switch (assetData.objectType)
        {
            // ***************************************
            // 小物系
            // ***************************************
        case MAP_OBJECT_TYPE::PROP:
            destructible->Setup(std::make_unique<PropDestructionBehavior>());
            break;

            // ***************************************
            // 建物
            // ***************************************
        case MAP_OBJECT_TYPE::BUILDING:
            destructible->Setup(std::make_unique<ReplaceModelDestructionBehavior>());
            break;

            // ***************************************
            // 爆発建物
            // ***************************************
        case MAP_OBJECT_TYPE::EXPLOSION_BUILDING:
            destructible->Setup(std::make_unique<PropDestructionBehavior>());
            break;
        }
    }

    auto modelResource = generatedObject->get_Component<ModelMeshResource>();

    //
    // コライダーの装着
    //
    for (const auto& colliderData : physicsData.colliders)
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

        // 材質設定
        collider->set_SurfaceType(colliderData.surfaceType);

        // カテゴリ設定
        collider->set_CollisionCategory(physicsData._myCollisionCategory);

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
        collider->set_CollisionResponse(COLLISION_CATEGORY::PROP, COLLISION_RESPONSE::RESPONSE_BLOCK);                  // 小物

        //
        // 判定なし
        //

        MO_RigidBodyData rigidBody = physicsData.rigidBody;

        PhysicsData::RigidBodyDesc rbDesc;
        rbDesc.mass = 0.0f;                                     // 質量（マップに配置されるオブジェクトは基本動かないため、最初は質量を0にする。破壊時などは、アセットIDからデータを取り出して設定しなおす）
        rbDesc.restitution = rigidBody.restitution;             // 反発係数
        rbDesc.friction = rigidBody.friction;                   // 摩擦係数
        rbDesc.linearFactor = rigidBody.linearFactor;           // 移動制限
        rbDesc.angularFactor = rigidBody.angularFactor;         // 回転制限
        rbDesc.linearDamping = rigidBody.linearDamping;         // 移動減衰
        rbDesc.angularDamping = rigidBody.angularDamping;       // 回転減衰
        rbDesc.gravity = VEC3(0.0f, -9.8f * rigidBody.gravityScale , 0.0f); // 重力

        rbDesc.centerOfMassOffset = collider->get_Center();     // 中心位置オフセット
        rbDesc.collider = collider;                             // コライダーの設定
        rbDesc.pos = pos;                                       // 位置
        rbDesc.owner = generatedObject;                         // オーナーオブジェクトの設定

             

        rb->Setup(*Master::m_pPhysicsEngine, rbDesc);
    }

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】建物の破片モデルの生成
//* [引数]
//* _assetID   : 紐付けされた建物のアセットID
//*
//* [返値]
//* 生成できたか 
//*----------------------------------------------------------------------------------------
bool MapObjectFactory::CreateBuildingDebris(const std::string& _assetID, const Map::MO_TransformData& _transform)
{
    // 破片モデルの定義データを探す
    auto fragmentData = Master::m_pMapManager->FindBuildingFragmentDataSet(_assetID);
    auto assetData = Master::m_pMapManager->FindMapObjectAssetData(_assetID);

    auto sourceModel = 
        Master::m_pResourceManager->LoadModel(assetData.modelPath.c_str());

    if (!sourceModel)
    {
        assert(false);
        return false;
    }

    const auto* sourceScene = sourceModel->get_Scene();
    std::unordered_map<std::string, std::shared_ptr<Material>>materialByName;
    for (const auto& mat : assetData.materials)
    {
        // 範囲、有効チェック
        if (mat.Index < 0 ||
            mat.Index >= sourceScene->mNumMaterials ||
            !mat.pMaterialData)
        {
            assert(false);
            return false;
        }

        aiString name;
        // マテリアル名取り出し
        if (sourceScene->mMaterials[mat.Index]->Get(AI_MATKEY_NAME, name) !=
            aiReturn_SUCCESS)
        {
            assert(false);
            return false;
        }

        if (!materialByName.emplace(name.C_Str(), mat.pMaterialData).second)
        {
            assert(false);
            return false;
        }
    }

    
    // データ定義配列回す
    for (const auto& data : fragmentData.fragmentDataArray)
    {
        auto fragmentModel =
            Master::m_pResourceManager->LoadModel(data.modelPath.c_str());
        
        if (!fragmentModel) {
            assert(false);
            return false;
        }

        const auto* fragmentScene = fragmentModel->get_Scene();
        std::vector<SetupMaterialInfo>fragmentMaterials;

        for (int slot = 0; slot < fragmentScene->mNumMaterials; slot++)
        {
            aiString name;
            if (fragmentScene->mMaterials[slot]->Get(AI_MATKEY_NAME, name) !=
                aiReturn_SUCCESS)
            {
                assert(false);
                return false;
            }

            auto it = materialByName.find(name.C_Str());
            if (it == materialByName.end())
            {
                assert(false);
                return false;
            }

            fragmentMaterials.push_back({ it->second, slot });
        }


        // 物理プリセットを探す
        MO_PhysicsData physicsData =
            Master::m_pMapManager->FindPhysicsDataPreset(data.physicsPresetID);

        std::shared_ptr<GameObject>generatedObject;

        auto renderer = Master::m_pDataManager->get_RendererEngine();
        VEC3 pos = _transform.position;
        VEC3 rot = _transform.rotation;
        VEC3 scl = _transform.scale;

        CreateModelInfo model;
        model.pRenderer = renderer;
        model.ObjTag = _assetID;
        model.IsAnim = false;
        model.MatNum = fragmentMaterials.size();
        model.SetupMaterial = fragmentMaterials.data();
        model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;

        // LOD
        model.LODModels[0] = { data.modelPath, 0.0f };

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
        // 時間削除コンポーネントの追加
        //
        auto timeDestruction = generatedObject->add_Component<TimerDestruction>();
        timeDestruction->set_LifeTime(15.0f);

        auto modelResource = generatedObject->get_Component<ModelMeshResource>();

        // ここから追加
        auto fragmentModelData = modelResource->get_ModelData().lock();
        if (!fragmentModelData)
        {
            assert(false);
            return false;
        }

        // Assimp取り込み後の頂点範囲の中心を、重心の近似として使う
        const auto& boundsCenter =
            fragmentModelData->get_LocalBounds().Center;

        const VEC3 fragmentCenter(
            boundsCenter.x,
            boundsCenter.y,
            boundsCenter.z
        );



        //
        // コライダーの装着
        //
        for (const auto& colliderData : physicsData.colliders)
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

            // 材質設定
            collider->set_SurfaceType(colliderData.surfaceType);

            // カテゴリ設定
            collider->set_CollisionCategory(physicsData._myCollisionCategory);

            // コライダーの中心位置をモデルのローカル座標に合わせる
            //collider->set_Center(data.localTransform.position);

            //*****************************************************************************************
            //						衝突マスクの設定
            //*****************************************************************************************
            // 
            // 押し出しあり
            //
            collider->set_CollisionResponse(COLLISION_CATEGORY::BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);              // 建物
            collider->set_CollisionResponse(COLLISION_CATEGORY::DESTRUCTION_BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);  // 破壊可能建物

            //
            // 判定なし
            //
            collider->set_CollisionResponse(COLLISION_CATEGORY::PLAYER, COLLISION_RESPONSE::RESPONSE_IGNORE);                // プレイヤー
            collider->set_CollisionResponse(COLLISION_CATEGORY::PLAYER_BULLET, COLLISION_RESPONSE::RESPONSE_IGNORE);                // プレイヤー
            collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY, COLLISION_RESPONSE::RESPONSE_IGNORE);                 // エネミー
            collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY_BULLET, COLLISION_RESPONSE::RESPONSE_IGNORE);          // エネミー弾
            collider->set_CollisionResponse(COLLISION_CATEGORY::ITEM, COLLISION_RESPONSE::RESPONSE_IGNORE);                  // アイテム
            collider->set_CollisionResponse(COLLISION_CATEGORY::PROP, COLLISION_RESPONSE::RESPONSE_IGNORE);                  // 小物

            MO_RigidBodyData rigidBody = physicsData.rigidBody;

            PhysicsData::RigidBodyDesc rbDesc;
            rbDesc.mass = rigidBody.mass;                           // 質量
            rbDesc.restitution = rigidBody.restitution;             // 反発係数
            rbDesc.friction = rigidBody.friction;                   // 摩擦係数
            rbDesc.linearFactor = rigidBody.linearFactor;           // 移動制限
            rbDesc.angularFactor = rigidBody.angularFactor;         // 回転制限
            rbDesc.linearDamping = rigidBody.linearDamping;         // 移動減衰
            rbDesc.angularDamping = rigidBody.angularDamping;       // 回転減衰
            rbDesc.gravity = VEC3(0.0f, -9.8f * rigidBody.gravityScale, 0.0f); // 重力

            rbDesc.centerOfMassOffset = fragmentCenter;             // 中心位置オフセット
            rbDesc.collider = collider;                             // コライダーの設定
            rbDesc.pos = pos;                                       // 位置
            rbDesc.owner = generatedObject;                         // オーナーオブジェクトの設定

            rb->Setup(*Master::m_pPhysicsEngine, rbDesc);
        }
    }
    return true;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderBox(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<BoxCollider>();
    collider->set_Size(data.size);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ

    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderSphere(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<SphereCollider>();
    collider->set_Radius(data.radius);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderCapsule(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<CapsuleCollider>();
    collider->set_Radius(data.radius);
    collider->set_Height(data.height);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderCylinder(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj)
{
    auto collider = obj->add_Component<CylinderCollider>();
    collider->set_Radius(data.radius);
    collider->set_Height(data.height);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderConvexHull(
    const Map::MO_ColliderData& data, 
    std::shared_ptr<GameObject> obj,
    std::weak_ptr<class ModelData> modelData)
{
    auto collider = obj->add_Component<MeshCollider>();
    collider->SetupModelData(modelData);
    collider->set_IsConvex(true);
    collider->set_Center(data.center);
    collider->set_IsStatic(false);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    return collider;
}

std::shared_ptr<class Collider> MapObjectFactory::
SetupColliderConcave(
    const Map::MO_ColliderData& data, 
    std::shared_ptr<GameObject> obj,
    std::weak_ptr<class ModelData> modelData)
{
    auto collider = obj->add_Component<MeshCollider>();
    collider->SetupModelData(modelData);
    collider->set_IsConvex(false);
    collider->set_Center(data.center);
    collider->set_IsStatic(true);
    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);// 衝突カテゴリ
    return collider;
}