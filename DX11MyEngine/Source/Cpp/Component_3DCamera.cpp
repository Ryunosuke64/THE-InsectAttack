#include "pch.h"
#include "Component_3DCamera.h"
#include "GameObject.h"
#include "InputFactory.h"
#include "DirectWriteManager.h"
#include "RendererEngine.h"
#include <cmath>

using namespace Input;
using namespace UtilityData;
using namespace VECTOR3;
using namespace VECTOR2;

using namespace DirectX;

#define CAMERA_ANGLE_SPEED		0.05f		// カメラの方向転換スピード
#define CAMERA_MOVE_FACTOR		0.5f		// 
#define CAMERA_POS_OFFSET		3.7f		// 位置のオフセット
#define CAMERA_FOCUS_Y_OFFSET	1.9f		// 中視点オブジェクトのオフセット

//*---------------------------------------------------------------------------------------
//* @:Camera3D Class 
//*【?】コンストラクタ
//* 引数：1.オーナーオブジェクト
//* 引数：2.更新レイヤー
//*----------------------------------------------------------------------------------------
Camera3D::Camera3D(std::weak_ptr<GameObject> pOwner, int updateRank) : IComponent(pOwner, updateRank),
m_FocusPoint({ 0.0f,0.0f,0.0f }),
m_UpVec({ 0.0f,1.0f,0.0f }),
m_CameraPos({ 0.0f,0.0f,0.0f }),
m_LookDir({ 0.0f,0.0f,0.0f }),
m_Angle_H(1.57f),
m_Angle_V(0.f),
m_Fov(45.0f),
m_NearClipDist(0.4f),
m_FarClipDist(2000.0f),
m_IsControl(true),
m_CameraMode(CAMERA_MODE::TPS),
m_Shaker()
{
	this->set_Tag("Camera3D"); 
	// 最初の更新前にも、初期角度に対応する有効な視線方向を持たせる。
	m_LookDir = VEC3(
		-cosf(m_Angle_V) * cosf(m_Angle_H),
		-sinf(m_Angle_V),
		-cosf(m_Angle_V) * sinf(m_Angle_H));
	m_PosOffset.x = CAMERA_POS_OFFSET;
	m_PosOffset.y = CAMERA_POS_OFFSET;
	m_PosOffset.z = CAMERA_POS_OFFSET;
	m_FocusOffset.x = 0.0f;
	m_FocusOffset.y = CAMERA_FOCUS_Y_OFFSET;
	m_FocusOffset.z = 0.0f;
}


//*---------------------------------------------------------------------------------------
//* @:Camera3D Class 
//*【?】デストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
Camera3D::~Camera3D()
{

}

//*---------------------------------------------------------------------------------------
//* @:Camera3D Class 
//*【?】初期化
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void Camera3D::Start(RendererEngine& renderer)
{
	// デフォルトFOVの設定
	Master::m_pDataManager->set_DefaultFov(m_Fov);
}


//*---------------------------------------------------------------------------------------
//* @:Camera3D Class 
//*【?】カメラの操作など
//*		Updateに書くと更新順で不都合が出る可能性があるので、
//*		明示的にこれを呼び出す。
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void Camera3D::LateUpdate(RendererEngine &renderer)
{
	if (Master::m_pDataManager->get_IsPause())return;	// TODO:ポーズ中なら返す

	m_IsControl = Master::m_pDataManager->get_IsCameraControl();

	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	// 操作フラグがオフなら操作できない
	if (m_IsControl)
	{
		CamraControl(renderer);
	}


	if (m_pFocusObject.expired())
	{
		return;
	}

	VEC3 focusObjPos = m_pFocusObject.lock()->get_Transform().lock()->get_VEC3ToPos();

	// 注視点を設定
	//m_FocusPoint = VEC3::Lerp(m_FocusPoint, focusObjPos, CAMERA_MOVE_FACTOR);	// ガタガタする
	//float followSpeed = 10.0f;
	//m_FocusPoint = VEC3::Lerp(m_FocusPoint, focusObjPos, followSpeed * deltaTime);

	// 注フォーカスされるオブジェクトにオフセット位置を足したものを、注視点とする
	m_FocusPoint = focusObjPos + m_FocusOffset;

	// 注視点からカメラへの方向ベクトルを作る
	VEC3 focusToCamera;
	focusToCamera.x = cosf(m_Angle_V) * cosf(m_Angle_H);
	focusToCamera.y = sinf(m_Angle_V);
	focusToCamera.z = cosf(m_Angle_V) * sinf(m_Angle_H);

	// カメラ位置決定
	m_CameraPos = m_FocusPoint + focusToCamera * m_PosOffset;

	// そのままでは「注視点->カメラ」になっているため、反転させる
    m_LookDir = -focusToCamera;


	//VEC3 forward;
	//forward.x = -cosf(m_Angle_V) * cosf(m_Angle_H);
	//forward.y = -sinf(m_Angle_V);
	//forward.z = -cosf(m_Angle_V) * sinf(m_Angle_H);

	//forward = forward.Normalize();

	//m_CameraPos = m_FocusPoint;
	//m_FocusPoint = m_CameraPos + forward;
	//m_LookDir = forward;

	// シェイクの適用
	if (Master::m_pDataManager->get_UserConfigData()._isCameraShake)
	{
		// シェイクの更新
		m_Shaker.Update(deltaTime);
		m_CameraPos = m_Shaker.Apply(m_CameraPos);
	}

	ResolveObstacleCollision(renderer);


	// カメラの位置
	m_pOwner.lock()->get_Transform().lock()->set_Pos(m_CameraPos);
}


void Camera3D::ResolveObstacleCollision(RendererEngine& renderer)
{
	if (!Master::m_pPhysicsEngine)
	{
		return;
	}

	constexpr float collisionSkin = 0.02f;
	constexpr int maxPushOutIterations = 8;
	const unsigned hitMask = UINT_CAST(COLLISION_CATEGORY::BUILDING) |
		UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING);
	const unsigned group = UINT_CAST(COLLISION_CATEGORY::PLAYER);
	const auto focusObject = m_pFocusObject.lock();
	auto& physics = *Master::m_pPhysicsEngine;

	// ニアクリップ面の四隅を含む球で判定し、画面の端が壁に入るのも防ぐ。
	const float aspect = renderer.get_ScreenHeight() > 0 ?
		static_cast<float>(renderer.get_ScreenWidth()) / renderer.get_ScreenHeight() : 1.0f;
	const float halfFov = XMConvertToRadians(std::clamp(m_Fov, 1.0f, 179.0f)) * 0.5f;
	const float halfHeight = std::max(m_NearClipDist, 0.0f) * tanf(halfFov);
	const float radius = std::max(0.2f, sqrtf(m_NearClipDist * m_NearClipDist +
		halfHeight * halfHeight * (1.0f + aspect * aspect)));

	const auto pushOut = [&](VEC3& position)
	{
		for (int i = 0; i < maxPushOutIterations; ++i)
		{
			PhysicsData::CollisionInfo contact{};
			if (!physics.GetSpherePenetration(position, radius, group, hitMask,
				&contact, focusObject.get()))
			{
				break;
			}

			// 法線は障害物から球へ向く。補間せず、めり込み量を全て解消する。
			position += contact.hitNormal * (contact.penetrationDepth + collisionSkin);
		}
	};

	// 開始点も押し出してから判定する。重なったままのSphereCastは当たりを保証しない。
	VEC3 castStart = m_FocusPoint;
	pushOut(castStart);

	// 注視点からシェイク適用後の予定位置へ判定し、壁の手前でカメラを止める。
	PhysicsData::SweepHitInfo sweepHit{};
	if (physics.SphereCast(castStart, m_CameraPos, radius, group, hitMask,
		&sweepHit, focusObject.get()))
	{
		const VEC3 movement = m_CameraPos - castStart;
		const float distance = movement.Length();
		const float safeDistance = std::max(0.0f, distance * sweepHit.hitFraction - collisionSkin);
		m_CameraPos = castStart + movement.Normalize() * safeDistance;
	}

	// 初期配置や複数の壁との接触による、残っためり込みを解消する。
	pushOut(m_CameraPos);

	// 押し出しで注視点を越えても、操作で決めたm_LookDirは変更しない。
}

void Camera3D::CamraControl(RendererEngine& renderer)
{
	// マウスの移動量の差を取得する
	LONG lX = Master::m_pInputManager->GetMousePosSlopeX();
	LONG lY = Master::m_pInputManager->GetMousePosSlopeY();
	const float MOUSE_SCALE = 0.0001f;	// そのままでは大きすぎるため、スケーリングする（最大1.0になるように）
	float sensitivity = Master::m_pDataManager->get_UserConfigData()._mouseSensitivity * MOUSE_SCALE;

	lY = Master::m_pDataManager->get_UserConfigData()._isInvertY ? -lY : lY;
	
	// FOVの比率を計算
	float fovFactor = m_Fov / Master::m_pDataManager->get_DefaultFov();

	// マウスの移動量を計算
	m_Angle_H -= (float)lX * sensitivity * fovFactor;
	m_Angle_V += (float)lY * sensitivity * fovFactor;

	if (m_Angle_V >= 1.57f)	// 下を向く
	{
		m_Angle_V = 1.57f;
	}
	if (m_Angle_V <= -1.3f)	// 上
	{
		m_Angle_V = -1.3f;
	}
	if (m_Angle_H >= 3.14f) {
		m_Angle_H -= 6.28f;
	}
	if (m_Angle_H <= -3.14f) {
		m_Angle_H += 6.28f;
	}


	if (GetInput(GAME_CONFIG::VIEW_UP))		// 上
	{
		m_Angle_V += CAMERA_ANGLE_SPEED;
		if (m_Angle_V >= 1.5f) {
			m_Angle_V = 1.5f;
		}
	}
	if (GetInput(GAME_CONFIG::VIEW_DOWN))	// 下
	{
		m_Angle_V -= CAMERA_ANGLE_SPEED;
		if (m_Angle_V <= -1.0f) {
			m_Angle_V = -1.0f;
		}
	}
	if (GetInput(GAME_CONFIG::VIEW_LEFT))	// 右
	{
		m_Angle_H += CAMERA_ANGLE_SPEED;
		if (m_Angle_H >= 3.14f) {
			m_Angle_H -= 6.28f;
		}
	}
	if (GetInput(GAME_CONFIG::VIEW_RIGHT))	// 左
	{
		m_Angle_H -= CAMERA_ANGLE_SPEED;
		if (m_Angle_H <= -3.14f) {
			m_Angle_H += 6.28f;
		}
	}
}


//*---------------------------------------------------------------------------------------
//* @:Camera3D Class 
//*【?】ビュー変換行列の取得
//* 引数：なし
//* 返値：XMMATRIX
//*----------------------------------------------------------------------------------------
XMMATRIX Camera3D::get_ViewMatrix()const
{
	XMFLOAT3 eye = m_pOwner.lock()->get_Transform().lock()->get_VEC3ToPos();
	XMFLOAT3 lookDirection = m_LookDir;
	XMFLOAT3 upVec = m_UpVec;

	// 位置補正と視線方向を分離し、注視点を越えたときの反転を防ぐ。
	XMMATRIX viewMat = XMMatrixLookToLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&lookDirection),
		XMLoadFloat3(&upVec)
	);

	return viewMat;
}

//*---------------------------------------------------------------------------------------
//*【?】カメラシェイクの開始
//*
//* [引数]
//* _duration : 持続時間
//* &_strength : 強さ
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void Camera3D::RequestShake(float _duration, const VECTOR3::VEC3& _strength)
{
	m_Shaker.Start(_duration, _strength);
}

//*---------------------------------------------------------------------------------------
//*【?】カメラシェイクの開始 
//*     距離減衰ver 
//*
//* [引数]
//* _duration : 持続時間
//* &_strength : 強さ
//* &_shakePos : シェイクを発生させる位置
//* &maxRadius : シェイクを及ぼす最大距離
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void Camera3D::DistanceDecay(float _duration, const VECTOR3::VEC3& _strength, const VECTOR3::VEC3& _shakePos, float _maxRange)
{
	float dist = VEC3::DistanceSq(m_CameraPos, _shakePos); // スタートからの移動距離

	if (_maxRange * _maxRange > dist)
	{
		float t = dist / (_maxRange * _maxRange);	// 0.0 ～ 1.0
		float factor = 1.0f - t;    // そのままでは、最大（端）で1.0になってしまい、離れるほど大きくなってしまうので
		VEC3 length = _strength * factor;

		m_Shaker.Start(_duration, length);
	}

}


void Camera3D::set_UpVec(const VECTOR3::VEC3& upVec)
{
	m_UpVec = upVec;
}

void Camera3D::set_FocusPoint(const VECTOR3::VEC3& focus)
{
	m_FocusPoint = focus;
}

void Camera3D::set_FocusObject(std::weak_ptr<class GameObject> pObj)
{
	m_pFocusObject = pObj;
}

std::string  Camera3D::get_FocusObjectTag()const
{
	return m_pFocusObject.lock()->get_Tag();
}

VECTOR3::VEC3 Camera3D::get_UpVec()const
{
	return m_UpVec;
}

VECTOR3::VEC3 Camera3D::get_FocusPoint()const
{
	return m_FocusPoint;
}

void Camera3D::set_PosOffset(const VECTOR3::VEC3& offset)
{
	m_PosOffset = offset;
}

VECTOR3::VEC3 Camera3D::get_PosOffset()const
{
	return m_PosOffset;
}

void Camera3D::set_FocusOffset(const VECTOR3::VEC3& offset)
{
	m_FocusOffset = offset;
}

VECTOR3::VEC3 Camera3D::get_FocusOffset()const
{
	return m_FocusOffset;
}

VECTOR3::VEC3 Camera3D::get_CameraPos()const
{
	return m_CameraPos;
}

VECTOR3::VEC3 Camera3D::get_LookDir()const
{
	return m_LookDir;
}
