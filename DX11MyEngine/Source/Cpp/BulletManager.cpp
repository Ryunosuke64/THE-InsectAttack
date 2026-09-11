#include "pch.h"
#include "RendererEngine.h"
#include "BulletManager.h"
#include "Component_Bullet.h"
#include "Component_ModelMeshResource.h"
#include "Component_NormalBullet.h"
#include "Component_ExplosionBullet.h"
#include "Component_ExplosionLightController.h"
#include "Component_PointLight.h"
#include "Component_BoxCollider.h"
#include "Component_SphereCollider.h"
#include "Component_TrailRenderer.h"
#include "MeshFactory.h"
#include "ResourceManager.h"
#include "Component_MoveLogic.h"
#include "Component_Physics.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;

//////////////////////////////////////////////////////////////////////////////////////////
//
//						各プールのパラメータ
// 
//////////////////////////////////////////////////////////////////////////////////////////
// 通常弾 =====================================================================
constexpr int NUM_DEFAULT__NORMAL_BULLET    = 100;
constexpr int NUM_MAX__NORMAL_BULLET        = 150;

// 爆発弾 =====================================================================
constexpr int NUM_DEFAULT__EXPLOSION_BULLET = 100;
constexpr int NUM_MAX__EXPLOSION_BULLET     = 150;

// 誘導弾 =====================================================================
constexpr int NUM_DEFAULT__HORMING_BULLET   = 50;
constexpr int NUM_MAX__HORMING_BULLET       = 100;



using namespace BulletData;
using namespace GIGA_Engine;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
BulletManager::BulletManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
BulletManager::~BulletManager()
{
    m_BulletObjectPoolMap.clear();
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
bool BulletManager::Init(RendererEngine &renderer)
{
    // 既に作成されているなら返す
    if (!m_BulletObjectPoolMap.empty())
    {
        return true;
    }


    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //						ビルボード弾のプール
    // 
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    m_BulletObjectPoolMap.emplace(BULLET_VISUAL_ARCHETYPE::BILLBOARD,ObjectPool<GameObject>(
        // 取得時に実行 ******************************************************************************************
        [&renderer](GameObject *obj) {          
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

        },
        // 返却時に実行 ******************************************************************************************
        [](GameObject *obj) 
        {
            auto bulletComp = obj->get_Component<Bullet>();
            bulletComp->Reset();

            auto physics = obj->get_Component<Physics>();
            physics->SetZeroVelocity();

            // 軌跡データをクリア
            auto trail = obj->get_Component<TrailRenderer>();
            trail->clear_TrailInfoList();
        },
        // 生成時に実行 ******************************************************************************************
        [&renderer]()->GameObject *
        {
            auto matPtr = Master::m_pResourceManager->FindMaterial("Bullet_01");

            SetupMaterialInfo matInfo[1];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr;
            CreateBillboradInfo billboard;
            billboard.pRenderer = &renderer;
            billboard.Type = BILLBOARD_USAGE_TYPE::SIMPLE;
            billboard.ShaderType = SHADER_TYPE::FORWARD_UNLIT_STATIC;
            billboard.IsActive = true;
            billboard.MatNum = 1;
            billboard.MaterialData = matInfo;
            billboard.IsTransparent = true; // 透明度があり
            billboard.ObjTag = "Bullet";
            auto obj = MeshFactory::CreateBillboard(billboard);

            
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);    // ノンデストロイ
            obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);        // ノンアクティブ
            obj->set_IsUpdateAllowedDuringPause(false);                     // ポーズ中は停止

            // 動的オブジェクトとして設定
            obj->set_IsStatic(false);

            // バレットコンポーネントの追加
            auto bulletComp = obj->add_Component<Bullet>();

            // 直線移動
            auto moveComp = obj->add_Component<MoveLogic>();
            moveComp->Register(MOVE_BEHAVIOUR_TYPE::LINEAR);
            moveComp->ChangeBehaviour(MOVE_BEHAVIOUR_TYPE::LINEAR);

            auto physics = obj->add_Component<Physics>();

            // 軌跡
            auto trail = obj->add_Component<TrailRenderer>();
            trail->set_Width(0.5f);
            trail->set_MinVertexDistance(0.01f);
            trail->set_DrawTime(2);
            trail->set_EmissivePower(1.0f);
            trail->set_Color(VECTOR4::VEC4(1.0f, 1.0f, 0.5f, 1.0f));

            // 初期化
            bulletComp->Start(renderer);

            return obj.get();
        },
        NUM_DEFAULT__NORMAL_BULLET,
        NUM_MAX__NORMAL_BULLET
    ));

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //						3Dモデル弾のプール
    // 
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    m_BulletObjectPoolMap.emplace(BULLET_VISUAL_ARCHETYPE::MODEL, ObjectPool<GameObject>(
        // 取得時に実行 ******************************************************************************************
        [&renderer](GameObject* obj)   
        {          
            // アクティブに
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE); 
        },
        // 返却時に実行 ******************************************************************************************
        [this](GameObject* obj)          
        {
            auto bulletComp = obj->get_Component<Bullet>();
            //const auto bulletHitParam = bulletComp->get_HitData <ExplosionHitConfig>();
            //float explosionRadius = bulletHitParam->_explosionRadius;   // 爆発半径を取得
            bulletComp->Reset();

            // 軌跡データをクリア
            auto trail = obj->get_Component<TrailRenderer>();
            trail->clear_TrailInfoList();

            auto physics = obj->get_Component<Physics>();
            physics->SetZeroVelocity();

            auto transform = obj->get_Transform().lock();
            VEC3 pos = transform->get_VEC3ToPos();
            transform->get_VEC3ToScale();
        },
        // 生成時に実行 ******************************************************************************************
        [&renderer]()->GameObject*  
        {
            // マテリアル取得
            auto matPtr1 = Master::m_pResourceManager->FindMaterial("Bullet");
            SetupMaterialInfo matInfo[1];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr1;

            // メッシュ作成
            CreateModelInfo model;
            model.pRenderer = &renderer;
            model.LODModels[0] = { "Resource/Model/Weapon/bullet.fbx", 0.0f };
            model.ObjTag = "Bullet";
            model.IsAnim = false;
            model.MatNum = 1;
            model.SetupMaterial = matInfo;
            model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC;
            auto obj = MeshFactory::CreateModel(model);
            if (obj == nullptr) {
                assert(false);
                return nullptr;
            }

            // 動的オブジェクトとして設定
            obj->set_IsStatic(false);

            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);    // ノンデストロイ
            obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);        // ノンアクティブ
            obj->set_IsUpdateAllowedDuringPause(false);                     // ポーズ中は停止

            // バレットコンポーネントの追加
            auto bulletComp = obj->add_Component<Bullet>();

            // 移動用コンポーネントの追加
            auto moveComp = obj->add_Component<MoveLogic>();
            moveComp->Register(MOVE_BEHAVIOUR_TYPE::LINEAR);
            moveComp->ChangeBehaviour(MOVE_BEHAVIOUR_TYPE::LINEAR);

            // 軌跡コンポーネントの追加
            auto trail = obj->add_Component<TrailRenderer>();
            trail->set_Width(0.5f);
            trail->set_MinVertexDistance(0.01f);
            trail->set_DrawTime(2);
            trail->set_EmissivePower(1.0f);
            trail->set_Color(VECTOR4::VEC4(1.0f, 1.0f, 0.5f, 1.0f));
            //trail->set_PosRandVec(VEC3(0.5f));

            auto physics = obj->add_Component<Physics>();

            // 初期化
            bulletComp->Start(renderer);

            return obj.get();
        },
        NUM_DEFAULT__EXPLOSION_BULLET,
        NUM_MAX__EXPLOSION_BULLET
    ));

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】更新処理
//*
//* [引数]
//* renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void BulletManager::Update(RendererEngine &renderer)
{
    //=========================================================================================
    //
    //						取り出されたオブジェクトのみ更新
    //
    //=========================================================================================

    //*****************************************************************************************
    //						弾
    //*****************************************************************************************
    for (auto mapIt = m_ExtractedBulletMap.begin(); mapIt != m_ExtractedBulletMap.end(); )
    {
        // プールが存在するかどうかの確認
        auto poolIt = m_BulletObjectPoolMap.find(mapIt->first);
        if (poolIt == m_BulletObjectPoolMap.end())
        {
            OutputDebugString(L"指定された弾のプールが存在しません");
            continue;
        }

        auto& pool = poolIt->second;        // プールの取り出し
        auto &bulletArray = mapIt->second;  // 弾配列の取り出し


        for (auto bulletIt = bulletArray.begin(); bulletIt != bulletArray.end(); )
        {
            auto bullet = *bulletIt;

            // アクティブフラグが降りていれば、プールへ返却
            if (bullet->get_IsStatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE) == false) 
            {
                // 返却
                pool.release(bullet);

                // 次の要素へ
                bulletIt = bulletArray.erase(bulletIt); 
            }
            else
            {
                ++bulletIt;
            }
        }

        ++mapIt;
    }


    //////////////////////////////////////////////////////////////////////////////////////////
    //						デバッグ用
    //              ※ デバッグモードが有効の際に表示
    //////////////////////////////////////////////////////////////////////////////////////////
    if (Master::m_pDataManager->get_IsDebugMode() == false)return;

    Master::m_pDebugger->BeginDebugWindow(Tool::U8ToChar(u8"弾プールの確認"), 0);

    for (int i = 0; i < static_cast<int>(BULLET_VISUAL_ARCHETYPE::NUM); i++)
    {
        if (m_BulletObjectPoolMap.empty())break;

        if (Master::m_pDebugger->DG_TreeNode(std::to_string(i)))
        {
            // プール本体の情報 **********************************************************
            // プールが存在するかどうかチェック
            auto it = m_BulletObjectPoolMap.find(static_cast<BULLET_VISUAL_ARCHETYPE>(i));
            if (it != m_BulletObjectPoolMap.end())
            {
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"プール最大数：%d"), it->second.get_MaxNum());
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"プールの現在の生成数：%d"), it->second.get_CrntCreateNum());


                // プールから取り出して使用しているオブジェクトの情報 ********************************************************
                auto& extractedIt = m_ExtractedBulletMap[static_cast<BULLET_VISUAL_ARCHETYPE>(i)];
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"使用しているオブジェクト数：%d"), extractedIt.size());
                
            }
            else
            {
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"プールが存在しません。"));
            }

            // ツリー終了
            Master::m_pDebugger->DG_TreePop();
        }
    }
    Master::m_pDebugger->EndDebugWindow();

}

//*---------------------------------------------------------------------------------------
//*【?】描画処理
//*
//* [引数]
//* renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void BulletManager::Draw(RendererEngine &renderer)
{

}

//*---------------------------------------------------------------------------------------
//*【?】描現在、アクティブ状態の弾をクリアする（プールへ帰す）
//*     シーンの遷移時など、弾が残ってしまわないように 
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void BulletManager::clear_CrntActiveBullet()
{
    //*****************************************************************************************
    //						弾
    //*****************************************************************************************
    for (auto mapIt = m_ExtractedBulletMap.begin(); mapIt != m_ExtractedBulletMap.end(); )
    {
        // プールが存在するかどうかの確認
        auto poolIt = m_BulletObjectPoolMap.find(mapIt->first);
        if (poolIt == m_BulletObjectPoolMap.end())
        {
            OutputDebugString(L"指定された弾のプールが存在しません");
            continue;
        }
        auto& pool = poolIt->second;        // プールの取り出し
        auto& bulletArray = mapIt->second;  // 弾配列の取り出し
        for (auto bulletIt = bulletArray.begin(); bulletIt != bulletArray.end(); )
        {
            auto bullet = *bulletIt;
            
            // フラグ下す
            bullet->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
            
            // 返却
            pool.release(bullet);
            // 次の要素へ
            bulletIt = bulletArray.erase(bulletIt);
        }
        ++mapIt;
    }
}


//*---------------------------------------------------------------------------------------
//*【?】弾の登録
//*
//* [引数]
//* _bulletType : 登録する弾の種類
//* pBullet : 弾のオブジェクト
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void BulletManager::RegisterBullet(BulletData::BULLET_TYPE _bulletType, std::shared_ptr<GameObject> pBullet)
{

}

//*---------------------------------------------------------------------------------------
//*【?】通常弾の発射
//*
//* [引数]
//* &renderer         : 描画エンジンの参照
//* &_transformData   : トランスフォームパラメータ
//* &_param           : パラメータ
//* [返値] なし
//*----------------------------------------------------------------------------------------
void BulletManager::Shot(RendererEngine &renderer, const BulletData::BulletSpawnContext& _context, const BulletData::Definition&_param)
{
    auto &pool = m_BulletObjectPoolMap.find(_param._commonVisualData._visualArchetype)->second;
    auto obj = pool.get();
    if (obj == nullptr)
    {
        OutputDebugString(L"プールに空きがありません");
        return;
    }
    
    // トランスフォームの設定
    auto transform = obj->get_Transform().lock();
    transform->set_Pos(_context._transform._pos);
    transform->set_RotationQuaternion(_context._transform._rotQ);
    //transform->set_RotateToRad(_transformData._rotRad);
    transform->set_Scale(_param._commonVisualData._scale);


    // 弾コンポーネントのセットアップ
    auto bulletComp = obj->get_Component<Bullet>();
    bulletComp->Setup(&_param, _context);

    const auto bulletParam = bulletComp->get_BulletData();
    
    // 物理コンポーネントに重力の設定
    auto physics = obj->get_Component<Physics>();
    physics->set_GravityScale(bulletParam->_commonData._gravityScale);


    // 軌跡のパラメータ設定
    VEC4 trailColor = VEC4(bulletParam->_commonVisualData._trailColor, 1.0f);
    auto trail = obj->get_Component<TrailRenderer>();
    trail->set_Color(trailColor);
    trail->set_DrawTime(bulletParam->_commonVisualData._trailDrawTime);
    trail->set_Width(bulletParam->_commonVisualData._trailWidth);

    // 弾に合わせたマテリアルに付け替え
    auto matPtr = Master::m_pResourceManager->FindMaterial(bulletParam->_commonVisualData._bulletMaterialTag);
    if (auto billboardRes = obj->get_Component<BillboardResource>()) {
        billboardRes->set_Material(matPtr);
    }

    // 更新リストに登録
    m_ExtractedBulletMap[_param._commonVisualData._visualArchetype].push_back(obj);
}

//*---------------------------------------------------------------------------------------
//*【?】爆発弾用ライトのプール
//*
//* [引数]
//* &renderer         : 描画エンジンの参照
//* &_transformData   : トランスフォームパラメータ
//* &_param           : パラメータ
//* [返値] なし
//*----------------------------------------------------------------------------------------
void BulletManager::ActivateExplosionLight(RendererEngine& renderer, const BulletData::Definition& _param)
{

}