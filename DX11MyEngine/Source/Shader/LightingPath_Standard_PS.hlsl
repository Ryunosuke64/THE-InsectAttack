// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
/* - @:ピクセルシェーダ -*/
//
//  【?】ディファードシェーディング
//       ライティングパス用
//
// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
#pragma once
#include "LightFunctions_H.hlsli"
#include "UtilityFunctions_H.hlsli"
#include "DebugFunctions_H.hlsli"


// ↓これがあるとRenderDocでソース単位のデバッグができるらしいがはてさて
//#pragma enable_d3d11_debug_symbols
// 追記：CSOファイル読み込みにすればデバッグ可能

SamplerState g_sSampler : register(s0);
//SamplerComparisonState g_sShadowSampler : register(s1);   // シャドウマップ用
SamplerState g_sShadowSampler : register(s1);               // シャドウマップ用
Texture2D<float4> g_tAlbedoTexture : register(t0);          // rgbにアルベド aにエミッシブ
Texture2D<float4> g_tNormalTexture : register(t1);          // rgbに法線
Texture2D<float4> g_tSpecularTexture : register(t2);        // rgbにスペキュラ色  wにスペキュラ強度
Texture2D<float4> g_tEmissiveMapTexture : register(t3);     // エミッシブ
Texture2D<float4> g_tDepthTexture : register(t4);           // rに深度値 gbaにエミッシブカラー
Texture2D<float4> g_tShadowMapTexture : register(t5);       // シャドウマップ
TextureCube g_tEnvironmentCube : register(t6);              // 環境マップ

/* =========================================================================
/* - @:出力構造体 - */
/* =========================================================================*/
struct PS_IN
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};

// **************************************************************************
/* - @:エントリーポイント - */
// **************************************************************************
float4 PSMain(PS_IN input) : SV_TARGET
{
    float4 albedoTex = g_tAlbedoTexture.Sample(g_sSampler, input.UV);
    float4 normalTex = g_tNormalTexture.Sample(g_sSampler, input.UV);
    float4 depthTex = g_tDepthTexture.Sample(g_sSampler, input.UV);
    float4 specularTex = g_tSpecularTexture.Sample(g_sSampler, input.UV);
    float4 emissiveTex = g_tEmissiveMapTexture.Sample(g_sSampler, input.UV);

    float4 finalCol = float4(0.0, 0.0, 0.0, 1.0);
    
    float depth = depthTex.r; // 深度値
    float4 worldPos; // 深度情報からワールド座標を計算する。
    
    
    // 遠すぎる物体にはライティングしない( ´Д｀)ﾉ~ﾊﾞｲﾊﾞｲ
    if (depth >= 1.0f)
    {
        return float4(0, 0, 0, 1);
    }

    // 深度値からワールド座標を求める
    worldPos = GetConvertPosFromDepthToWorld(depth, input.UV);
    worldPos.w = 1.0f;
    
    // 法線取り出す（0～1 を -1～1 に）
    float3 normal = normalTex.xyz * 2.0f - 1.0f;
    normal = normalize(normal); // 頂点シェーダですでに正規化済み
    
    // スペキュラ強度・カラー
    float3 spcColor = specularTex.rgb;
    
    // スペキュラ値を復元
    float spcPow = specularTex.a;
    spcPow *= 255;
    
    // エミッシブ値を復元
    float3 emissiveColor = emissiveTex.rgb; // デプスのRGBにエミッシブカラー入れてる
    
    OUT_DiffAndSpec dirLig;             // ディレクション用
    dirLig.Diffuse = float3(0, 0, 0);
    dirLig.Specular = float3(0, 0, 0);
    OUT_DiffAndSpec pointLig;           // ポイント用
    pointLig.Diffuse = float3(0, 0, 0);
    pointLig.Specular = float3(0, 0, 0);
    
    float3 limLig = float3(0, 0, 0);
    
    //************************************************************************
    //                      ディレクションライト計算
    //************************************************************************
    for (int dirIdx = 0; dirIdx < DIRECTIONLIGHT_MAX_NUM; dirIdx++)
    { 
        OUT_DiffAndSpec res = DirectionLightCalc(cb_DirLightData[dirIdx], cb_EyePos, spcColor, spcPow, worldPos.xyz, normal);
        dirLig.Diffuse += res.Diffuse;
        dirLig.Specular += res.Specular;
        
        // ***************************************************************
        //                          リムライト
        // ***************************************************************
        // 視線の方向
        float3 viewDir = normalize(cb_EyePos - worldPos.xyz);
        
        // ライトとサーフェスの法線の内積を計算
        float power1 = 1.0f - max(0.0f, saturate(dot(normalize(cb_DirLightData[dirIdx].Direction), normal)));
        
        // 視線と法線の内積を計算
        float power2 = 1.0f - max(0.0f, saturate(dot(viewDir, normal)));
        
        // 強さを決定
        float limPower = power1 * power2;
        limPower = pow(limPower, 3.0f);
        
        // リムライトを乗算
        limLig += dirLig.Diffuse * limPower;
    }
    
    //************************************************************************
    //                      ポイントライト計算 
    //************************************************************************
    for (int pointIdx = 0; pointIdx < cb_PointLightCount; pointIdx++)
    {
        OUT_DiffAndSpec res = PointLightCalc(cb_PointLightData[pointIdx], cb_EyePos, spcColor, spcPow, worldPos.xyz, normal);
        pointLig.Diffuse += res.Diffuse;
        pointLig.Specular += res.Specular;
    }
    
    //************************************************************************
    //                      スポットライト計算 
    //************************************************************************
    for (int spotIdx = 0; spotIdx < cb_SpotLightCount; spotIdx++)
    {
        OUT_DiffAndSpec res = SpotLightCalc(cb_SpotLightData[spotIdx], cb_EyePos, spcColor, spcPow, worldPos.xyz, normal);
        pointLig.Diffuse += res.Diffuse;
        pointLig.Specular += res.Specular;
    }
    
    
    // 天球ライト
    float3 hemiLig = HemisphereLightCalc(normal);
    
    // ディレクションライト + ポイントライト + 天球 + アンビエント
    //float3 lighting = dirLig + pointLig + hemiLig + 0.1f;
    // 平行光源のみシャドウの影響を受けるので、一旦別で保持
    float3 dirDiffuse = dirLig.Diffuse + limLig;
    float3 dirSpecular = dirLig.Specular;
    float3 dirLightColor =  float3(0.0f, 0.0f, 0.0f);   // ディレクションライト（シャドウの影響を受ける）
    float3 pointLightColor = float3(0.0f, 0.0f, 0.0f);  // ポイントライト
    float3 afterShadowColor = float3(0.0f, 0.0f, 0.0f); // シャドウ適用後のカラー値
    float3 ambientColor = albedoTex.xyz * AMBIENT_COLOR; // アンビエントカラー
    
    // 最終色 (アルベド * 光度 + スペキュラ) + エミッシブ
    dirLightColor = (albedoTex.xyz * dirDiffuse + dirSpecular);
    pointLightColor = (albedoTex.xyz * pointLig.Diffuse );
    
    //************************************************************************
    // シャドウ
    //************************************************************************
    // シャドウの有効/無効
    
    [branch]    // 動的に変えるということを明示的に指示するやつ？
    if (cb_EnableShadow == true)
    {
    
        float3 shadowColor = float3(0.0f, 0.0f, 0.0f);
        float4 posInLVP = mul(worldPos, cb_DirLightData[0].LightViewProj);
    
        //posInLVP.xyz /= posInLVP.w;
    
        // ライトビュースクリーン空間（NDC）からUV空間に座標変換
        // -1.0 ～ 1.0 を 0.0～1.0に
        // 平行投影の場合は設定次第でw除算は要らないっぽい
    
        float2 shadowMapUV = posInLVP.xy;
        shadowMapUV *= float2(0.5f, -0.5f);
        shadowMapUV += 0.5f;
        // ライトから見た深度値を計算
        float zInLVP = posInLVP.z;
        float shadowFactor = 1.0f;
    
        // バイアス参考サイト https://project-asura.com/articles/d3d11/d3d11_008.html
        // 最大深度傾斜を求める.
        //float maxDepthSlope = max(abs(ddx(zInLVP)), abs(ddy(zInLVP)));
        //float bias = cb_BaseShadowBias; // 固定バイアス
        //float slopeScaledBias = cb_SlopeScaledBias; // 深度傾斜
        //float depthBiasClamp = cb_DepthBiasClamp; // バイアスクランプ値
        //float shadowBias = bias + slopeScaledBias * maxDepthSlope;
        //shadowBias = min(shadowBias, depthBiasClamp); // クランプ

        // シャドウマップから値をサンプリング
        float2 shadow = g_tShadowMapTexture.Sample(g_sShadowSampler, shadowMapUV).rg;
    
        //if (zInLVP > shadow.r && zInLVP <= 1.0f)
        {
            //チェビシェフの不等式を利用して光が当たる確率を求める
            float depth_sq = shadow.r * shadow.r;
        
            // 分散具合を求める
            // 分散が大きいほど、varianceも大きくなる
            float variance = min(max(shadow.g - depth_sq, 0.0000001f), 1.0f);
        
            // このピクセルのライトから見た深度値とシャドウマップの平均の深度値との差を求める
            float md = zInLVP - shadow.r;
        
            // 光が届く確率を求める
            float lit_Factor = variance / (variance + md * md);
        
            //（UV 0.0～1.0）の端でフェードさせる
            // 端から指定範囲の領域で滑らかに影を薄くする
            // 今は端から30%ぐらいのところからフェードさせている
            float2 edge = smoothstep(0.0, 0.3, shadowMapUV) * smoothstep(1.0, 0.7, shadowMapUV);
            float vignette = edge.x * edge.y;
        
            // 範囲外ではvignetteが0になるので、1.0で明るくなる
            lit_Factor = lerp(1.0f, lit_Factor, vignette);
        
            // 現在のカラーより暗く
            shadowColor = dirLightColor.xyz * SHADOW_STRENGTH;
        
            // 通常カラーとシャドウカラーで線形補間
            afterShadowColor.xyz = lerp(shadowColor, dirLightColor.xyz, lit_Factor);
        }
        
        // アンビエント + シャドウ後カラー + ポイントライト + エミッシブ
        finalCol.xyz = ambientColor + afterShadowColor + pointLightColor + emissiveColor;
    }
    // シャドウなしの場合は、そのままディレクションライトの結果を入れる
    else
    {
        finalCol.xyz = ambientColor + dirLightColor + pointLightColor + emissiveColor;
    }
    
    //// フォグ（TODO:スカイボックスに適用されないので、どうしようか）
    //float3 FogColor = { 0.6f, 0.7f, 0.8f };
    //float FogStart = 30.0f;
    //float FogEnd = 150.0f;
    
    //float distanceFromCamera =
    //    distance(worldPos.xyz, cb_EyePos);
    //float fogFactor =
    //    saturate(
    //        (FogEnd - distanceFromCamera) /
    //        (FogEnd - FogStart)
    //    );
    //finalCol.rgb =
    //    lerp(
    //        FogColor,
    //        finalCol.rgb,
    //        fogFactor
    //    );

    
    // シャドウマップの範囲内か
    //if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f &&
    //shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    //{
    //// シャドウマップから深度値をサンプリング
    //// zinLVP : この値が比較するテクセルより大きければ1.0、小さければ0.0

    //// 現在のカラーより暗く
    //    shadowColor = finalCol.xyz * 0.5f;
    //// shadowFactor : 1.0 = 光が当たっている, 0.0 = 影
    //    finalCol.xyz = lerp(shadowColor, finalCol.xyz, shadowFactor);
        //}
    
    
    //float reflectionStrength = emissiveTex.a;

    //if (reflectionStrength > 0.0f)
    //{
    //    float3 N = normalize(normal);
    //    float3 toEye = normalize(cb_EyePos - worldPos.xyz);

    //// カメラから表面へ入ってくる向き
    //    float3 reflectionDir = reflect(-toEye, N);

    //    float3 environmentColor =
    //    g_tEnvironmentCube.Sample(g_sSampler, reflectionDir).rgb;

    //// 正面よりも斜めから見た方が強く反射する
    //    float fresnel =
    //    pow(1.0f - saturate(dot(N, toEye)), 5.0f);

    //    float reflectionWeight =
    //    saturate(reflectionStrength * lerp(0.15f, 1.0f, fresnel));

    //    finalCol.rgb =
    //    lerp(finalCol.rgb, environmentColor, reflectionWeight);
    //}
    
    //************************************************************************
    // 最終調整
    //************************************************************************
    finalCol.a = 1.0f;
    
    return (finalCol);
}
