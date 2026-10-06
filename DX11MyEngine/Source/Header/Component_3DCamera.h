#pragma once
#include "IComponent.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Camera3D Class --- */
//
//  ★継承：IComponent ★
//
// 【?】3Dカメラコンポーネント
//
// ***************************************************************************************
class Camera3D : public IComponent
{
public:
	enum class CAMERA_MODE
	{
		TPS,
		FPS,
	};

private:
	void ResolveObstacleCollision(RendererEngine& renderer);

	std::weak_ptr<class GameObject> m_pFocusObject;		// フォーカス対象
	VECTOR3::VEC3 m_FocusPoint;			// 注視位置
	VECTOR3::VEC3 m_UpVec;				// 上ベクトル　
    VECTOR3::VEC3 m_CameraPos;			// カメラの座標（色んなところで使うのでトランスフォームからではなく、ここに持つ）
    VECTOR3::VEC3 m_LookDir;			// 向いてる方向
	VECTOR3::VEC3 m_PosOffset;			// 対象からどのくらい離れて球面移動するか
	VECTOR3::VEC3 m_FocusOffset;		// フォーカス位置のオフセット
	Tool::VEC3_Shaker m_Shaker;			// シェイク用
	CAMERA_MODE m_CameraMode;

	float m_Angle_H;	// 水平方向アングル
	float m_Angle_V;	// 垂直方向アングル

	float m_Fov;			// デグリー
	float m_NearClipDist;	// 手前クリップ
	float m_FarClipDist;	// 奥クリップ


	bool m_IsControl;	// 操作フラグ
public:
	Camera3D(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~Camera3D();

	void Start(RendererEngine& renderer) override;		// 初期化

	void LateUpdate(RendererEngine &renderer)override;

	void CamraControl(RendererEngine& renderer);	// カメラ操作
	
	bool get_IsControl()const { return m_IsControl; }	// 操作フラグの取得
	void set_IsControl(bool _f) { m_IsControl = _f; }	// 操作フラグの設定

	void set_UpVec(const VECTOR3::VEC3& upVec);					// 上方向ベクトルの設定
	void set_FocusPoint(const VECTOR3::VEC3& focus);			// 注視点の設定
	void set_FocusObject(std::weak_ptr<class GameObject> pObj);	// 注視点オブジェクトの設定
	std::string get_FocusObjectTag()const;

	VECTOR3::VEC3 get_UpVec()const;			// 上方向ベクトル取得
	VECTOR3::VEC3 get_FocusPoint()const;	// 注視点取得

    float get_Angle_H()const { return m_Angle_H; }			// 水平アングル取得	
	float get_Angle_V()const { return m_Angle_V; }			// 垂直アングル取得
    void set_Angle_H(float angle) { m_Angle_H = angle; }	// 水平アングル設定	
    void set_Angle_V(float angle) { m_Angle_V = angle; }	// 垂直アングル設定

	/* オフセット関連 */
    void set_PosOffset(const VECTOR3::VEC3 &offset);	// 座標オフセット設定
    VECTOR3::VEC3 get_PosOffset()const;					// 座標オフセット取得
    void set_FocusOffset(const VECTOR3::VEC3 &offset);	// 注視点オフセットの設定
    VECTOR3::VEC3 get_FocusOffset()const;				// 注視点オフセットの取得


	void set_Fov(float _fov) { m_Fov = _fov; };				// 視野の設定
	void set_Near(float _near) { m_NearClipDist = _near; }; // 手前クリップの設定
	void set_Far(float _far) { m_FarClipDist = _far; };		// 奥クリップの設定

	float get_Fov()const { return m_Fov; };				    // 視野の設定
	float get_Near()const { return m_NearClipDist; };	    // 手前クリップの設定
	float get_Far()const { return m_FarClipDist; };		    // 奥クリップの設定

	VECTOR3::VEC3 get_CameraPos()const;	// 座標取得
    VECTOR3::VEC3 get_LookDir()const;		// 注視方向取得

	void RequestShake(float _duration, const VECTOR3::VEC3& _strength);	// カメラシェイクの設定
	void DistanceDecay(float _duration, const VECTOR3::VEC3& _strength, const VECTOR3::VEC3& _shakePos, float _maxRange);	// カメラシェイクの設定（距離減衰ver）


	/// <summary>
	/// ビュー変換行列の取得
	/// </summary>
	DirectX::XMMATRIX get_ViewMatrix()const;
};

