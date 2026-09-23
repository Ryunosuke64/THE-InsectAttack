#pragma once
#include "IComponent.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Collider Class --- */
//
//  ★ 継承 ★
//
// 【?】コライダーコンポーネントの基底クラス
//
// ***************************************************************************************
class Collider : public IComponent
{
protected:
	bool m_IsEnable;								// 使用するかどうか
	bool m_IsTrigger;								// 衝突判定のみ取るかどうか（falseなら物理も判定をする）
	bool m_IsHit;									// 現在衝突しているかどうか
	bool m_IsConvex;								// 凸形状か
	bool m_IsStatic;								// 静的かどうか（建物など動かないもの）
	UtilityData::COLLIDER_TYPE m_ColliderType;		// コライダーの種類
	class MyTransform *m_pTransform;				// 自身のトランスフォームポインタ
	VECTOR3::VEC3 m_Center;							// コライダーの中心位置
	UtilityData::COLLISION_CATEGORY m_CategoryBits;	// 衝突判定を分けるためのカテゴリー
	unsigned m_CollisionBitMask;					// 衝突判定を分けるためのビットマスク
	unsigned m_ResponseBitMask;						// 押し出し処理を行うかどうかのビットマスク
	bool m_IsDrawDebugMesh;							// デバッグ用メッシュを表示するか


	std::weak_ptr<class RigidBody> m_pRigidBody;

public:
	Collider(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~Collider();

	void set_RigidBody(std::weak_ptr<class RigidBody> rb) { m_pRigidBody = rb; };

	// 各コライダーごとにシェイプ情報を生成させる
	virtual PhysicsData::PhysicsShapeDesc GetShapeDesc() const = 0;

	// 使用フラグ
	void set_IsEnable(bool _flag) { m_IsEnable = _flag; }
	bool get_IsEnable()const { return m_IsEnable; }

	// 物理判定フラグ
	void set_IsTrigger(bool _flag) { m_IsTrigger = _flag; }
	bool get_IsTrigger()const { return m_IsTrigger; }

	// 中心位置
	void set_Center(const VECTOR3::VEC3 &_vIn) { m_Center = _vIn; }
	VECTOR3::VEC3 get_Center()const { return m_Center; }

	UtilityData::COLLIDER_TYPE get_ColliderType()const { return m_ColliderType; }

	class MyTransform* get_Transform() { return m_pTransform; };

	/* 衝突したかどうか */
	void set_IsHit(bool _flag) { m_IsHit = _flag; }
	bool get_IsHit()const { return m_IsHit; }

	/* 静的かどうか */
	void set_IsStatic(bool _flag) { m_IsStatic = _flag; }
	bool get_IsStatic()const { return m_IsStatic; }

	/* 凸形状かどうか */
	void set_IsConvex(bool _flag) { m_IsConvex = _flag; }
	bool get_IsConvex()const { return m_IsConvex; }

	/* デバッグメッシュ表示するか */
	void set_IsDrawDebugMesh(bool _flag) { m_IsDrawDebugMesh = _flag; }
	bool get_IsDrawDebugMesh()const { return m_IsDrawDebugMesh; }


	/* 衝突のカテゴリ 自身のタイプ */
	void set_CollisionCategory(UtilityData::COLLISION_CATEGORY _category) { m_CategoryBits = _category; }
	UtilityData::COLLISION_CATEGORY get_CollisionCategory()const { return m_CategoryBits; }

	/* 衝突判定のビットマスク */
	/// <summary> 一括設定</summary>
	void set_CollisionBitMask(unsigned _mask);
	/// <summary> 特定のカテゴリを追加 </summary>
	void add_CollisionBitMask(UtilityData::COLLISION_CATEGORY _category);
	/// <summary> 特定のカテゴリを除外 </summary>
	void remove_CollisionBitMask(UtilityData::COLLISION_CATEGORY _category);


	/// <summary> 衝突判定用ビットマスクの取得</summary>
	unsigned get_CollisionBitMask()const { return m_CollisionBitMask; }
	/// <summary> 衝突応答用のビットマスク取得 </summary>
	unsigned get_ResponseBitMask()const { return m_ResponseBitMask; }
	/// <summary> 指定したカテゴリに対応した衝突応答を返す </summary>
	UtilityData::COLLISION_RESPONSE get_Response(UtilityData::COLLISION_CATEGORY _otherCategory)const;
	/// <summary> 衝突の応答用ビットマスクの設定 </summary>
	void set_ResponseBitMask(unsigned _mask) { m_ResponseBitMask = _mask; }
	/// <summary>衝突の応答（判定のみか、押し出しも行うか）を設定 </summary>
	/// <param name="_category">衝突のカテゴリ</param>
	/// <param name="_response">判定時の応答</param>
	void set_CollisionResponse(UtilityData::COLLISION_CATEGORY _category, UtilityData::COLLISION_RESPONSE _response);
};

