#include "pch.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "PhysicsEngine.h"
#include "RendererEngine.h"
#include "DirectWriteManager.h"
#include "Component_3DCamera.h"

using namespace RenderData;


//*---------------------------------------------------------------------------------------
//* @:GameManager Class 
//*【?】コンストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
GameManager::GameManager() :
	m_pSceneManager(nullptr),
	m_IsClose(false)
{
}


//*---------------------------------------------------------------------------------------
//* @:GameManager Class 
//*【?】デストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
GameManager::~GameManager()
{

}


//*---------------------------------------------------------------------------------------
//* @:GameManager Class 
//*【?】
//* 引数：1.RendererEngine
//* 返値：bool
//*----------------------------------------------------------------------------------------
bool GameManager::Init(RendererEngine& renderer)
{
	//
	// パイプラインの作成
	//
	if (!renderer.CreateRendererPipeline(RENDER_PIPELINE_STATE::DEFAULT))
	{
		return false;
	}

	//
	// シーン管理クラスの生成・初期化
	//
	m_pSceneManager = std::make_unique <SceneManager>();
	if (!m_pSceneManager->Init(renderer))
	{
		return false;
	}

	m_pSceneManager->set_GM(*this);

	//
	// 物理エンジンの作成・セットアップ
	//
	if (!Master::m_pPhysicsEngine->Setup())
	{
		return false;
	}

	return true;
}


//*---------------------------------------------------------------------------------------
//* @:GameManager Class 
//*【?】更新
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void GameManager::Update(RendererEngine& renderer)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	// シーンの更新
	m_pSceneManager->Update(renderer);

	// オブジェクト更新
	Master::m_pGameObjectManager->ObjectUpdate(renderer);

	// 物理エンジン更新
	Master::m_pPhysicsEngine->Update(deltaTime);

	// 衝突判定
	Master::m_pCollisionManager->CollisionProcess();

	// 遅延更新
	Master::m_pGameObjectManager->ObjectLateUpdate(renderer);
	
	// Tweenの更新
	Master::m_pTweenManager->Update(deltaTime);
	
	// scriptの更新
	Master::m_pScriptManager->Update(deltaTime);

	// UIの更新
	Master::m_pUIManager->Update(renderer);

	// 弾の更新
	Master::m_pBulletManager->Update(renderer);

	// アイテムの更新
	Master::m_pItemManager->Update(renderer);

	// シーンが終了していたら終わらせる
	if (m_pSceneManager->get_IsSceneClose())
	{
		m_IsClose = true;
	}
}


//*---------------------------------------------------------------------------------------
//* @:GameManager Class 
//*【?】描画
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void GameManager::Draw(RendererEngine& renderer)
{
	// レンダリングパイプラインの実行
	if (auto camera = Master::m_pDataManager->get_CameraComponent().lock()) {
		renderer.ExecuteDefaultRendererPipeline(RENDER_PIPELINE_STATE::DEFAULT, camera.get());
	}

	m_pSceneManager->Draw(renderer);
}



//*---------------------------------------------------------------------------------------
//* @:GameManager Class 
//*【?】終了
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void GameManager::Term(RendererEngine &renderer)
{
	m_pSceneManager->Term(renderer);
	m_pSceneManager.reset();

	Master::m_pGameObjectManager->Term(renderer);

	Master::m_pPhysicsEngine->Shutdown();
}

