#pragma once
//#include "Object.h"
#include "Component_Transform.h"
#include "Component_RectTransform.h"
#include "ConstantPhysicsData.h"


enum class OBJECT_CATEGORY
{
	UNKNOWN,
	PLAYER,         // プレイヤー
	ALLY_NPC,       // 味方（隊員など）
	ENEMY,          // 敵
	VEHICLE,        // 乗り物
	ITEM,           // 回復・武器アイテム
	OBSTACLE        // 破壊可能な建物など
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:GameObject3D Class --- */
//
//  ★継承：Object ★
//
// 【?】ゲームにに存在するもの全ての基底クラス
//
//	※enable_shared_from_thisについて※
//		コンポーネントに対してオーナーであるthisを渡す際に、
//		そのままmake_sharedと渡すことはできない。
//		なので、安全に取得するにはenable_shared_from_thisを継承させ、
//		shared_from_this()メソッドを使用して渡すことによって解決できるらしい。
//		コンストラクタが呼ばれた後に適用されるため、コンストラクタ内でコンポーネントの追加は出来ない。
// 
//  ※3/12
//　　　コンポーネントを連想配列で持つようにした。ダイナミックキャストするよりも多分こっちの方が速い
//　　　ただし、親クラスから子クラスを取得することは出来ない。
//		↓みたいなこと
//		obj->add_Component<BoxCollider>();
//      obj->get_Component<Collider>();
// 
//	※検証した結果
//		コンポーネントの数が多いほど、unordered_mapの力が発揮されるけど、少ないとvectorでdynamic_cast回してたのと
//		ほぼ同等の速度になる。Updateを回すだけなら多分vectorのほうが速い。
//		なので更新はvector、取得時にはunordered_mapとかでもいいかもしれない。
//
// ***************************************************************************************
class GameObject : public std::enable_shared_from_this<GameObject> , public Object
{
private:	
	std::unordered_map<std::type_index, std::shared_ptr<IComponent>> m_pComponentMap;	// コンポーネント配列

    std::shared_ptr<MyTransform> m_pTransform;	// トランスフォームコンポーネントはデフォルトで持つ（2DならRectTransformを持つ）

	bool m_IsCalcUpdate;	// 更新処理がすでに呼ばれたかどうか
	bool m_IsStatic;		// 静的か動的か
	bool m_IsUpdateAllowedDuringPause;	// ポーズ中でも更新可能か
	bool m_IsDrawAllowedDuringPause;	// ポーズ中でも描画可能か

	/* オブジェクトマネージャをフレンドとして登録 */
	friend class GameObjectManager;
public:
    GameObject();
    virtual ~GameObject();

	virtual void Update(RendererEngine& renderer) {};
	virtual void Draw(RendererEngine& renderer) {};

	void OnCollisionEnter(const PhysicsData::CollisionInfo& _other);	// 当たった瞬間
	void OnCollisionStay(const  PhysicsData::CollisionInfo& _other);	// 当たっている間
	void OnCollisionExit(const  PhysicsData::CollisionInfo& _other);	// 離れた瞬間
	void OnTriggerEnter(const   PhysicsData::CollisionInfo& _other);		// トリガー 当たった瞬間
	void OnTriggerStay(const    PhysicsData::CollisionInfo& _other);		// トリガー 当たっている間
	void OnTriggerExit(const    PhysicsData::CollisionInfo& _other);		// トリガー 離れた瞬間

	/* 静的フラグ */
	bool get_IsStatic()const { return m_IsStatic; }
	void set_IsStatic(bool _flag) { m_IsStatic = _flag; }

	/* ポーズフラグ */
	void set_IsUpdateAllowedDuringPause(bool _flag) { m_IsUpdateAllowedDuringPause = _flag; }
	bool get_IsUpdateAllowedDuringPause()const { return m_IsUpdateAllowedDuringPause; }
	void set_IsDrawAllowedDuringPause(bool _flag) { m_IsDrawAllowedDuringPause = _flag; }
	bool get_IsDrawAllowedDuringPause()const { return m_IsDrawAllowedDuringPause; }

	// ****************************************************************************************************************************************
	/* コンポーネント関連 */
	// ****************************************************************************************************************************************
	/// <summary>
	/// コンポーネントの更新
	/// </summary>
	void ComponentUpdate(RendererEngine& renderer);

	/// <summary>
	/// 更新後の更新
	/// </summary>
	/// <param name="renderer"></param>
	void ComponentLateUpdate(RendererEngine &renderer);

	/// <summary>
	/// コンポーネントの描画
	/// </summary>
	/// <param name="renderer"></param>
	void ComponentRender(RendererEngine& renderer);


	/// <summary>
	/// Transformの取得
	/// </summary>
	/// <returns></returns>
	std::weak_ptr<MyTransform> get_Transform()const;

	/// <summary>
	/// RectTransformの取得
	/// </summary>
	/// <returns></returns>
	std::weak_ptr<RectTransform> get_RectTransform()const;


	/// <summary>
	/// 読み取り専用Transform取得
	/// </summary>
	/// <returns></returns>
	const MyTransform* get_TransformConst()const;

	/// <summary>
	/// 読み取り専用RectTransform取得
	/// </summary>
	/// <returns></returns>
	const RectTransform* get_RectTransformConst()const;


	/// <summary>
	/// コンポーネントのリストを取得
	/// </summary>
	/// <returns></returns>
	std::unordered_map<std::type_index, std::shared_ptr<IComponent>> get_ComponentMap()const;

	/// <summary>
	/// コンポーネントの取得
	/// ※ 型が完全に一致していないと取得できない
	/// 　 子クラスから親クラスへの変換取得などは不可
	/// </summary>
	/// <typeparam name="T">取得するコンポーネントの型</typeparam>
	/// <returns></returns>
	template<typename T>
	std::shared_ptr<T> get_Component() const
	{
		auto it = m_pComponentMap.find(std::type_index(typeid(T)));

		if (it != m_pComponentMap.end())
		{
			return std::static_pointer_cast<T>(it->second);
		}

		return nullptr;
	};


	/// <summary>
	/// コンポーネントの追加
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	std::shared_ptr<T> add_Component(int updateRank = 100)
	{
		auto typeIdx = std::type_index(typeid(T));
		auto it = m_pComponentMap.find(typeIdx);

		// 既に存在するならキャストして返す
		if (it != m_pComponentMap.end())
		{
			return std::static_pointer_cast<T>(it->second);
		}

		// 生成
		auto newComp = std::make_shared<T>(shared_from_this(), updateRank);
		m_pComponentMap[typeIdx] = newComp;

		return newComp;
	}


	/// <summary>
	/// コンポーネントを外す
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	void remove_Component()
	{
		m_pComponentMap.erase(std::type_index(typeid(T)));
	}
};

