#pragma once
//--------------------------------------------------------------------------------------
//      * Includes *
//--------------------------------------------------------------------------------------

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:GameManager Class --- */
//
// 【?】ゲーム全体の管理
//		DXAppが実体を持つ
//
// ***************************************************************************************
class GameManager
{
private:
	SceneManager* m_pSceneManager;

	bool m_IsClose;	// 終了フラグ

public:
	GameManager();
	~GameManager();

	bool Init(RendererEngine& renderer);
	void Update(RendererEngine& renderer);
	void Draw(RendererEngine& renderer);
	void Term(RendererEngine &renderer);

	/* ゲームの終了フラグ */
	void OnGameClose() { m_IsClose = true; };
	bool get_IsGameClose()const { return m_IsClose; }
};

