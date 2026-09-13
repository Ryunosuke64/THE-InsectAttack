#include "pch.h"
#include "EffectManager.h"
#include "Component_3DCamera.h"
#include "RendererEngine.h"
#include <Effekseer.h>

using namespace DirectX;
using namespace VECTOR3;

EffectManager::EffectManager():
    m_Timer(0.0f)
{
}

EffectManager::~EffectManager()
{

}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool EffectManager::Setup(RendererEngine &renderer)
{
    auto pDevice = renderer.get_Device();
    auto pContext = renderer.get_DeviceContext();
    UINT screenW = renderer.get_ScreenWidth();
    UINT screenH = renderer.get_ScreenHeight();

    // エフェクトのマネージャーの作成
    m_EfkManager = ::Effekseer::Manager::Create(PERTICLE_BUFFER);

    // Create a  graphics device
    // 描画デバイスの作成
    ::Effekseer::Backend::GraphicsDeviceRef graphicsDevice;
    graphicsDevice = ::EffekseerRendererDX11::CreateGraphicsDevice(pDevice, pContext);

    // Create a renderer of effects
    // エフェクトのレンダラーの作成
    m_EfkRenderer = ::EffekseerRendererDX11::Renderer::Create(graphicsDevice, PERTICLE_BUFFER);

    // Specify rendering modules
    // 描画モジュールの設定
    m_EfkManager->SetSpriteRenderer(m_EfkRenderer->CreateSpriteRenderer());
    m_EfkManager->SetRibbonRenderer(m_EfkRenderer->CreateRibbonRenderer());
    m_EfkManager->SetRingRenderer(m_EfkRenderer->CreateRingRenderer());
    m_EfkManager->SetTrackRenderer(m_EfkRenderer->CreateTrackRenderer());
    m_EfkManager->SetModelRenderer(m_EfkRenderer->CreateModelRenderer());

    // Specify a texture, model, curve and material loader
    // It can be extended by yourself. It is loaded from a file on now.
    // テクスチャ、モデル、カーブ、マテリアルローダーの設定する。
    // ユーザーが独自で拡張できる。現在はファイルから読み込んでいる。
    m_EfkManager->SetTextureLoader(m_EfkRenderer->CreateTextureLoader());
    m_EfkManager->SetModelLoader(m_EfkRenderer->CreateModelLoader());
    m_EfkManager->SetMaterialLoader(m_EfkRenderer->CreateMaterialLoader());
    m_EfkManager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

    // Setup the coordinate system. This must be matched with your application.
    // 座標系を設定する。アプリケーションと一致させる必要がある。
    m_EfkManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

    // Specify a position of view
    // 視点位置を確定
    VECTOR3::VEC3 viewPos = Master::m_pDataManager->get_CameraPos();
    ::Effekseer::Vector3D viewerPosition = ::Effekseer::Vector3D(viewPos.x, viewPos.y, viewPos.z);

    // Specify a projection matrix
    // 投影行列を設定
    ::Effekseer::Matrix44 projectionMatrix;
    projectionMatrix.PerspectiveFovLH(90.0f / 180.0f * 3.14f, (float)screenW / (float)screenH, 1.0f, 5000.0f);

    // Specify a camera matrix
    // カメラ行列を設定
    ::Effekseer::Matrix44 cameraMatrix;
    cameraMatrix.LookAtLH(viewerPosition, ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f));

    // エフェクトの読み込み
    LoadEffect(u"Resource/Effect/Spark.efkefc", "Spark");
    LoadEffect(u"Resource/Effect/Smoke.efkefc", "Smoke");
    LoadEffect(u"Resource/Effect/AntHit_01.efkefc", "AntHit_01");
    LoadEffect(u"Resource/Effect/AntDead_01.efkefc", "AntDead_01");
    LoadEffect(u"Resource/Effect/AntBloodSpatter_01.efkefc", "AntBloodSpatter_01");
    LoadEffect(u"Resource/Effect/Simple_Sprite_BillBoard.efkefc", "DeadExplosion");
    LoadEffect(u"Resource/Effect/Simple_SpawnMethod1.efkefc", "Hit");
    //LoadEffect(u"Resource/Effect/Explosion_01.efkefc", "Explosion_01");
    LoadEffect(u"Resource/Effect/Explosion_01.efkefc", "Explosion_01");
    LoadEffect(u"Resource/Effect/Explosion_02.efkefc", "Explosion_02");
    LoadEffect(u"Resource/Effect/Explosion_03.efkefc", "Explosion_03");
    LoadEffect(u"Resource/Effect/Explosion_Smoke_01.efkefc", "Explosion_Smoke_01");
    LoadEffect(u"Resource/Effect/Explosion_Smoke_02.efkefc", "Explosion_Smoke_02");
    LoadEffect(u"Resource/Effect/Explosion_Plasma_01.efkefc", "Explosion_Plasma_01");
    LoadEffect(u"Resource/Effect/Explosion_Acid_01.efkefc", "Explosion_Acid_01");
    LoadEffect(u"Resource/Effect/Smoke_01.efkefc", "Smoke_01");
    LoadEffect(u"Resource/Effect/Smoke_02.efkefc", "Smoke_02");
    LoadEffect(u"Resource/Effect/Fragment_Smoke_01.efkefc", "Fragment_Smoke_01");
    LoadEffect(u"Resource/Effect/Fragment_Plasma_01.efkefc", "Fragment_Plasma_01");
    LoadEffect(u"Resource/Effect/Fragment_Acid_01.efkefc", "Fragment_Acid_01");
    LoadEffect(u"Resource/Effect/Fragment_Acid_02.efkefc", "Fragment_Acid_02");
    LoadEffect(u"Resource/Effect/Destruction_Fragments.efkefc", "Destruction_Fragments");
    LoadEffect(u"Resource/Effect/Destruction_Fragments_Small.efkefc", "Destruction_Fragments_Small");
    LoadEffect(u"Resource/Effect/Fragment_Octahedron.efkefc", "Fragment_Octahedron");
    LoadEffect(u"Resource/Effect/BulletHit_Standard.efkefc", "BulletHit_Standard");
    LoadEffect(u"Resource/Effect/BulletHit_Acid.efkefc", "BulletHit_Acid");
    LoadEffect(u"Resource/Effect/BulletHit_Flame.efkefc", "BulletHit_Flame");
    LoadEffect(u"Resource/Effect/MuzzleEffect_01.efkefc", "MuzzleEffect_01");
    LoadEffect(u"Resource/Effect/MuzzleEffect_Rocket_01.efkefc", "MuzzleEffect_Rocket_01");
    LoadEffect(u"Resource/Effect/MuzzleEffect_Flame_01.efkefc", "MuzzleEffect_Flame_01");
    LoadEffect(u"Resource/Effect/Bullet_Laser_01.efkefc", "Bullet_Laser_01");
    LoadEffect(u"Resource/Effect/BulletHit_Laser.efkefc", "BulletHit_Laser");

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】エフェクトの更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void EffectManager::UpdateEffect(RendererEngine& renderer)
{
    UINT screenW = renderer.get_ScreenWidth();
    UINT screenH = renderer.get_ScreenHeight();

    if (Master::m_pDataManager->get_CameraComponent().expired())
    {
        assert(false);
        return;
    }

    auto camera = Master::m_pDataManager->get_CameraComponent().lock();
    VEC3 cameraPos = Master::m_pDataManager->get_CameraPos();

    // 視点位置を確定
    VECTOR3::VEC3 viewPos = cameraPos;
    float fov = camera->get_Fov();
    float camera_near = camera->get_Near();
    float camera_far = camera->get_Far();

    ::Effekseer::Vector3D viewerPosition = ::Effekseer::Vector3D(viewPos.x, viewPos.y, viewPos.z);

    // Specify a projection matrix
    // 投影行列を設定
    ::Effekseer::Matrix44 projectionMatrix;
    projectionMatrix.PerspectiveFovLH(XMConvertToRadians(fov),(float)screenW / (float)screenH, camera_near, camera_far);

    VECTOR3::VEC3 forcus = camera->get_FocusPoint();

    // Specify a camera matrix
    // カメラ行列を設定
    ::Effekseer::Matrix44 cameraMatrix;
    cameraMatrix.LookAtLH(viewerPosition, ::Effekseer::Vector3D(forcus.x, forcus.y, forcus.z), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f));

    // Set layer parameters
    // レイヤーパラメータの設定
    Effekseer::Manager::LayerParameter layerParameter;
    layerParameter.ViewerPosition = viewerPosition;
    m_EfkManager->SetLayerParameter(0, layerParameter);

    float timeScale = Master::m_pTimeManager->get_TimeScale();
    float deltaTime = Master::m_pTimeManager->get_DeltaTime();

    // Update the manager
    // マネージャーの更新
    Effekseer::Manager::UpdateParameter updateParameter;
    updateParameter.DeltaFrame = timeScale;
    m_EfkManager->Update(updateParameter);

    // Update a time
    // 時間を更新する
    m_EfkRenderer->SetTime(m_Timer);

    // Specify a projection matrix
    // 投影行列を設定
    m_EfkRenderer->SetProjectionMatrix(projectionMatrix);

    // Specify a camera matrix
    // カメラ行列を設定
    m_EfkRenderer->SetCameraMatrix(cameraMatrix);

    m_Timer += deltaTime * timeScale;
}


//*---------------------------------------------------------------------------------------
//*【?】エフェクトの描画
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void EffectManager::DrawEffect()
{
    //m_EfkRenderer->SetMaintainGammaColorInLinearColorSpace(true);
    // エフェクトの描画開始処理
    m_EfkRenderer->BeginRendering();
    // エフェクトの描画
    Effekseer::Manager::DrawParameter drawParameter;
    drawParameter.ZNear = 0.0f;
    drawParameter.ZFar = 1.0f;
    drawParameter.ViewProjectionMatrix = m_EfkRenderer->GetCameraProjectionMatrix();
    drawParameter.IsSortingEffectsEnabled = true;
    m_EfkManager->Draw(drawParameter);
    // エフェクトの描画終了処理
    m_EfkRenderer->EndRendering();
}


//*---------------------------------------------------------------------------------------
//*【?】エフェクトの読み込み
//*
//* [引数]
//* *_name : ファイル名
//* &_key : 取り出すためのキー
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool EffectManager::LoadEffect(const char16_t *_name, const std::string& _key)
{
    //最大数オーバー
    if (m_EffectMap.size() >= EFFECT_MAX) {
        return false;
    }

    // すでに何か読み込まれている
    if (m_EffectMap[_key] != nullptr){
        return false;
    }

    //エフェクトの読込
    m_EffectMap[_key] = Effekseer::Effect::Create(m_EfkManager, _name);

    //読み込みに失敗した
    if (m_EffectMap[_key] == nullptr) {
        assert(false);
        return false;
    }
    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】エフェクトの再生
//*
//* [引数]
//* &_key : 取り出すためのキー
//*
//* [返値]
//* int : ハンドル（呼び出し側で管理）
//*----------------------------------------------------------------------------------------
int EffectManager::PlayEffect(const std::string& _key, const VECTOR3::VEC3& _pos , int32_t _startFrame )
{
    return m_EfkManager->Play(m_EffectMap[_key], Effekseer::Vector3D(_pos.x, _pos.y, _pos.z), _startFrame);
}

// エフェクトが再生中かどうかを確認
bool EffectManager::IsPlayingEffect(int handle)
{
    return m_EfkManager->Exists(handle);
}

// 全てのエフェクトの停止
void EffectManager::StopAllEffects()
{
    m_EfkManager->StopAllEffects();
}

// 動的パラメータの設定
void EffectManager::SetDynamicParameter(int handle, int32_t _index, float _param)
{
    m_EfkManager->SetDynamicInput(handle, _index, _param);
}

// エフェクトを停止
void EffectManager::StopEffect(int handle)
{
    m_EfkManager->StopEffect(handle);
}

// エフェクトの位置を設定
void EffectManager::SetPositionEffect(int handle, float x, float y, float z)
{
    m_EfkManager->SetLocation(handle, x, y, z);

}

// エフェクトの回転（ラジアン）を設定
void EffectManager::SetRotationEffect(int handle, float x, float y, float z)
{
    m_EfkManager->SetRotation(handle, x, y, z);
}

// 任意軸周りの反時計周りの回転角度を指定する
void EffectManager::SetRotationEffect(int handle, const VECTOR3::VEC3& _axis, float _angle)
{
    m_EfkManager->SetRotation(handle, { _axis.x,_axis.y,_axis.z }, _angle);
}

// エフェクトの大きさを設定
void EffectManager::SetScaleEffect(int handle, float x, float y, float z)
{
    m_EfkManager->SetScale(handle, x, y, z);
}

//
// VECTOR3::VEC3を使ったオーバーロード関数の実装
//

// エフェクトの位置を設定
void EffectManager::SetPositionEffect(int handle, const VECTOR3::VEC3& _v)
{
    m_EfkManager->SetLocation(handle, _v.x, _v.y, _v.z);
}

// エフェクトの回転（ラジアン）を設定
void EffectManager::SetRotationEffect(int handle, const VECTOR3::VEC3& _v)
{
    m_EfkManager->SetRotation(handle, _v.x, _v.y, _v.z);
}

// エフェクト全体の色を設定
void EffectManager::SetAllColorEffect(int handle, const Effekseer::Color& _color)
{
    m_EfkManager->SetAllColor(handle, _color);
}

// エフェクトの大きさを設定
void EffectManager::SetScaleEffect(int handle, const VECTOR3::VEC3& _v)
{
    m_EfkManager->SetScale(handle, _v.x, _v.y, _v.z);
}
