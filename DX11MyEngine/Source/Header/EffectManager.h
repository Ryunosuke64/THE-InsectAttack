#pragma once

#include <Effekseer.h>
#include <EffekseerRendererDX11.h>

#define EFFECT_MAX 256
#define PERTICLE_BUFFER 12000

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:EffectManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】エフェクトの管理
//      Effekseerのエフェクトの読み込み、再生等を行う
// 
// 参考サイト：https://github.com/effekseer/Effekseer/blob/master/docs/Help_Cpp/18x/Guide_Cpp_Ja.md
//
// ***************************************************************************************
class EffectManager
{
private:
    ::Effekseer::ManagerRef m_EfkManager;           // エフェクシアの管理クラス的な？
    ::EffekseerRenderer::RendererRef m_EfkRenderer; // エフェクトの描画に必要なもの
    std::unordered_map<std::string, Effekseer::EffectRef> m_EffectMap; // エフェクトのパラメータを持つ

    float m_Timer;

public:
    EffectManager();
    ~EffectManager();

    bool Setup(RendererEngine& renderer);           // 初期化
    void UpdateEffect(RendererEngine& renderer);    // エフェクトの更新
    void DrawEffect();                              // エフェクトの描画
    bool LoadEffect(const char16_t *_name,const std::string&_key);           // エフェクトの読み込み
    int PlayEffect(const std::string& _key, const VECTOR3::VEC3& _pos = 0, int32_t _startFrame = 0); // エフェクトの再生
    void StopEffect(int handle);                                    // エフェクトを止める
    void SetPositionEffect(int handle, float x, float y, float z);  // エフェクトの位置を設定
    void SetScaleEffect(int handle, float x, float y, float z);     // エフェクトの大きさを設定
    void SetRotationEffect(int handle, float x, float y, float z);  // エフェクトの回転値を設定
    void SetRotationEffect(int handle, const VECTOR3::VEC3& _axis, float _angle);  // エフェクトの軸を指定し、角度（ラジアン）を設定
    void SetPositionEffect(int handle,const VECTOR3::VEC3& _v);     
    void SetScaleEffect(int handle,   const VECTOR3::VEC3& _v);     
    void SetRotationEffect(int handle,const VECTOR3::VEC3& _v);     
    void SetAllColorEffect(int handle,const Effekseer::Color& _color);
    bool IsPlayingEffect(int handle);                               // エフェクトの再生状態を取得
    void SetDynamicParameter(int handle, int32_t _index, float _param);             // 動的パラメータの設定
    void StopAllEffects();  // 全てのエフェクトの停止

    
private:
    // コピー禁止
    EffectManager(const EffectManager &) = delete;
    EffectManager &operator=(const EffectManager &) = delete;

};

