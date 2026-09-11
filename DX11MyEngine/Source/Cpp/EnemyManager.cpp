#include "pch.h"
#include "EnemyManager.h"
#include "Component_EnemyController.h"

using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
EnemyManager::EnemyManager()
{
	m_Enemies.clear();
	m_Groups.clear();
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
EnemyManager::~EnemyManager()
{

}

//*---------------------------------------------------------------------------------------
//*【?】エネミーの登録
//*
//* [引数]
//* pEnemy : 登録するエネミーの参照ポインタ
//*
//* [返値]
//* 登録したエネミーのID 
//*----------------------------------------------------------------------------------------
EnemyID EnemyManager::RegisterEnemy(std::weak_ptr<GameObject> pEnemy)
{
	EnemyID enemyID;

	// 空いている場所を探し再利用
	for (uint32_t i = 0; i < m_Enemies.size(); i++)
	{
		EnemySlot& slot = m_Enemies[i];

		if (!slot.active)
		{
			slot.enemy = pEnemy;
			slot.active = true;

			enemyID.index = i;
			enemyID.generation = slot.generation;


			// エネミーコントローラー側にもIDを渡す
			auto enemyController = pEnemy.lock()->get_Component<EnemyController>();
			enemyController->set_EnemyID(enemyID, -1);

			return enemyID;
		}
	}

	// 空いていないなら追加
	EnemySlot enemySlot;
	enemySlot.active = true;
	enemySlot.enemy = pEnemy;

	// 配列に追加
	m_Enemies.push_back(enemySlot);

	enemyID.index = static_cast<uint32_t>(m_Enemies.size() - 1);
	enemyID.generation = enemySlot.generation;


	// エネミーコントローラー側にもIDを渡す
	auto enemyController = pEnemy.lock()->get_Component<EnemyController>();
	enemyController->set_EnemyID(enemyID, -1); // グループじゃないので -1 


	return enemyID;
}

//*---------------------------------------------------------------------------------------
//*【?】エネミーの登録解除
//*
//* [引数]
//* id : 登録解除するエネミーのID
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void EnemyManager::UnregisterEnemy(EnemyID id)
{
	if (!IsValidEnemy(id)) {
		return;
	}

	// 状態をリセット
	EnemySlot& slot = m_Enemies[id.index];
	slot.enemy.reset();
	slot.active = false;

	// 次にこのindexが使用された際に、
	// 古いEnemyIDと区別するため
	slot.generation++;
}


//*---------------------------------------------------------------------------------------
//*【?】エネミーグループの登録
//*
//* [引数]
//* pEnemies : 登録するエネミーの参照ポインタ配列
//*
//* [返値]
//* 登録したエネミーグループのID 
//*----------------------------------------------------------------------------------------
EnemyGroupID EnemyManager::RegisterEnemyGroup(std::vector<std::weak_ptr<GameObject>> pEnemies)
{
	EnemyGroupID groupID = m_NextGroupID;
	
	// グループを作成
	EnemyGroup group;
	group.id = groupID;
	
	// エネミーの登録
	for (auto& enemy : pEnemies)
	{
		EnemyID enemyID = RegisterEnemy(enemy);

		// エネミーコントローラー側にもIDを渡す
		auto enemyController = enemy.lock()->get_Component<EnemyController>();
		enemyController->set_EnemyID(enemyID, groupID);


		// エネミーをグループに追加
		group.enemies.push_back(enemyID);
	}

	// グループを追加
	m_Groups[groupID] = group;

	// グループを進める
	m_NextGroupID++;

	return groupID;
}


//*---------------------------------------------------------------------------------------
//*【?】エネミーグループの登録解除
//*
//* [引数]
//* id : 登録解除するエネミーグループのID
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void EnemyManager::UnregisterEnemyGroup(EnemyGroupID id)
{

}

//*---------------------------------------------------------------------------------------
//*【?】指定IDのエネミーが有効状態か
//*
//* [引数]
//* id : エネミーID
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
bool EnemyManager::IsValidEnemy(const EnemyID& enemyID)const
{
	// インデックス範囲
	if (enemyID.index >= m_Enemies.size()) {
		return false;
	}

	const EnemySlot& slot = m_Enemies[enemyID.index];
	
	// 非アクティブ
	if (!slot.active) {
		return false;
	}

	if (slot.generation != enemyID.generation) {
		return false;
	}

	// エネミー参照が切れている
	if (slot.enemy.expired()) {
		return false;
	}


	// 有効状態
	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】全敵数取得
//*----------------------------------------------------------------------------------------
int EnemyManager::GetAllEnemyCount()const
{
	int count = 0;
	for (int i = 0; i < m_Enemies.size(); i++)
	{
		const EnemySlot& slot = m_Enemies[i];

		if (slot.active && !
			slot.enemy.expired() ) 
		{
			count++;
		}
	}
	return count;
}

//*---------------------------------------------------------------------------------------
//*【?】グループ内エネミーの生存数
//*----------------------------------------------------------------------------------------
int EnemyManager::GetGroupAliveCount(EnemyGroupID groupID)const
{
	auto it = m_Groups.find(groupID);

	// グループが存在しない
	if (it == m_Groups.end())
		return 0;

	int count = 0;

	// 生存数確認
	for (const EnemyID& id : it->second.enemies)
	{
		if (IsValidEnemy(id))
		{
			count++;
		}
	}

	return count;
}

//*---------------------------------------------------------------------------------------
//*【?】指定IDのエネミーが倒されたか
//*----------------------------------------------------------------------------------------
bool EnemyManager::IsDead(uint32_t id)
{
	return  m_Enemies[id].active ? false : true;
}

//*---------------------------------------------------------------------------------------
//*【?】全ての敵が倒されたか
//*----------------------------------------------------------------------------------------
bool EnemyManager::IsAllDead()const
{
	return GetAllEnemyCount() == 0 ? true : false;
}

//*---------------------------------------------------------------------------------------
//*【?】指定グループが壊滅したかどうか
//*----------------------------------------------------------------------------------------
bool EnemyManager::IsGroupDestroyed(EnemyGroupID groupID)const
{
	return GetGroupAliveCount(groupID) == 0 ? true : false;
}