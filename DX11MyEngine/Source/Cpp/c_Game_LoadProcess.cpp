#include "pch.h"
#include "GameScene_StateHeader.h"
#include "RendererEngine.h"
#include "GameManager.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "GameObject.h"
#include "InputFactory.h"
#include "MeshFactory.h"
#include "Component_3DCamera.h"
#include "Component_PlayerController.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_ModelMeshResource.h"
#include "Component_ModelMeshRenderer.h"
#include "Component_MeshRenderer.h"
#include "Component_DirectionalLight.h"
#include "Component_PointLight.h"
#include "Component_SpotLight.h"
#include "Component_SpriteRenderer.h"
#include "Component_BillboardRenderer.h"
#include "Component_SkyRenderer.h"
#include "Component_AssultRifle.h"
#include "Component_GunWeapon.h"
#include "Component_BoxCollider.h"
#include "Component_SphereCollider.h"
#include "Component_TrailRenderer.h"
#include "Component_EnemyController.h"
#include "Component_Health.h"
#include "Component_LineRenderer.h"
#include "Component_WeaponController.h"
#include "Component_BuildingController.h"
#include "Component_MiniMapRader.h"
#include "Component_Player_HPBar.h"
#include "Component_MoveLogic.h"
#include "Component_Faction.h"
#include "Component_Item.h"
#include "Component_DistortionEffect.h"
#include "Component_Physics.h"
#include "EnemyFactory.h"

using namespace UtilityData;
using namespace EnemyData;
using namespace SceneStateEnums;
using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;

constexpr int ENEMY_OCT_NUM = 1;
constexpr int ENEMY_ANT_NUM = 300;

//*---------------------------------------------------------------------------------------
//* @:c_Game_LoadProcess Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_LoadProcess::OnEnter(SceneManager *pOwner)
{
	// ロード画面用スプライトをオンに
	auto obj = Master::m_pGameObjectManager->get_ObjectByTag("LoadScreen_Sp");
	if (obj)
	{
		obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	}

    // ロード画面用用スプライトを作る **********************************************
    float width = static_cast<float>(m_pRenderer->get_ScreenWidth());
    float height = static_cast<float>(m_pRenderer->get_ScreenHeight());
    UIData::RectTransformData rectData;
    rectData._size = VEC2(width, height);
    UIData::SpriteUIData spriteData;
    spriteData._tag = "LoadSprite";
    spriteData._imagePath = "Resource/Texture/Title/Game_Load_01.png";
    spriteData._layerRank = 101;
    m_pLoadBackObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectData, spriteData);
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_LoadProcess Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_LoadProcess::OnExit(SceneManager* pOwner)
{
    // *************************************************************************************************
    // 弾管理クラスの初期化
    // *************************************************************************************************
    if (!Master::m_pBulletManager->Init(*m_pRenderer))
    {
        MessageBoxA(NULL, "弾管理クラスの初期化に失敗しました", "GameLoad", MB_OK);
        assert(false);
    }

    // *************************************************************************************************
    // アイテム管理クラスの初期化
    // *************************************************************************************************
    if (!Master::m_pItemManager->Init(*m_pRenderer))
    {
        MessageBoxA(NULL, "アイテム管理クラスの初期化に失敗しました", "GameLoad", MB_OK);
        assert(false);
    }
    
    Master::m_pMissionDirector->LoadMissionScript("Resource/MISSION_AS/MISSION_01.as");
    Master::m_pMissionDirector->ExcuteScriptMissionSetup();


    /* B-2 モデルの生成 */
    {
        //// マテリアル取得
        //auto matPtr = Master::m_pResourceManager->FindMaterial("B_2");

        //SetupMaterialInfo matInfo[1];
        //matInfo[0].Index = 0;
        //matInfo[0].pMaterialData = matPtr;

        //CreateModelInfo model;
        //model.pRenderer = m_pRenderer;
        //model.Path = "Resource/Model/b-2/B-2_NonBone.fbx";
        //model.ObjTag = "B-2";
        //model.IsAnim = false;
        //model.MatNum = 1;
        //model.SetupMaterial = matInfo;
        //model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;
        //for (int i = 0; i < 3; i++)
        //{
        //    model.ObjTag = "B-2_" + std::to_string(i + 1);
        //    auto obj = MeshFactory::CreateModel(model);
        //    obj->get_Component<MyTransform>()->set_Scale(0.05f, 0.05f, 0.05f);           
        //    // ポーズ中は停止
        //    obj->set_IsUpdateAllowedDuringPause(false);
        //}
    }

    /* 建物 モデルの生成 */
    {
        {
            // マテリアル取得
            auto matPtr1 = Master::m_pResourceManager->FindMaterial("Building01_Top");
            auto matPtr2 = Master::m_pResourceManager->FindMaterial("Building01_Base1");
            auto matPtr3 = Master::m_pResourceManager->FindMaterial("Building01_Base2");
            auto matPtr4 = Master::m_pResourceManager->FindMaterial("Building01_Base3");

            SetupMaterialInfo matInfo[6];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr1;

            matInfo[1].Index = 1;
            matInfo[1].pMaterialData = matPtr2;

            matInfo[2].Index = 2;
            matInfo[2].pMaterialData = matPtr3;

            matInfo[3].Index = 3;
            matInfo[3].pMaterialData = matPtr4;

            //matInfo[4].Index = 4;
            //matInfo[4].pMaterialData = matPtr5;

            //matInfo[5].Index = 5;
            //matInfo[5].pMaterialData = matPtr6;


            CreateModelInfo model;
            model.pRenderer = m_pRenderer;
            //model.Path = "Resource/Model/Building/03/Building_Tower_1.fbx";
            model.LODModels[0] = { "Resource/Model/Building/02/Building_01.fbx", 0.0f };
            model.ObjTag = "Building";
            model.IsAnim = false;
            model.MatNum = 4;
            model.SetupMaterial = matInfo;
            model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;

            // 建物を5x5のグリッドで配置
            for (int x = -2; x < 3; x++)
            {
                for (int y = -2; y < 3; y++)
                {
                    VEC3 scale = VEC3(0.8f);
                    auto obj = MeshFactory::CreateModel(model);
                    obj->get_Component<MyTransform>()->set_Scale(scale);
                    obj->get_Component<MyTransform>()->set_Pos(50.0f * x, 0.0f, 70.0f * y);
                    obj->get_Component<MyTransform>()->set_RotateToDeg(0.0f, 0.0f, 0.0f);

                    // ポーズ中は停止
                    obj->set_IsUpdateAllowedDuringPause(false);

                    // 建物制御コンポーネント追加
                    obj->add_Component<BuildingController>();

                    // 体力コンポーネントの追加
                    auto health = obj->add_Component<Health>();
                    health->set_MaxHP(600.0f);
                    health->set_CrntHP(600.0f);

                    // コライダーの追加
                    auto collider = obj->add_Component<BoxCollider>();
                    collider->set_Size(VEC3(20.0f * scale.x, 30.0f * scale.y, 10.0f * scale.z));
                    collider->set_Center(VEC3(0.0f, 30.0f * scale.y, 0.0f));
                    collider->set_IsStatic(true);
                    // 衝突カテゴリ
                    collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);


                    // コライダーの登録
                    Master::m_pCollisionManager->RegisterCollider(obj->get_Component<BoxCollider>());
                }
            }
        }

        // タワー
        {
            auto matPtr1 = Master::m_pResourceManager->FindMaterial("Building_Tower_1_SupportPillar");
            auto matPtr2 = Master::m_pResourceManager->FindMaterial("Building_Tower_1_TopPillar");
            auto matPtr3 = Master::m_pResourceManager->FindMaterial("Building_Tower_1_TopToroid");
            auto matPtr4 = Master::m_pResourceManager->FindMaterial("Building_Tower_1_Top");
            auto matPtr5 = Master::m_pResourceManager->FindMaterial("Building_Tower_1_BaseGround");
            auto matPtr6 = Master::m_pResourceManager->FindMaterial("Building_Tower_1_Wall");

            SetupMaterialInfo matInfo[6];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr1;

            matInfo[1].Index = 1;
            matInfo[1].pMaterialData = matPtr2;

            matInfo[2].Index = 2;
            matInfo[2].pMaterialData = matPtr3;

            matInfo[3].Index = 3;
            matInfo[3].pMaterialData = matPtr4;

            matInfo[4].Index = 4;
            matInfo[4].pMaterialData = matPtr5;

            matInfo[5].Index = 5;
            matInfo[5].pMaterialData = matPtr6;            
            CreateModelInfo model;
            model.pRenderer = m_pRenderer;
            model.ObjTag = "Building";
            model.IsAnim = false;
            model.LODModels[0] = { "Resource/Model/Building/03/Building_Tower_1.fbx", 0.0f };
            model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;
            model.MatNum = 6;
            model.SetupMaterial = matInfo;

            VEC3 scale = VEC3(1.0f);
            auto obj = MeshFactory::CreateModel(model);
            obj->get_Component<MyTransform>()->set_Scale(scale);
            obj->get_Component<MyTransform>()->set_Pos(-200.0f, 0.0f, 150.0f);
            obj->get_Component<MyTransform>()->set_RotateToDeg(0.0f, 0.0f, 0.0f);

            // ポーズ中は停止
            obj->set_IsUpdateAllowedDuringPause(false);

            // 建物制御コンポーネント追加
            obj->add_Component<BuildingController>();

            // 体力コンポーネントの追加
            auto health = obj->add_Component<Health>();
            health->set_MaxHP(1600.0f);
            health->set_CrntHP(1600.0f);

            // コライダーの追加
            auto collider = obj->add_Component<BoxCollider>();
            collider->set_Size(VEC3(20.0f * scale.x, 50.0f * scale.y, 10.0f * scale.z));
            collider->set_Center(VEC3(0.0f, 50.0f * scale.y, 0.0f));
            collider->set_IsStatic(true);
            // 衝突カテゴリ
            collider->set_CollisionCategory(COLLISION_CATEGORY::DESTRUCTION_BUILDING);

            // コライダーの登録
            Master::m_pCollisionManager->RegisterCollider(obj->get_Component<BoxCollider>());
        }

    }

    /* マザーシップの生成 */
    {
        // マテリアル取得
        auto matPtr1 = Master::m_pResourceManager->FindMaterial("MotherShip");

        SetupMaterialInfo matInfo[1];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr1;

        CreateModelInfo model;
        model.pRenderer = m_pRenderer;
        model.LODModels[0] = { "Resource/Model/Enemy/MotherShip/MotherShip.fbx", 0.0f };
        model.ObjTag = "MotherShip";
        model.IsAnim = false;
        model.MatNum = 1;
        model.IsActive = true;
        model.SetupMaterial = matInfo;
        model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC;

        auto obj = MeshFactory::CreateModel(model);
		obj->set_IsStatic(false);
        obj->get_Component<MyTransform>()->set_Scale(0.1f, 0.1f, 0.1f);
        obj->get_Component<MyTransform>()->set_Pos(0.0f, 500.0f, 0.0f);
        obj->get_Component<MyTransform>()->set_RotateToDeg(0.0f, 0.0f, 0.0f);
    }

    /* 地面の生成 */
    {
        // マテリアル取得
        auto matPtr = Master::m_pResourceManager->FindMaterial("Ground");
        //auto matPtr = Master::m_pResourceManager->FindMaterial("PointLight");

        SetupMaterialInfo matInfo[1];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr;

        CreateUtilityMeshInfo mesh;
        mesh.pRenderer = m_pRenderer;
        mesh.Type = UTILITY_MESH_TYPE::PLANE;
        mesh.ObjTag = "Ground";
        mesh.MatNum = 1;
        mesh.MaterialData = matInfo;
        mesh.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;
        mesh.IsNormalMap = true;
		mesh.TilingScale = VEC2(60.0f, 60.0f);
        mesh.ObjLayer = 90;

        auto obj = MeshFactory::CreateUtilityMesh(mesh);
        obj->get_Transform().lock()->set_Scale(400.0f, 1.0f, 400.0f);
        obj->get_Transform().lock()->set_Pos(0.0f, 0.0f, 0.0f);
        obj->get_Transform().lock()->set_RotateToDeg(0.0f, 0.0f, 0.0f);

        // コライダーの追加
        auto collider = obj->add_Component<BoxCollider>();
        collider->set_Size(VEC3(400.0f, 1.0f, 400.0f));
        collider->set_Center(VEC3(0, -1.0f, 0)); // コライダーの中心を地面の厚み分だけ下げる
        collider->set_IsStatic(true);
        // 衝突カテゴリ
        collider->set_CollisionCategory(COLLISION_CATEGORY::BUILDING);

        // コライダーの登録
        Master::m_pCollisionManager->RegisterCollider(collider);
    }

    /* ディストーション */
  //  {
  //      auto obj = GIGA_Engine::Instantiate3D(std::make_shared<GameObject>(), true);
  //      obj->set_Tag("DistortionEffect");
  //      obj->set_LayerRank(1000);
  //      obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
		//obj->get_Transform().lock()->set_Pos(-155.0f, 5.0f, 100.0f);
  //      auto distortion = obj->add_Component<DistortionEffect>();
  //      distortion->Setup(*m_pRenderer);
  //  }

    /* スカイボックスの生成 */
    {
        // マテリアル取得
        auto matPtr = Master::m_pResourceManager->FindMaterial("SkyBox01");

        SetupMaterialInfo matInfo[1];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr;

        CreateSkyboxInfo skyInfo;
        skyInfo.pRenderer = m_pRenderer;
        skyInfo.ObjTag = "Skybox";
        skyInfo.MatNum = 1;
        skyInfo.MaterialData = matInfo;
        skyInfo.ShaderType = SHADER_TYPE::POST_SKYBOX;
        skyInfo.IsActive = false;

        auto obj = MeshFactory::CreateSkybox(skyInfo);
        obj->get_Transform().lock()->set_Scale(1.0f, 1.0f, 1.0f);
        obj->get_Transform().lock()->set_Pos(0.0f, 0.0f, 0.0f);

		// スカイレンダラーをデータマネージャに登録
		Master::m_pDataManager->set_SkyRenderer(obj->get_Component<SkyRenderer>());
    }

    /* ポイントライトの生成 (Cubuで分かりやすく)*/
    {
        // マテリアル取得
        auto matPtr = Master::m_pResourceManager->FindMaterial("Block");

        SetupMaterialInfo matInfo[1];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr;

        CreateUtilityMeshInfo mesh;
        mesh.pRenderer = m_pRenderer;
        mesh.Type = UTILITY_MESH_TYPE::CUBE;
        mesh.MatNum = 1;
        mesh.MaterialData = matInfo;
        mesh.IsActive = true;
        mesh.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;
        mesh.IsNormalMap = true;
        mesh.ObjLayer = 105;

        for (int i = 0; i < 5; i++)
        {
            VEC3 pos;
            pos.x = -140.0f;
            pos.y = 1.0f;
            pos.z = 50.0f + i * 10;
            VEC3 col;
            col.x = static_cast<float>(rand() % 255) / 255.0f;
            col.y = static_cast<float>(rand() % 255) / 255.0f;
            col.z = static_cast<float>(rand() % 255) / 255.0f;
            VEC3 scl = VEC3(1.0f);

            auto obj = MeshFactory::CreateUtilityMesh(mesh);
            obj->get_Transform().lock()->set_Pos(pos);
            obj->get_Transform().lock()->set_Scale(scl);
            obj->set_Tag("Block");
            obj->set_IsUpdateAllowedDuringPause(false);
            obj->set_IsStatic(false);
            auto light = obj->add_Component<SpotLight>();
            light->set_SpotLightData(30.0f, 15.0f);
            light->set_LightColor(col);
            light->set_Intensity(5.5f);

            // 物理コンポーネント
            auto physics = obj->add_Component<Physics>();
            physics->set_AirDrag(1.0f);

            // コライダーの追加
            auto collider = obj->add_Component<BoxCollider>();
            collider->set_Size(VEC3(1.0f, 1.0f, 1.0f));
            collider->set_Center(VEC3(0, 0, 0));
            collider->set_IsStatic(false);
            // 衝突カテゴリ
            collider->set_CollisionCategory(COLLISION_CATEGORY::BUILDING);

            // コライダーの登録
            Master::m_pCollisionManager->RegisterCollider(collider);
        }
    }

    /* 足場 */
    {
        // マテリアル取得
        auto matPtr = Master::m_pResourceManager->FindMaterial("PointLight");

        SetupMaterialInfo matInfo[1];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr;

        CreateUtilityMeshInfo mesh;
        mesh.pRenderer = m_pRenderer;
        mesh.Type = UTILITY_MESH_TYPE::CUBE;
        mesh.MatNum = 1;
        mesh.MaterialData = matInfo;
        mesh.IsActive = true;
        mesh.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;
        mesh.IsNormalMap = true;

        auto obj = MeshFactory::CreateUtilityMesh(mesh);
        obj->get_Transform().lock()->set_Pos(VEC3(0.0f, 10.0f, 0.0f));
        obj->get_Transform().lock()->set_Scale(VEC3(5.0f, 1.0f, 5.0f));
        obj->set_Tag("Scaffolding");

        // コライダーの追加
        auto collider = obj->add_Component<BoxCollider>();
        collider->set_Size(VEC3(5.0f, 1.0f, 5.0f));
        collider->set_Center(VEC3(0, 0, 0));
        collider->set_IsStatic(true);

        // 衝突カテゴリ
        collider->set_CollisionCategory(COLLISION_CATEGORY::BUILDING);

        // コライダーの登録
        Master::m_pCollisionManager->RegisterCollider(collider);
    }


    /* 足場 */
    {
        // マテリアル取得
        auto matPtr = Master::m_pResourceManager->FindMaterial("Block");
        SetupMaterialInfo matInfo[1];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr;
        CreateUtilityMeshInfo mesh;
        mesh.pRenderer = m_pRenderer;
        mesh.Type = UTILITY_MESH_TYPE::CUBE;
        mesh.MatNum = 1;
        mesh.MaterialData = matInfo;
        mesh.IsActive = true;
        mesh.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;
        mesh.IsNormalMap = true;
        auto obj = MeshFactory::CreateUtilityMesh(mesh);
        obj->get_Transform().lock()->set_Pos(VEC3(-50.0f, 1.0f, 90.0f));
        obj->get_Transform().lock()->set_Scale(VEC3(5.0f, 1.0f, 5.0f));
        obj->set_Tag("Scaffolding");

        // コライダーの追加
        auto collider = obj->add_Component<BoxCollider>();
        collider->set_Size(VEC3(5.0f, 1.0f, 5.0f));
        collider->set_Center(VEC3(0, 0, 0));
        collider->set_IsStatic(true);

        // 衝突カテゴリ
        collider->set_CollisionCategory(COLLISION_CATEGORY::BUILDING);

        // コライダーの登録
        Master::m_pCollisionManager->RegisterCollider(collider);
    }


    /* 画面上のUIの生成 */
    {

        /* 武器のサイト用スプライト*/
        {
            CreateSpriteInfo sprite;
            sprite.pTextureMap[0] = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/crosshair094.png");
            sprite.IsActive = true;
            sprite.ObjTag = "GunSight01";
            sprite.pRenderer = m_pRenderer;
            sprite.ShaderType = SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE;
            sprite.Type = SPRITE_USAGE_TYPE::NORMAL;
            sprite.Width = 50.0f;
            sprite.Height = 50.0f;
            sprite.IsActive = true;
            sprite.IsTransparent = true;
            auto obj = MeshFactory::CreateSprite(sprite);
            if (obj)
            {
                obj->get_RectTransform().lock()->set_AnchorMax(VEC2(0.5f, 0.5f));
                obj->get_RectTransform().lock()->set_AnchorMin(VEC2(0.5f, 0.5f));
                obj->get_RectTransform().lock()->set_RectPosition(VEC2(-30.0f, 0.0f));
                obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);
            }
        }
		// ミニマップレーダー
        {
            auto obj = GIGA_Engine::Instantiate2D(std::make_shared<GameObject>(), true);
            auto rader = obj->add_Component<MiniMapRader>();
            obj->set_Tag("MiniMapRader");
            obj->set_LayerRank(100);
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        }

		// プレイヤーHPバー
        {
            auto obj = GIGA_Engine::Instantiate2D(std::make_shared<GameObject>(), true);
            auto rader = obj->add_Component<Player_HPBar>();
            obj->set_Tag("Player_HPBar");
            obj->set_LayerRank(100);
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        }
    }


    // プレイヤーオブジェクトの取得
    auto playerObj = Master::m_pGameObjectManager->get_ObjectByTag("Player");
    playerObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);


    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //						武器
    // 
    //////////////////////////////////////////////////////////////////////////////////////////
    // プレイヤーの武器制御コンポーネントへの登録用のキャッシュ
    std::shared_ptr<GunWeapon>weapon_1;
    std::shared_ptr<GunWeapon>weapon_2;
    {
        /* アサルトライフル */
        {
            // マテリアル取得
            auto matPtr1 = Master::m_pResourceManager->FindMaterial("AssultRifle");

            SetupMaterialInfo matInfo[1];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr1; // 体

            CreateModelInfo model;
            model.pRenderer = m_pRenderer;
            model.LODModels[0] = { "Resource/Model/Weapon/M4A1.fbx", 0.0f };
            model.ObjTag = "AssultRifle";
            model.IsAnim = false;
            model.MatNum = 1;
            model.SetupMaterial = matInfo;
            model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC;
            model.ObjLayer = 92;
            auto obj = MeshFactory::CreateModel(model);

            // 破棄しない
            //obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);
            // 動的オブジェクトに設定
            obj->set_IsStatic(false);

            // ポーズ中は停止
            obj->set_IsUpdateAllowedDuringPause(false);

            // アサルトライフル
            //obj->add_Component<AssultRifle>(1);
            weapon_1 = obj->add_Component<GunWeapon>(1);
            weapon_1->Setup(Master::m_pWeaponDataManager->FindWeaponData(Master::m_pDataManager->get_SelectWeaponID(0)));


            // フラッシュ用ポイントライト
            auto flash = obj->add_Component<PointLight>();
            flash->set_Intensity(0.0f);
            flash->set_Range(0.0f);


            // スポットライト
            auto spotLight = obj->add_Component<SpotLight>();
            spotLight->set_SpotLightData(300.0f, 15.0f);
            spotLight->set_Intensity(15.0f);
            spotLight->set_LightColor(VEC3(1.0f));


            // レーザーサイト
            auto line = obj->add_Component<LineRenderer>();
            line->set_Color(VEC4(1.0f, 0.0f, 0.0f, 1.0f));
            line->set_Emissive(3.0f);
            line->set_Width(0.01f);
            line->set_Length(20.0f);

            // プレイヤーを親に設定
            obj->get_Transform().lock()->set_Parent(playerObj->get_Transform());
        }

        /* ロケットランチャー */
        {
            // マテリアル取得
            auto matPtr1 = Master::m_pResourceManager->FindMaterial("RocketLauncher");

            SetupMaterialInfo matInfo[1];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr1; // 体

            CreateModelInfo model;
            model.pRenderer = m_pRenderer;
            model.LODModels[0] = { "Resource/Model/Weapon/RocketLauncher/RocketLauncher_01.fbx", 0.0f };
            model.ObjTag = "RocketLauncher";
            model.IsAnim = false;
            model.MatNum = 1;
            model.SetupMaterial = matInfo;
            model.ShaderType = SHADER_TYPE::DEFERRED_STD_STATIC_N;
            model.ObjLayer = 91;
            auto obj = MeshFactory::CreateModel(model);

            // 破棄しない
            //obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);
            // 動的オブジェクトに設定
            obj->set_IsStatic(false);

            // ポーズ中は停止
            obj->set_IsUpdateAllowedDuringPause(false);

            // コンポーネントの追加
            weapon_2 = obj->add_Component<GunWeapon>(1);
            weapon_2->Setup(Master::m_pWeaponDataManager->FindWeaponData(Master::m_pDataManager->get_SelectWeaponID(1)));

            // フラッシュ用ポイントライト
            auto flash = obj->add_Component<PointLight>();
            flash->set_Intensity(0.0f);
            flash->set_Range(0.0f);

            // スポットライト
            auto spotLight = obj->add_Component<SpotLight>();
            spotLight->set_SpotLightData(300.0f, 15.0f);
            spotLight->set_Intensity(15.0f);
            spotLight->set_LightColor(VEC3(1.0f));

            // レーザーサイト
            auto line = obj->add_Component<LineRenderer>();
            line->set_Color(VEC4(1.0f, 0.0f, 0.0f, 1.0f));
            line->set_Emissive(3.0f);
            line->set_Width(0.01f);
            line->set_Length(20.0f);

            // プレイヤーを親に設定
            obj->get_Transform().lock()->set_Parent(playerObj->get_Transform());
            obj->get_Transform().lock()->set_Scale(VEC3(1.5f, 1.5f, 1.5f));
        }

        /* レーザーポインタ ビルボードの生成 */
        {
            // マテリアル取得
            auto matPtr = Master::m_pResourceManager->FindMaterial("LaserPointBillboard");

            SetupMaterialInfo matInfo[1];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr;

            CreateBillboradInfo billboard;
            billboard.pRenderer = m_pRenderer;
            billboard.Type = BILLBOARD_USAGE_TYPE::SIMPLE;
            billboard.ShaderType = SHADER_TYPE::FORWARD_UNLIT_STATIC;
            billboard.IsActive = false;
            billboard.MatNum = 1;
            billboard.MaterialData = matInfo;
            billboard.IsTransparent = true; // 透明度があり

            VEC3 pos = VEC3(-600.0f, 50.0f, 800.0f);

            //mat->m_DiffuseColor = VEC4(0.5, 0.5, 0.5, 1.0f);
            auto obj = MeshFactory::CreateBillboard(billboard);
            obj->get_Transform().lock()->set_Pos(pos);
            obj->get_Transform().lock()->set_Scale(0.1f, 0.1f, 0.1f);
            obj->set_Tag("LaserPointBillboard");
            //obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);
        }
    }

    auto camera = Master::m_pDataManager->get_CameraComponent().lock();
    
    // カメラ操作をオンに
    camera->set_IsControl(true);


    // プレイヤーに操作コンポーネントつける
    playerObj->get_Transform().lock()->set_Pos(-90.0f, 0.0f, 90.0f);  // プレイヤーの初期位置を設定
    playerObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    auto playerControl = playerObj->add_Component<PlayerController>(1); // コンポーネントの追加
    playerControl->Reset(); // パラメータ等リセットする

    // 体力コンポーネントの追加
    float hp = Master::m_pDataManager->get_PlayerHP();
    auto health = playerObj->get_Component<Health>();
    health->Reset();    // 前回までの値が残っているのでリセット
    health->set_MaxHP(hp);
    health->set_CrntHP(hp);


    //*****************************************************************************************
    //						プレイヤーに武器制御コンポーネントの追加し、
    //                                  GunWeaponを登録
    //*****************************************************************************************
    constexpr int WEAPON_MAX_SLOT = 2;  // プレイヤーの装備する最大武器スロット
    auto weaponControl = playerObj->add_Component<WeaponController>();
    weaponControl->Setup(*m_pRenderer, WEAPON_MAX_SLOT);
    weaponControl->RegisterWeapon(weapon_1, 0);
    weaponControl->RegisterWeapon(weapon_2, 1);
    weaponControl->StartingWeapon(0); // 0の武器を初期装備に
    playerControl->Start(*m_pRenderer);


    //*****************************************************************************************
    //						難易度によって空の色を変える
    //                        ちょっとした雰囲気作り  
    //*****************************************************************************************
    DIFFICULTY_LEVEL crntDiffLevel = Master::m_pDataManager->get_DifficultyLevel();
 //   auto dirLightObj = Master::m_pGameObjectManager->get_ObjectByTag("DirectionLight");
 //   auto dirLight = dirLightObj->get_Component<DirectionalLight>();

 //   // ステージ環境パラメータ
 //   StageEnvironmentParam stageEnvironmentParam;
 //   stageEnvironmentParam.dirLightColor     = VEC3(1.0f);              // ライトカラー
 //   stageEnvironmentParam.dirLightIntensity = 2.5f;                    // ライトの強さ
 //   stageEnvironmentParam.fogColor          = VEC3(0.6f, 0.7f, 0.8f);  // フォグカラー
 //   stageEnvironmentParam.fogStart          = 0.0f;                    // フォグ開始距離
 //   stageEnvironmentParam.fogEnd            = 0.0f;                    // フォグ最大距離
 //   stageEnvironmentParam.dofStart          = 300.0f;                  // 被写界深度開始距離
 //   stageEnvironmentParam.dofEnd            = 1500.0f;                 // 被写界深度最大距離

 //   switch (crntDiffLevel)
 //   {
 //   case UtilityData::DIFFICULTY_LEVEL::EASY:
 //       stageEnvironmentParam.dirLightColor = VEC3(0.7f, 0.7f, 0.7f);
 //       stageEnvironmentParam.dirLightIntensity = 3.5f;                   
 //       break;
 //   case UtilityData::DIFFICULTY_LEVEL::NORMAL:
 //       stageEnvironmentParam.dirLightColor = VEC3(0.7f, 0.7f, 0.7f);
 //       stageEnvironmentParam.dirLightIntensity = 3.5f;
 //       break;
 //   case UtilityData::DIFFICULTY_LEVEL::HARD:
 //       stageEnvironmentParam.dirLightColor = VEC3(0.6f, 0.4f, 0.4f);
 //       break;
 //   case UtilityData::DIFFICULTY_LEVEL::DISASTER:
 //       stageEnvironmentParam.dirLightColor = VEC3(0.5f, 0.2f, 0.4f);
 //       stageEnvironmentParam.fogColor = VEC3(0.6f, 0.7f, 0.8f);;
 //       stageEnvironmentParam.fogStart = 30.0f;
 //       stageEnvironmentParam.fogEnd = 180.0f;
 //       stageEnvironmentParam.dofStart = 30.0f;
 //       stageEnvironmentParam.dofEnd = 180.0f;
 //       break;

 //   case UtilityData::DIFFICULTY_LEVEL::IMPOSSIBLE:
 //       stageEnvironmentParam.dirLightColor = VEC3(0.9f, 0.0f, 0.0f);
 //       stageEnvironmentParam.fogColor = VEC3(0.25f, 0.0f, 0.0f);
 //       stageEnvironmentParam.fogStart = 30.0f;
 //       stageEnvironmentParam.fogEnd = 200.0f;
 //       stageEnvironmentParam.dofStart = 30.0f;
 //       stageEnvironmentParam.dofEnd = 200.0f;
 //       break;    

 //   default:
 //       break;
 //   }
 //   /* 
 //   * ディレクションライトの設定 
 //   */
 //   dirLight->set_LightColor(stageEnvironmentParam.dirLightColor);
	//dirLight->set_Intensity(stageEnvironmentParam.dirLightIntensity);

 //   /* 
 //   *  DOFとフォグの設定 
 //   */
 //   m_pRenderer->set_DofParam(stageEnvironmentParam.dofStart, stageEnvironmentParam.dofEnd);
 //   m_pRenderer->set_FogParam(stageEnvironmentParam.fogColor, stageEnvironmentParam.fogStart, stageEnvironmentParam.fogEnd);

    // ロード画面用スプライトをオフに
    auto obj = Master::m_pGameObjectManager->get_ObjectByTag("LoadScreen_Sp");
    if (obj)
    {
        obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }

    // ロード画面スプライトをオフに（プールへ返す）
	m_pLoadBackObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);


    //*****************************************************************************************
    //
    //                        爆速ロード防止のため、一時的に処理を停止させる
    //  
    //*****************************************************************************************
    Sleep(100);
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_LoadProcess Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Game_LoadProcess::Update(SceneManager *pOwner)
{
	if (m_IsLoad)
	{
        Master::m_pInputManager->StopInput(5);	// 入力を少しの間受け付けないようにする（シーン遷移の瞬間に入力されるのを防止）

		return c_GAME::c_GAME_PLAY;
	}

	return c_GAME::c_GAME_LOAD;
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_LoadProcess Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_LoadProcess::Draw(SceneManager *pOwner)
{
	// すでにロードされているなら返す
	if (m_IsLoad)
	{
		return;
	}

	//Master::m_pDirectWriteManager->DrawString("☆ロード中", VECTOR2::VEC2(940, 500));

	// ロード完了
	m_IsLoad = true;
}
