#include "pch.h"
#include "TitleScene_StateHeader.h"
#include "RendererEngine.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "Component_SpriteRenderer.h"
#include "Component_ButtonUI.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_PlayerController.h"
#include "Component_BoxCollider.h"
#include "Component_3DCamera.h"
#include "Component_Transform.h"
#include "Component_TrailRenderer.h"
#include "Component_LineRenderer.h"
#include "Component_DirectionalLight.h"
#include "Component_AssultRifle.h"
#include "Component_Health.h"
#include "Component_PointLight.h"
#include "Component_Faction.h"
#include "Component_Physics.h"
#include "Component_RigidBody.h"
#include "GameObject.h"
#include "MeshFactory.h"
#include "InputFactory.h"

using namespace SceneStateEnums;
using namespace UtilityData;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace GIGA_Engine;;

//*---------------------------------------------------------------------------------------
//* @:c_Title_LoadProcess Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_LoadProcess::OnEnter(SceneManager *pOwner)
{
	// すでにロードされているなら返す
	if (m_IsLoad) {
		return;
	}

    // *************************************************************************************************
    /**  UI管理の初期化 **/
    // *************************************************************************************************
    if (!Master::m_pUIManager->Init(*m_pRenderer))
    {
        MessageBoxA(NULL, "UI管理クラスの初期化に失敗しました", "GameLoad", MB_OK);
        assert(false);
    }

    /* カメラの作成 */
    {
        auto obj = Instantiate3D(std::move(std::make_shared<GameObject>()), false);
        if (obj == nullptr)
        {
            assert(false);
        }
        obj->set_LayerRank(LAYER_RANK_CAMERA);
        obj->set_Tag("MainCamera");
        obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);
        obj->get_Transform().lock()->set_Pos(0.0f, 0.0f, 1000.0f);
        m_pCameraComp = obj->add_Component<Camera3D>(); // カメラコンポーネントの追加
        m_pCameraComp->set_IsControl(false);    // 操作フラグをオフに

        // ポーズ中は停止
        obj->set_IsUpdateAllowedDuringPause(false);
        
        // データ管理にカメラコンポーネントを渡す
        Master::m_pDataManager->set_CameraComponent(m_pCameraComp);
    }

    // ライトにカメラのTransformを持たせる
    Master::m_pLightManager->set_CameraTransform(m_pCameraComp->get_OwnerObj().lock()->get_Transform());

    // ロード画面用用スプライトを作る **********************************************
	float width = static_cast<float>(m_pRenderer->get_ScreenWidth());
	float height = static_cast<float>(m_pRenderer->get_ScreenHeight());
	UIData::RectTransformData rectData;
    rectData._size = VEC2(width, height);
	UIData::SpriteUIData spriteData;
    spriteData._tag = "LoadSprite";
    spriteData._imagePath = "Resource/Texture/Title/Title_Load.png";
    spriteData._layerRank = 101;
    m_pLoadBackObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectData, spriteData);
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_LoadProcess Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_LoadProcess::OnExit(SceneManager *pOwner)
{
    //// ロード画面用スプライトオフ
    //m_pLoadBackSprite->get_OwnerObj().lock()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    if (m_IsMatLoad)
    {
        return;
    }

    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/Cursor_L.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/Cursor_R.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/Decoration01.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/ConfigBackGround.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/Frame08.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/Frame07.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/Frame10.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/Manual.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/Radar.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/HPBar_Ver2_ai.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/BulletGage.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/GunIcon_AR_01.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/GunIcon_RL_01.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/MissionCleared.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/UI/MissionFailed.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Particle/Particle04_bokashi_hard.png");
    Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/Game_Load_01.png");

    // CSVからマテリアルデータの読み込み
    if (!Master::m_pResourceManager->ImportCSV_AllMaterialData("Resource/Excel_Param/MaterialParam.csv"))
    {
        MessageBoxA(NULL, "CSVの読み込みに失敗", "GameLoad", MB_OK);
        assert(false);
    }

    // マテリアルの作成 (今後CSVで読み込むようにする)
    {
        /* クモ */
        {
            Material mat[4];
            mat[0].m_DiffuseMap.Texture = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Model/fbx/textures/Spinnen_Bein_tex.jpg");
            mat[0].m_NormalMap.Texture = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Model/fbx/textures/haar_detail_NRM.jpg");
            mat[0].m_DiffuseColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[0].m_SpecularColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[0].m_SpecularPower = 150.0f;

            // マテリアル登録
            Master::m_pResourceManager->RegisterMaterialData("Spider_1", mat[0]);

            mat[1].m_DiffuseMap.Texture = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Model/fbx/textures/Spinnen_Bein_tex.jpg");
            mat[1].m_DiffuseColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[1].m_SpecularColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[1].m_SpecularPower = 150.0f;

            // マテリアル登録
            Master::m_pResourceManager->RegisterMaterialData("Spider_2", mat[1]);

            mat[2].m_DiffuseMap.Texture = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Model/fbx/textures/Spinnen_Bein_tex.jpg");
            mat[2].m_DiffuseColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[2].m_SpecularColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[2].m_SpecularPower = 150.0f;

            // マテリアル登録
            Master::m_pResourceManager->RegisterMaterialData("Spider_3", mat[2]);

            mat[3].m_DiffuseMap.Texture = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Model/fbx/textures/SH3.png");
            mat[3].m_NormalMap.Texture = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Model/fbx/textures/haar_detail_NRM.jpg");
            mat[3].m_DiffuseColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[3].m_SpecularColor = VEC4(1.0f, 1.0f, 1.0f, 1.0f);
            mat[3].m_SpecularPower = 100.0f;

            // マテリアル登録
            Master::m_pResourceManager->RegisterMaterialData("Spider_4", mat[3]);
        }

        /* スカイボックス */
        {
            Material mat;
            mat.m_DiffuseMap.Texture = Master::m_pResourceManager->LoadDDS_CubeMap_Texture(L"Resource/Texture/CubeMap/skybox_01.dds");

            // マテリアル登録
            Master::m_pResourceManager->RegisterMaterialData("SkyBox01", mat); 
            
            mat.m_DiffuseMap.Texture = Master::m_pResourceManager->LoadDDS_CubeMap_Texture(L"Resource/Texture/CubeMap/skybox_02.dds");
            
            // マテリアル登録
            Master::m_pResourceManager->RegisterMaterialData("SkyBox02", mat);
        }
    }

    std::shared_ptr<GameObject> pPlayerObj;
    /* プレイヤー モデルの生成 */
    {
        // マテリアル取得
        auto matPtr1 = Master::m_pResourceManager->FindMaterial("Soldier_body");
        auto matPtr2 = Master::m_pResourceManager->FindMaterial("Soldier_head");

        SetupMaterialInfo matInfo[3];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr1; // 体

        matInfo[1].Index = 1;
        matInfo[1].pMaterialData = matPtr1; // ヘルメット（体）

        matInfo[2].Index = 2;
        matInfo[2].pMaterialData = matPtr2; // 頭

        CreateModelInfo model;
        model.pRenderer = m_pRenderer;
        model.LODModels[0] = { "Resource/Model/Ranger/Swat_01.fbx", 0.0f };
        model.ObjTag = "Player";
        model.IsAnim = true;
        model.MatNum = 3;
        model.SetupMaterial = matInfo;
        model.ObjLayer = LAYER_RANK_PLAYER;
        model.Shadow_ShaderType = SHADER_TYPE::POST_SHADOWMAP;
        model.ShaderType = SHADER_TYPE::DEFERRED_STD_SKINNED_N;
        model.IsActive = false;
        pPlayerObj = MeshFactory::CreateModel(model);
        pPlayerObj->get_Component<MyTransform>()->set_Scale(1.0f, 1.0f, 1.0f);
        pPlayerObj->get_Component<SkinnedMeshAnimator>()->set_IsAnim(true);
        pPlayerObj->get_Component<SkinnedMeshAnimator>()->set_AnimIndex(INT_CAST(PlayerData::PLAYER_RANGER_ANIM_ID::RIFLE_AMING_IDLE));
        pPlayerObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);        // 破棄しない
        pPlayerObj->set_IsStatic(false);        // 動的オブジェクト
        pPlayerObj->get_Transform().lock()->set_Pos(-900.0f, 0.0f, 900.0f);

        // ポーズ中は停止
        pPlayerObj->set_IsUpdateAllowedDuringPause(false);

        float hp = Master::m_pDataManager->get_PlayerHP();

        // 体力コンポーネントの追加
        auto health = pPlayerObj->add_Component<Health>();
        health->set_MaxHP(hp);
		health->set_CrntHP(hp);

        // 派閥コンポーネントの追加
        auto faction = pPlayerObj->add_Component<Faction>();
        faction->set_Faction(FACTION::PLAYER);

        auto physics = pPlayerObj->add_Component<Physics>();
        //physics->set_GravityScale(0.0f);


        // コライダーの追加
        auto collider = pPlayerObj->add_Component<BoxCollider>();
        collider->set_Size(VEC3(0.5f, 1.0f, 0.5f));
        collider->set_Center(VEC3(0.0f, 1.0f, 0.0f));

        // コリジョンのカテゴリ
        collider->set_CollisionCategory(COLLISION_CATEGORY::PLAYER);

        // 衝突マスクの設定
        collider->set_CollisionResponse(COLLISION_CATEGORY::ENEMY, COLLISION_RESPONSE::RESPONSE_IGNORE);                // エネミー
        collider->set_CollisionResponse(COLLISION_CATEGORY::ITEM, COLLISION_RESPONSE::RESPONSE_OVERLAP);                // アイテム
        collider->set_CollisionResponse(COLLISION_CATEGORY::BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);              // 建物
        collider->set_CollisionResponse(COLLISION_CATEGORY::DESTRUCTION_BUILDING, COLLISION_RESPONSE::RESPONSE_BLOCK);  // 破壊可能建物

        // コライダーの登録
        Master::m_pCollisionManager->RegisterCollider(collider);

        // カメラのフォーカスオブジェクトに設定
        m_pCameraComp->set_FocusObject(pPlayerObj);
    }

    /* ディレクションライトの生成(Cubuで分かりやすく) */
    {
        // マテリアル取得
        auto matPtr = Master::m_pResourceManager->FindMaterial("DirLight");

        SetupMaterialInfo matInfo[1];
        matInfo[0].Index = 0;
        matInfo[0].pMaterialData = matPtr;

        CreateUtilityMeshInfo mesh;
        mesh.pRenderer = m_pRenderer;
        mesh.Type = UTILITY_MESH_TYPE::CUBE;
        mesh.ObjTag = "DirectionLight";
        mesh.MatNum = 1;
        mesh.MaterialData = matInfo;

        auto obj = MeshFactory::CreateUtilityMesh(mesh);
        //auto obj = Instantiate3D(std::move(std::make_shared<GameObject>()));
        obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);// 破棄しない
		obj->set_LayerRank(LAYER_RANK_DIRLIGHT);
        auto light = obj->add_Component<DirectionalLight>();
        light->set_LightColor(VEC3(0.9f, 0.9f, 0.9f));
        light->set_Intensity(3.0f);
        light->set_LightCameraTrackingObj(m_pCameraComp->get_OwnerObj().lock());
        light->Start(*m_pRenderer);

        obj->get_Transform().lock()->set_Pos(VEC3(0.0f, 1000.0f, -1000.0f));
        obj->get_Transform().lock()->set_Scale(VEC3(30.0, 30.0, 80.0));
        obj->get_Transform().lock()->set_RotateToDeg(VEC3(30.0f, 150.0f, 0.0f));
    }

    // オブジェクトを非アクティブに（プールに返す）
    m_pLoadBackObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

    m_IsMatLoad = true;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_LoadProcess Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Title_LoadProcess::Update(SceneManager *pOwner)
{
    // ロード済みならメインメニューへ
    if (m_IsLoad)
    {
        return c_TITLE::c_TITLE_MAIN_MENU;
    }


	return c_TITLE::c_TITLE_LOAD_PROCESS;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_LoadProcess Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_LoadProcess::Draw(SceneManager *pOwner)
{
    // すでにロードされているなら返す
    if (m_IsLoad)
    {
        return;
    }

	//Master::m_pDirectWriteManager->DrawString("☆ロード中", VECTOR2::VEC2(40.0f, 500.0f), "White_40_STD");

    // ロード完了
    m_IsLoad = true;
}
