#pragma once

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Material Class --- */
//
// 【?】cpp側のマテリアル情報
//      テクスチャの弱参照をそれぞれ持つ リソース管理から受け取る
//
// ***************************************************************************************
class Material
{
public:
    // 基本パラメータ================================================
    VECTOR4::VEC4 m_DiffuseColor;   // ディフューズ反射光
    VECTOR4::VEC4 m_SpecularColor;  // スペキュラ反射光

    float m_SpecularPower;          // スペキュラの絞り強さ
    float m_EnvironmentReflectionStrength;  // 環境反射の強さ

    VECTOR3::VEC3 m_EmissiveColor;  // 発光色
    float m_EmissivePower;          // 自己発光の強さ（ブルーム強度）

    // テクスチャマップ==============================================
    DIFFUSE_MAP_DATA m_DiffuseMap;     // ディフューズマップ
    NORMAL_MAP_DATA m_NormalMap;       // ノーマルマップ
    SPECULAR_MAP_DATA m_SpecularMap;   // スペキュラマップ

    BLEND_MODE m_BlendMode;            // ブレンドモード

    RenderData::CULL_MODE m_CullMode;  // カリングモード


    Material() :
        m_DiffuseColor(VECTOR4::VEC4(1.f, 1.f, 1.f, 1.f)),
        m_SpecularColor(VECTOR4::VEC4(1.f, 1.f, 1.f, 1.f)),
        m_SpecularPower(0.0f),
        m_EnvironmentReflectionStrength(0.0f),
        m_BlendMode(BLEND_MODE::NONE),
        m_EmissivePower(0.0f),
        m_EmissiveColor(VECTOR3::VEC3(0.0f,0.0f,0.0f)),
        m_CullMode(RenderData::CULL_MODE::BACK)
    {

    };

    ~Material()
    {
    }
};
