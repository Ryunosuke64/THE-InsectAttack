#include "pch.h"
#include "ItemManager.h"
#include "GameObject.h"
#include "MeshFactory.h"
#include "RendererEngine.h"
#include "Component_BoxCollider.h"
#include "Component_Item.h"
#include "Component_Faction.h"
#include "Component_Physics.h"
#include "Component_RigidBody.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;


constexpr int NUM_DEFAULT_ITEM = 32;    // デフォルトの生成数
constexpr int NUM_MAX_ITEM = 256;       // 最大アイテム数

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
ItemManager::ItemManager():
	m_pItemObjectPool()
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
ItemManager::~ItemManager()
{

}



//*---------------------------------------------------------------------------------------
//*【?】初期化処理
//*
//* [引数]
//* renderer : 描画エンジンの参照
//*
//* [返値] 
//* true : 成功
//* false: 失敗
//*----------------------------------------------------------------------------------------
bool ItemManager::Init(RendererEngine& renderer)
{
    // 既に作成されているなら返す
    if(m_pItemObjectPool != nullptr)
    {
        return true;
    }

    // プール生成
    m_pItemObjectPool = std::make_unique<ObjectPool<GameObject>>(
        // 取得時に実行 ******************************************************************************************
        [&renderer](GameObject* obj)
        {
            // アクティブに
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

            // コライダーを有効状態に
            auto collider = obj->get_Component<BoxCollider>();
            collider->set_IsEnable(true);

        },
        // 返却時に実行 ******************************************************************************************
        [](GameObject* obj)
        {
            // コライダーを無効状態に
            auto collider = obj->get_Component<BoxCollider>();
            collider->set_IsEnable(false);

        },
        // 生成時に実行 ******************************************************************************************
        [&renderer]()->GameObject*
        {
            // マテリアル取得
            auto matPtrR = Master::m_pResourceManager->FindMaterial("Recovery_Billboard");
            auto matPtrRP = Master::m_pResourceManager->FindMaterial("RecoveryPlus_Billboard");
            auto matPtrW = Master::m_pResourceManager->FindMaterial("WeaponBox_Billboard");
            auto matPtrA = Master::m_pResourceManager->FindMaterial("Armor_Billboard");

            SetupMaterialInfo matInfo[1];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtrR;

            CreateBillboradInfo billboard;
            billboard.pRenderer = &renderer;
            billboard.Type = BILLBOARD_USAGE_TYPE::SIMPLE;
            billboard.ShaderType = SHADER_TYPE::FORWARD_UNLIT_STATIC;
            billboard.IsActive = false;
            billboard.MatNum = 1;
            billboard.MaterialData = matInfo;
            billboard.IsTransparent = true; // 透明度があり

            // 生成
            auto obj = MeshFactory::CreateBillboard(billboard);
            obj->get_Transform().lock()->set_Pos(VEC3(0.0f, 0.0f, 0.0f));
            obj->get_Transform().lock()->set_Scale(1.0f, 1.0f, 1.0f);
            obj->set_Tag("Recovey_Large");
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);    // ノンデストロイ
            obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);        // ノンアクティブ

            obj->set_IsUpdateAllowedDuringPause(false); // ポーズ中は更新止める

            //*****************************************************************************************
            //						コンポーネントの追加
            //*****************************************************************************************
            //
            // アイテムコンポーネントの追加
            //
            auto item = obj->add_Component<Item>();
            item->set_ItemType(ITEM_TYPE::RECOVERY_SMALL);
            item->Start(renderer);

            //
            // 派閥コンポーネントの追加
            //
            auto faction = obj->add_Component<Faction>();
            faction->set_Faction(FACTION::ITEM);

            //
            // 物理コンポーネントの追加
            //
            auto rigidBody = obj->add_Component<RigidBody>();


            //
            // コライダーの追加
            //
            auto collider = obj->add_Component<BoxCollider>();
            collider->set_Size(VEC3(0.75f, 0.75f, 0.75f));
            collider->set_Center(VEC3(0, 0.0f, 0)); // コライダーの中心を地面の厚み分だけ下げる
            collider->set_IsTrigger(true);          // 物理判定無し
            collider->set_IsEnable(false);		    // 最初は無効状態
            collider->set_IsStatic(false);

            // 衝突カテゴリ
            collider->set_CollisionCategory(COLLISION_CATEGORY::ITEM);
            // マスク設定
            collider->set_CollisionResponse(COLLISION_CATEGORY::PLAYER, COLLISION_RESPONSE::RESPONSE_OVERLAP);
            collider->set_CollisionResponse(COLLISION_CATEGORY::ITEM, COLLISION_RESPONSE::RESPONSE_BLOCK);
            collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY, COLLISION_RESPONSE::RESPONSE_IGNORE);
            collider->set_CollisionResponse(COLLISION_CATEGORY::BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);
            collider->set_CollisionResponse(COLLISION_CATEGORY::DESTRUCTION_BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);

            //
            // リジッドボディのセットアップ
            //
            PhysicsData::RigidBodyDesc rbDesc;
            rbDesc.mass = 1.0f;
            rbDesc.pos = VEC3(0.0f, 0.0f, 0.0f);
            rbDesc.restitution = 0.2f;
            rbDesc.friction = 0.7f;
            rbDesc.angularFactor = VEC3(0.0f);  // 回転させない
            rbDesc.owner = obj;         // オーナーオブジェクトの設定
            rbDesc.collider = collider; // コライダーの設定


            rigidBody->Setup(*Master::m_pPhysicsEngine, rbDesc);

            return obj.get();
        },
        NUM_DEFAULT_ITEM, 
        NUM_MAX_ITEM
    );

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】更新処理
//*
//* [引数]
//* renderer : 描画エンジンの参照
//*
//* [返値] 
//* true : 成功
//* false: 失敗
//*----------------------------------------------------------------------------------------
void ItemManager::Update(RendererEngine& renderer)
{
    //=========================================================================================
    //
    //						取り出されたオブジェクトのみ更新
    //
    //=========================================================================================
    for (auto it = m_ExtractedItemObject.begin(); it != m_ExtractedItemObject.end(); )
    {
        auto item = *it;

        // アクティブフラグが降りていれば、プールへ返却
        if (item->get_IsStatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE) == false)
        {
            // 返却
            m_pItemObjectPool->release(item);

            // 次の要素へ
            it = m_ExtractedItemObject.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


//*---------------------------------------------------------------------------------------
//*【?】出現中のアイテムをすべてクリア
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void ItemManager::clear_AllItem()
{
    for (auto it = m_ExtractedItemObject.begin(); it != m_ExtractedItemObject.end(); )
    {
        auto item = *it;

        // 非アクティブ化
        item->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        
        // 返却
        m_pItemObjectPool->release(item);

        // 次の要素へ
        it = m_ExtractedItemObject.erase(it);
    }
}


//*---------------------------------------------------------------------------------------
//*【?】アイテムをスポーンさせる
//*
//* [引数]
//* _type : アイテムの種類
//* _pos : 位置
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void ItemManager::SpawnItem(UtilityData::ITEM_TYPE _type, const VECTOR3::VEC3& _pos)
{
    auto obj = m_pItemObjectPool->get();
    if (obj == nullptr)
    {
        OutputDebugStringA("プールに空きがありません");
        return;
    }

    // リジッドボディから位置の設定
    auto rigidBody = obj->get_Component<RigidBody>();
    rigidBody->Teleport(_pos);

    //// トランスフォームの設定
    //auto transform = obj->get_Transform().lock();
    //transform->set_Pos(_pos);

    // アイテムコンポーネントのセットアップ
    auto itemComp = obj->get_Component<Item>();
    itemComp->set_ItemType(_type);

    // **********************************************************
    //                  マテリアルの付け替え
    // **********************************************************
    std::shared_ptr<Material> pMat;
    switch (_type)
    {
    case UtilityData::ITEM_TYPE::RECOVERY_SMALL:
        pMat = Master::m_pResourceManager->FindMaterial("Recovery_Billboard");
        break;
    case UtilityData::ITEM_TYPE::RECOVERY_LARGE:
        pMat = Master::m_pResourceManager->FindMaterial("RecoveryPlus_Billboard");
        break;
    case UtilityData::ITEM_TYPE::ARMOR:
        pMat = Master::m_pResourceManager->FindMaterial("Armor_Billboard");
        break;
    case UtilityData::ITEM_TYPE::WEAPON:
        pMat = Master::m_pResourceManager->FindMaterial("WeaponBox_Billboard");
        break;
    default:
        assert(false);
        break;
    }
    if (auto billboardRes = obj->get_Component<BillboardResource>()) {
        billboardRes->set_Material(pMat);
    }

    // 更新リストに登録
    m_ExtractedItemObject.push_back(obj);
}

//*---------------------------------------------------------------------------------------
//*【?】アイテムをランダムにスポーンさせる
//*
//* [引数]
//* _minNum  : アイテムの最小数
//* _maxNum  : アイテムの最大数
//* _pos     : 位置
//* _radiuse : 位置を中心とした出現範囲
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void ItemManager::SpawnItemRand(int _minNum, int _maxNum, const VECTOR3::VEC3& _pos, float _radiuse)
{
    // ランダム
    int num = Master::m_pRandomManager->GetIntRandom(_minNum, _maxNum);

    for (int i = 0; i < num; i++)
    {
        // プールから取り出し
        auto obj = m_pItemObjectPool->get();
        if (obj == nullptr){
            OutputDebugStringA("プールに空きがありません");
            return;
        }

        // リジッドボディから位置の設定
        auto rigidBody = obj->get_Component<RigidBody>();
        rigidBody->Teleport(_pos + Master::m_pRandomManager->GetVEC3Random(-_radiuse, _radiuse));

        //// トランスフォームの設定
        //auto transform = obj->get_Transform().lock();
        //transform->set_Pos(_pos + Master::m_pRandomManager->GetVEC3Random(-_radiuse, _radiuse));

        // アイテムがまだ回復しかないので
        ITEM_TYPE type = static_cast<ITEM_TYPE>(Master::m_pRandomManager->GetIntRandom(0, 1)); 

        // アイテムコンポーネントのセットアップ
        auto itemComp = obj->get_Component<Item>();
        itemComp->set_ItemType(type);

        // **********************************************************
        //                  マテリアルの付け替え
        // **********************************************************
        std::shared_ptr<Material> pMat;
        switch (type)
        {
        case UtilityData::ITEM_TYPE::RECOVERY_SMALL:
            pMat = Master::m_pResourceManager->FindMaterial("Recovery_Billboard");
            break;
        case UtilityData::ITEM_TYPE::RECOVERY_LARGE:
            pMat = Master::m_pResourceManager->FindMaterial("RecoveryPlus_Billboard");
            break;
        case UtilityData::ITEM_TYPE::ARMOR:
            pMat = Master::m_pResourceManager->FindMaterial("Armor_Billboard");
            break;
        case UtilityData::ITEM_TYPE::WEAPON:
            pMat = Master::m_pResourceManager->FindMaterial("WeaponBox_Billboard");
            break;
        default:
            assert(false);
            break;
        }
        if (auto billboardRes = obj->get_Component<BillboardResource>()) {
            billboardRes->set_Material(pMat);
        }

        // 更新リストに登録
        m_ExtractedItemObject.push_back(obj);
    }
}
