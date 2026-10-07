// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
/* - @:ピクセルシェーダ -*/
//
//  【?】フォワードシェーディング
//       水表現用
//
// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
#pragma once
#include "ConstantBuffers_H.hlsli"
#include "LightFunctions_H.hlsli"
#include "UtilityFunctions_H.hlsli"

SamplerState g_sSampler : register(s0);
Texture2D g_tNormalTextureA : register(t0); // 水用ノーマルマップA
Texture2D g_tNormalTextureB : register(t1); // 水用ノーマルマップB 
Texture2D<float> g_tSceneDepth : register(t7);  // 深度テクスチャ

/* =========================================================================
/* - @:出力構造体 - */
/* =========================================================================*/
struct PS_SimpleIntput
{
    float4 Pos      : SV_Position;
    float2 UV       : TEXCOORD0;
    float ViewDepth : TEXCOORD1;
};


// **************************************************************************
/* - @:エントリーポイント - */
// **************************************************************************
float4 PSMain(PS_SimpleIntput input) : SV_TARGET
{
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    float finalAlpha = cb_DiffuseColor.a;

    float2 uvA = input.UV;
    float2 uvB = input.UV;
    
    //=====================================
    // Aは右斜め前方向に 
    // Bは左斜め前方向に
    //=====================================
    uvA += float2(0.04f, 0.07f) * cb_Time;
    uvB += float2(-0.03f, 0.04f) * cb_Time;
    
    float3 normalA = g_tNormalTextureA.Sample(g_sSampler, uvA).rgb;
    float3 normalB = g_tNormalTextureB.Sample(g_sSampler, uvB).rgb;
    
    // そのままだと、0～1になっているので、-1～1になるように補正する
    normalA = normalA * 2.0f - 1.0f;
    normalB = normalB * 2.0f - 1.0f;
    
    // 合成する
    float3 waterNormal = normalize(normalA + normalB);
    
    float waterStlength =10.0f;
    waterNormal.xy *= waterStlength;
    waterNormal = normalize(waterNormal);
    
    float3 waterColor = float3(0.2f, 0.3f, 0.3f);

    float2 screenUV;
    screenUV.x = input.Pos.x / cb_WindowWidth;
    screenUV.y = input.Pos.y / cb_WindowHeight;
    
    // 深度テクスチャのサンプリング
    float sceneDepthRaw =
    g_tSceneDepth.Sample(g_sSampler, screenUV).r;
    
    // ビュー空間の深度値を求める
    float sceneDepth = LinearizeDepth(sceneDepthRaw);
    
    // 地面と水面との差
    float depthDiff = sceneDepth - input.ViewDepth;
    
    float ShallowDistance = 2.5f;   // 
    float shallowFactor = saturate(depthDiff / ShallowDistance);
    
    float3 DeepColor = float3(0.05f, 0.05f, 0.05f); // 深部カラー（暗い）
    float3 ShallowColor = float3(0.9, 0.9, 0.9f);   // 浅瀬カラー（白に近い）
    
    finalColor =
        lerp(
            ShallowColor,
            DeepColor,
            shallowFactor
        );
    
    //************************************************************************
    //                      ディレクションライト計算
    //************************************************************************
    for (int dirIdx = 0; dirIdx < DIRECTIONLIGHT_MAX_NUM; dirIdx++)
    {
        float3 lightDir = cb_DirLightData[dirIdx].Direction;
        float lighting = saturate(dot(waterNormal, lightDir));
        finalColor += waterColor * (0.5f + lighting * 0.5f);
    }
    
    float ShallowAlpha = 0.1f;  // 最浅瀬のアルファ
    float DeepAlpha = 0.85f;    // 最深部のアルファ
    
    finalAlpha =
        lerp(
            ShallowAlpha,
            DeepAlpha,
            shallowFactor
        );

    
    return float4(finalColor, finalAlpha);

}