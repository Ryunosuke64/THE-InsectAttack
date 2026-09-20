#include "pch.h"
#include "Component_Transform.h"
#include "GameObject.h"

using namespace DirectX;
using namespace VERTEX;
using namespace Tool::UV;
using namespace Tool;

/// <summary>
/// クオータニオンからオイラー角を取得する関数
/// </summary>
/// <param name="q"></param>
/// <returns></returns>
VEC3 QuaternionToEuler(const XMVECTOR& q)
{
    // クォータニオンからオイラー角を計算
    float x = XMVectorGetX(q);
    float y = XMVectorGetY(q);
    float z = XMVectorGetZ(q);
    float w = XMVectorGetW(q);
    // オイラー角の計算
    float pitch = atan2f(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));
    float yaw   = asinf(2.0f * (w * y - z * x));
    float roll  = atan2f(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));
    return VEC3(pitch, yaw, roll);
}

const DirectX::XMVECTOR MyTransform::FORWARD  = DirectX::XMVectorSet(0, 0, 1, 0);
const DirectX::XMVECTOR MyTransform::UP       = DirectX::XMVectorSet(0, 1, 0, 0);
const DirectX::XMVECTOR MyTransform::RIGHT    = DirectX::XMVectorSet(1, 0, 0, 0);


/// <summary>
/// コンストラクタ
/// </summary>
MyTransform::MyTransform(std::weak_ptr<GameObject> pOwner, int updateRank)
    : IComponent(pOwner, updateRank),
	m_Position(DirectX::XMVectorZero()),
	m_RotationQ(DirectX::XMQuaternionIdentity()),
	m_Scale(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f)),
    m_Local_Position(DirectX::XMVectorZero()),
    m_Local_RotationQ(DirectX::XMQuaternionIdentity()),
    m_Local_Scale(DirectX::XMVectorZero()),
	m_pParent(),
    m_isDirty(false),
    m_OffsetWorldTransfomationMatrix(XMMatrixIdentity())
{
    this->set_Tag("MyTransform");
}

/// <summary>
/// デストラクタ
/// </summary>
MyTransform::~MyTransform()
{
    this->Release();
}


//*---------------------------------------------------------------------------------------
//*【?】特定の方向に向かせる
//*     主にターゲットになるオブジェクトへ向かせる場合等に使う 
//*
//* [引数]
//* &target : 目標座標
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void MyTransform::LookAt(const VECTOR3::VEC3 &_targetPos)
{
    // LookAt関数で、現在の位置から目標位置を向くためのビュー行列を作成
    XMMATRIX lookAtMtx = XMMatrixLookAtLH(m_Position, _targetPos, UP);  

    // 逆行列を取ることで、目標方向を向くための回転行列を得る
    XMMATRIX invLookAtMtx = XMMatrixInverse(nullptr, lookAtMtx);       

    // 回転行列からクォータニオンを作成
    XMVECTOR rot = XMQuaternionRotationMatrix(invLookAtMtx);             

    m_RotationQ = rot;                                      
}


//*---------------------------------------------------------------------------------------
//*【?】球面線形補間を使用した目標方向への回転
//*
//* [引数]
//* &target : 目標座標
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void MyTransform::SlerpLookAt(const VECTOR3::VEC3 &_targetPos, float _t)
{
    XMMATRIX lookAtMtx = XMMatrixLookAtLH(m_Position, _targetPos, UP);  
    XMMATRIX invLookAtMtx = XMMatrixInverse(nullptr, lookAtMtx);        
    XMVECTOR rot = XMQuaternionRotationMatrix(invLookAtMtx);            
    // ここまではLookAtと同じ --------------------------------------

    // 球面線形補間で滑らかに回転させる
    m_RotationQ = XMQuaternionSlerp(m_RotationQ, rot, _t);
}


// ---------------------------------------------------------------------------
/// <summary>
/// 解放
/// </summary>
// ---------------------------------------------------------------------------
void MyTransform::Release()
{
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//  ◆ワールド空間
//
/////////////////////////////////////////////////////////////////////////////////////////
// *===========================================================================
// ・セッタ
// *===========================================================================
/*--------------- Position ---------------*/
void MyTransform::set_Pos(float x, float y, float z){
    m_Position = XMVectorSet(x, y, z, 1.f);
}
void MyTransform::set_Pos(const VECTOR3::VEC3& vIn) {
    m_Position = vIn;
}

/*--------------- Rotate ---------------*/
/* ラジアン指定 */

void MyTransform::set_RotateToRad(float _pitch, float _yaw, float _roll)
{
    m_EulerAngles = VEC3(_pitch, _yaw, _roll);   // オイラー角も保持しておく
    m_RotationQ = XMQuaternionRotationRollPitchYaw(_pitch, _yaw, _roll);
}
void MyTransform::set_RotateToRad(const VECTOR3::VEC3 &_vIn)
{
    m_EulerAngles = _vIn;   // オイラー角も保持しておく
    m_RotationQ = XMQuaternionRotationRollPitchYaw(_vIn.x, _vIn.y, _vIn.z);
}

/* デグリー指定 */

void MyTransform::set_RotateToDeg(float _pitch, float _yaw, float _roll)
{
   m_EulerAngles.x =  XMConvertToRadians(_pitch);
   m_EulerAngles.y =  XMConvertToRadians(_yaw);
   m_EulerAngles.z =  XMConvertToRadians(_roll);

    m_RotationQ = XMQuaternionRotationRollPitchYaw(m_EulerAngles.x, m_EulerAngles.y, m_EulerAngles.z);
}

void MyTransform::set_RotateToDeg(const VECTOR3::VEC3& _vIn) 
{
    m_EulerAngles.x = XMConvertToRadians(_vIn.x);
    m_EulerAngles.y = XMConvertToRadians(_vIn.y);
    m_EulerAngles.z = XMConvertToRadians(_vIn.z);

    m_RotationQ = XMQuaternionRotationRollPitchYaw(m_EulerAngles.x, m_EulerAngles.y, m_EulerAngles.z);
}

//
// クォータニオンを直接設定
//
void MyTransform::set_RotationQuaternion(const DirectX::XMVECTOR &_q)
{
    m_EulerAngles = QuaternionToEuler(_q);   // オイラー角も保持しておく
    m_RotationQ = _q;
}

/*--------------- Scale ---------------*/
void MyTransform::set_Scale(float x, float y, float z){
    m_Scale = XMVectorSet(x, y, z, 1.f);
}
void MyTransform::set_Scale(const VECTOR3::VEC3& vIn) {
    m_Scale = vIn;
}

// *===========================================================================
// ・ゲッタ
// *===========================================================================
/*--------------- Position ---------------*/
XMVECTOR MyTransform::get_XMVecToPos() const {
    return m_Position;
}
const VEC3 MyTransform::get_VEC3ToPos() const {
    return VEC3(
        XMVectorGetX(m_Position),
        XMVectorGetY(m_Position),
        XMVectorGetZ(m_Position)
    );
}

/*--------------- Rotate ---------------*/
// クォータニオンで保持しているが、外からはオイラーで取得できるようにする
// 今後、クォータニオンでの取得、設定用のメソッドも用意するかも


/* ラジアン */

XMVECTOR MyTransform::get_XMVecToRotateToRad() const 
{
    return m_EulerAngles;
}

const VEC3 MyTransform::get_VEC3ToRotateToRad() const
{
    return m_EulerAngles;
}

/* デグリー */

XMVECTOR MyTransform::get_XMVecToRotateToDeg() const 
{
	return XMVectorSet(
		XMConvertToDegrees(m_EulerAngles.x),
		XMConvertToDegrees(m_EulerAngles.y),
		XMConvertToDegrees(m_EulerAngles.z),
		1.0f
	);
}

const VEC3 MyTransform::get_VEC3ToRotateToDeg() const 
{
    VEC3 res = VEC3();
    res.x = RadToDeg(m_EulerAngles.x);
    res.y = RadToDeg(m_EulerAngles.y);
    res.z = RadToDeg(m_EulerAngles.z);
    return res;
}

//
// クォータニオンでの取得
//
XMVECTOR MyTransform::get_RotationQuaternion()const
{
    return m_RotationQ;
}
//
// オイラーの取得
//
VEC3 MyTransform::get_EulerAngles()const
{
    return m_EulerAngles;
}

/*--------------- Scale ---------------*/
XMVECTOR MyTransform::get_XMVecToScale() const {
    return m_Scale;
}
const VEC3 MyTransform::get_VEC3ToScale() const {
    return VEC3(
        XMVectorGetX(m_Scale),
        XMVectorGetY(m_Scale),
        XMVectorGetZ(m_Scale)
    );
}

/* ワールド座標の取得 */
const VECTOR3::VEC3 MyTransform::get_WorldVEC3ToPos()const
{
    XMMATRIX mat = get_WorldMtx();
    return VEC3::FromXMVECTOR(mat.r[3]);
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//  ◆ローカル空間オフセット
//
/////////////////////////////////////////////////////////////////////////////////////////

// *===========================================================================
// ・セッタ
// *===========================================================================
/*--------------- Position ---------------*/
void MyTransform::set_VEC3ToLocal_Pos(const VECTOR3::VEC3 &vIn) 
{
    m_Local_Position = vIn;
}
/*--------------- Rotate ---------------*/
/* ラジアン */
void MyTransform::set_VEC3ToLocal_RotateToRad(const VECTOR3::VEC3 &vIn) 
{
    m_Local_EulerAngles = vIn;   // オイラー角も保持しておく
    m_Local_RotationQ = XMQuaternionRotationRollPitchYaw(vIn.x, vIn.y, vIn.z);
}
/* デグリー */
void MyTransform::set_VEC3ToLocal_RotateToDeg(const VECTOR3::VEC3 &_vIn) 
{
    m_Local_EulerAngles.x =  XMConvertToRadians(_vIn.x);
    m_Local_EulerAngles.y =  XMConvertToRadians(_vIn.y);
    m_Local_EulerAngles.z =  XMConvertToRadians(_vIn.z);
    m_Local_RotationQ = XMQuaternionRotationRollPitchYaw(m_Local_EulerAngles.x, m_Local_EulerAngles.y, m_Local_EulerAngles.z);
}
/*--------------- Scale ---------------*/
void MyTransform::set_VEC3ToLocal_Scale(const VECTOR3::VEC3 &vIn) 
{
    m_Local_Scale = vIn;
}

// *===========================================================================
// ・ゲッタ
// *===========================================================================
/*--------------- Position ---------------*/
const VEC3 MyTransform::get_VEC3ToLocal_Pos()const
{
    return VEC3(
        XMVectorGetX(m_Local_Position),
        XMVectorGetY(m_Local_Position),
        XMVectorGetZ(m_Local_Position)
    );
}

/*--------------- Rotate ---------------*/
/* ラジアン */
const VEC3 MyTransform::get_VEC3ToLocal_RotateToRad()const
{
    return m_Local_EulerAngles;
}
/* デグリー */
const VEC3 MyTransform::get_VEC3ToLocal_RotateToDeg()const
{
    VEC3 res = VEC3();
    res.x = RadToDeg(m_Local_EulerAngles.x);
    res.y = RadToDeg(m_Local_EulerAngles.y);
    res.z = RadToDeg(m_Local_EulerAngles.z);
    return res;
}

/*--------------- Scale ---------------*/
const VEC3 MyTransform::get_VEC3ToLocal_Scale()const
{
    return VEC3(
        XMVectorGetX(m_Local_Scale),
        XMVectorGetY(m_Local_Scale),
        XMVectorGetZ(m_Local_Scale)
    );
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//  ◆行列
//
/////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 座標行列取得
/// </summary>
XMMATRIX MyTransform::get_MtxPos()const{
    return XMMatrixTranslationFromVector(
        m_Position
    );
}
/// <summary>
/// 回転行列取得
/// </summary>
XMMATRIX MyTransform::get_MtxRotate()const{
    return XMMatrixRotationQuaternion(
        m_RotationQ
    );
}

/// <summary>
/// 拡大縮小行列取得
/// </summary>
XMMATRIX MyTransform::get_MtxScale()const{
    return XMMatrixScalingFromVector(
        m_Scale
    );
}

// -----------------------------------------------------------------------------
/// <summary>
/// ワールド行列取得
/// </summary>
/// <returns></returns>
// -----------------------------------------------------------------------------
XMMATRIX MyTransform::get_WorldMtx()const {

    // オフセットを加算
    XMVECTOR rot = XMQuaternionMultiply(m_Local_RotationQ, m_RotationQ);
    XMVECTOR scl = DirectX::XMVectorAdd(m_Scale, m_Local_Scale);
    XMVECTOR pos = DirectX::XMVectorAdd(m_Position, m_Local_Position);

    XMMATRIX mtxS = XMMatrixScalingFromVector(scl);
    XMMATRIX mtxRot = XMMatrixRotationQuaternion(rot);
    XMMATRIX mtxT = XMMatrixTranslationFromVector(pos);

    XMMATRIX localMtx = mtxS * mtxRot * mtxT;

    localMtx *= m_OffsetWorldTransfomationMatrix;   


    // 親がいるなら自分と親を掛けたものを返す
    if (m_pParent.lock())
    {
        return localMtx * m_pParent.lock()->get_WorldMtx();
    }
    else
    {
        return localMtx;
    }
}

// -----------------------------------------------------------------------------
/// <summary>
/// ワールド行列取得
/// パラメータを明示的に指定する
/// </summary>
/// <returns></returns>
// -----------------------------------------------------------------------------
XMMATRIX MyTransform::get_WorldMtx(const XMMATRIX &scl, const XMMATRIX &rot, const XMMATRIX &trans)const
{
    XMMATRIX localMtx = scl * rot * trans;

    // 親がいるなら自分と親を掛けたものを返す
    if (m_pParent.lock())
        return localMtx * m_pParent.lock()->get_WorldMtx();
    else
        return localMtx;
}

// -----------------------------------------------------------------------------
/// <summary>
/// 回転を含めないワールド行列
/// </summary>
/// <returns></returns>
// -----------------------------------------------------------------------------
XMMATRIX MyTransform::get_ExcludingRotWorldMtx()const{
    // オフセットを加算
    XMVECTOR scl = DirectX::XMVectorAdd(m_Scale, m_Local_Scale);
    XMVECTOR pos = DirectX::XMVectorAdd(m_Position, m_Local_Position);


    XMMATRIX mtxS = XMMatrixScalingFromVector(scl);
    XMMATRIX mtxT = XMMatrixTranslationFromVector(pos);

    XMMATRIX localMtx = mtxS * mtxT;

    localMtx *= m_OffsetWorldTransfomationMatrix;

    // 親がいるなら自分と親を掛けたものを返す
    if (m_pParent.lock())
        return localMtx * m_pParent.lock()->get_WorldMtx();
    else
        return localMtx;
}

// -----------------------------------------------------------------------------
/// <summary>
/// 回転を含めないワールド行列
/// パラメータを明示的に指定する
/// </summary>
/// <returns></returns>
// -----------------------------------------------------------------------------
XMMATRIX MyTransform::get_ExcludingRotWorldMtx(const DirectX::XMMATRIX& _scl, const DirectX::XMMATRIX& _trans)const
{
    XMMATRIX localMtx = _scl * _trans;

    // 親がいるなら自分と親を掛けたものを返す
    if (m_pParent.lock())
        return localMtx * m_pParent.lock()->get_WorldMtx();
    else
        return localMtx;
}



//*---------------------------------------------------------------------------------------
//* @:Transform Class 
//*【?】ローカル空間の「前」方向取得
//* 引数：なし
//* 返値：VEC3
//*----------------------------------------------------------------------------------------
const VEC3 MyTransform::get_Forward() const
{
    return VEC3::FromXMVECTOR(DirectX::XMVector3Rotate(FORWARD, m_RotationQ));
}

//*---------------------------------------------------------------------------------------
//* @:Transform Class 
//*【?】ワールド空間の「前」方向取得
//* 引数：なし
//* 返値：VEC3
//*----------------------------------------------------------------------------------------
const VEC3 MyTransform::get_WorldForward() const
{
    // 最終的なワールド行列を取得
    DirectX::XMMATRIX worldMat = get_WorldMtx();

    // Z方向(0,0,1)のベクトルを、ワールド行列の回転成分だけで変換する
    // TransformNormal を使うことで、移動量やスケールを無視して純粋な方向だけを取り出せる
    DirectX::XMVECTOR worldForward = DirectX::XMVector3TransformNormal(
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        worldMat
    );

    // 正規化して返す
    return VEC3::FromXMVECTOR(DirectX::XMVector3Normalize(worldForward));
}


//*---------------------------------------------------------------------------------------
//* @:Transform Class 
//*【?】「上」方向取得
//* 引数：なし
//* 返値：VEC3
//*----------------------------------------------------------------------------------------
VEC3 MyTransform::get_Up() const
{
    return VEC3::FromXMVECTOR(DirectX::XMVector3Rotate(UP, m_RotationQ));
}


//*---------------------------------------------------------------------------------------
//* @:Transform Class 
//*【?】「右」方向取得
//* 引数：なし
//* 返値：VEC3
//*----------------------------------------------------------------------------------------
VEC3 MyTransform::get_Right() const
{
    return VEC3::FromXMVECTOR(DirectX::XMVector3Rotate(RIGHT, m_RotationQ));
}

std::weak_ptr<MyTransform> MyTransform::get_Parent()const
{
    return m_pParent;
}
