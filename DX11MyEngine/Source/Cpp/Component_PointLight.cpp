#include "pch.h"
#include "Component_PointLight.h"
#include "Component_Transform.h"
#include "Component_DirectionalLight.h"
#include "RendererEngine.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Component_3DCamera.h"

using namespace VECTOR3;
constexpr float DEF_RANGE = 100.0f;


//*---------------------------------------------------------------------------------------
//* @:PointLight Class 
//*【?】コンストラクタ
//* 引数：1.オーナーオブジェクト
//* 引数：2.更新レイヤー
//*----------------------------------------------------------------------------------------
PointLight::PointLight(std::weak_ptr<GameObject> pOwner, int updateRank) 
    : Light(pOwner, updateRank),
    m_Range(DEF_RANGE),
    m_IsFlash(false),
    m_FlashElapsed(0.0f),
    m_FlashMaxRange(0.0f),
    m_FlashMaxIntensity(0.0f),
    m_FlashDuration(0.0f)
{
    this->set_Tag("PointLight");
}


//*---------------------------------------------------------------------------------------
//* @:PointLight Class 
//*【?】デストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
PointLight::~PointLight()
{

}


//*---------------------------------------------------------------------------------------
//* @:PointLight Class 
//*【?】初期化
//* 引数：1.RendererEngine
//* 返値：bool
//*----------------------------------------------------------------------------------------
void PointLight::Start(RendererEngine &renderer)
{
    m_pOwnerTransform = m_pOwner.lock()->get_Transform();
}


//*---------------------------------------------------------------------------------------
//* @:PointLight Class 
//*【?】更新
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void PointLight::Update(RendererEngine &renderer)
{
    auto pContext = renderer.get_DeviceContext();

    // フラッシュ処理
    if (m_IsFlash)
    {
        m_FlashElapsed += Master::m_pTimeManager->get_DeltaTime();
        float t = m_FlashElapsed / m_FlashDuration; // フラッシュ割合
        if (t >= 1.0f)
        {
            m_IsFlash = false;
        }

        float ease = 1.0f - Tool::Easing::EaseOutQuart(t);

        m_Intensity = ease * m_FlashMaxIntensity;
        //m_Range = ease * m_FlashMaxRange;
    }

    // 消灯中のライトは登録しない
    if (m_Intensity <= 0.0f || m_Range <= 0.0f)
    {
        return;
    }

    VEC3 pos = m_pOwnerTransform.lock()->get_WorldVEC3ToPos();;

    CB_PointLightData pointData{};
    // バッファの更新
    pointData.Pos               = pos;
	pointData.Range	            = m_Range;
    pointData.SpecularColor     = VECTOR3::VEC3(1.0f);
    pointData.SpecularIntensity = 150.0f;
    pointData.DiffuseColor      = m_LightColor;
	pointData.DiffuseIntensity  = m_Intensity;

    // 情報を設定
    Master::m_pLightManager->set_PointLightData(pointData);
}


//*---------------------------------------------------------------------------------------
//* @:PointLight Class 
//*【?】ライトをフラッシュさせる
//* 引数：1.フラッシュ時間
//* 返値：void
//*----------------------------------------------------------------------------------------
void PointLight::Flash(float _duration, float _maxIntensity, float _maxRange)
{
    m_FlashDuration = _duration;
    m_FlashMaxIntensity = _maxIntensity;
    m_FlashMaxRange = _maxRange;
    m_FlashElapsed = 0.0f;
    m_IsFlash = true;

    m_Range = _maxRange;
}