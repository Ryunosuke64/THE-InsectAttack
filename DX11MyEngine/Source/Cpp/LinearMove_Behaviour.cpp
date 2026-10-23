#include "pch.h"
#include "LinearMove_Behaviour.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace DirectX;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
LinearMove_Behaviour::LinearMove_Behaviour()
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
LinearMove_Behaviour::~LinearMove_Behaviour()
{

}

//*---------------------------------------------------------------------------------------
//*【?】直線移動を行う
//*
//* [引数]
//* _deltaTime  : デルタタイム
//* &_param     : 移動に必要なパラメータをまとめた構造体（この移動では方向と速度のみ使う）
//* &_transform : トランスフォームの参照（constなのでsetではなく、行列など必要なパラメータがあるときなどに）
//*
//* [返値]
//* VEC3 : 移動ベクトル
//*----------------------------------------------------------------------------------------
ResultMove LinearMove_Behaviour::MoveCalculate(float _deltaTime, const MoveParam& _param, const MyTransform& _transform)
{
	ResultMove res;

	VEC3 forward = _param._moveDirection;;		// 前方向のベクトルを取得
	float targetAngleY = atan2f(forward.x, forward.z);

	// 目標とするクォータニオン
	XMVECTOR targetRotQ = XMQuaternionRotationRollPitchYaw(0.0f, targetAngleY, 0.0f);
	XMVECTOR crntRotQ = _transform.get_RotationQuaternion();
	// クォータニオンの球面線形補間
	// 普通の線形補間だと、値が飛んでしまうためクォータニオンの場合は球面線形補間を使う
	XMVECTOR newRotQ = XMQuaternionSlerp(crntRotQ, targetRotQ, _param._turnSpeed);

	float speed = _param._moveSpeed + _param._acceleration;

	if (speed >= _param._maxSpeed)
	{
		speed = _param._maxSpeed;
	}
	
	//speed += _param._acceleration;
	res._moveVelocity = forward * speed;	// 移動量を計算
	res._RotQ = newRotQ;
	return res;
}