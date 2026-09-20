#pragma once
//--------------------------------------------------------------------------------------
//      * Includes *
//--------------------------------------------------------------------------------------
#include "Model.h"
#include "StateMachine.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SceneManager Class --- */
//
// 【?】シーンの管理
//
// ***************************************************************************************
class SceneManager
{
private:
	// ステートマシン
	StateMachine<SceneManager> m_StateMachine;
	bool m_IsClose;
	const class GameManager* m_pGameManager;

public:
	SceneManager();
	~SceneManager();

	bool Init(RendererEngine& renderer);
	void Update(RendererEngine& renderer);
	void Draw(RendererEngine& renderer);
	void Term(RendererEngine &renderer);

	void set_GM(const class GameManager& gm) { m_pGameManager = &gm; };
	const class GameManager* get_GM()const { return m_pGameManager; };

	/* シーンの終了フラグ */
	void OnSceneClose() { m_IsClose = true; }
	bool get_IsSceneClose()const { return m_IsClose; }
};

// シーンマネージャはタイトルシーンの中身を知る必要はない
// 逆もしかり
// タイトルの中にモード選択、設定、難易度...それらはシーンマネージャにとってどうでもいい