// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
/* - @:ピクセルシェーダ -*/
//
//  【?】デカールの書き込み用
//       現在はテクスチャが端で途切れるようにしているけど、綺麗に見せるなら端から先へテクスチャが綺麗に張られるようにする必要がある。
//      以下の「Tri-Planar Texture Mapping」というやり方で行けるかも？一旦今回は、他の個所を優先するため保留
//      https://qiita.com/edo_m18/items/c8995fe91778895c875e
//
//
// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
#pragma once
#include "ConstantBuffers_H.hlsli"
#include "UtilityFunctions_H.hlsli"
SamplerState g_sSampler : register(s0);
SamplerState g_sClampSampler : register(s2);
Texture2D g_tAlbedTexture : register(t0);    // カラーテクスチャ
Texture2D g_tNormalTexture : register(t1);   // 法線テクスチャ
Texture2D g_tDepthTexture : register(t2);    // 深度値Gバッファ


/* =========================================================================
/* - @:入力構造体 - */
/* =========================================================================*/
struct PS_IN
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

/* =========================================================================
/* - @:MRT出力構造体 - */
/* =========================================================================*/
struct PS_OUT
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
};

// **************************************************************************
/* - @:エントリーポイント - */
// **************************************************************************
PS_OUT PSMain(PS_IN input) 
{
    PS_OUT output;
    
    float4 albedColor = float4(0.0, 0.0, 0.0, 1.0);
    float4 normalColor = float4(0.0, 0.0, 0.0, 1.0);
    
    // スクリーン空間のUVを求める（デカールはボックスの為そのままでは使えない）
    float2 screenUV = input.Pos.xy / float2(cb_WindowWidth, cb_WindowHeight);
    
    // 深度値
    float depth = g_tDepthTexture.Sample(g_sSampler, screenUV).r;
    
    // 深度値からワールド座標求める
    float4 worldPos = GetConvertPosFromDepthToWorld(depth, screenUV);
    worldPos.w = 1.0f;
    
    // デカールボックスワールド逆行列を掛けて、デカールボックスのローカル空間に変換する
    float4 localPosToDecal = mul(worldPos, cb_InvDecalTransform);
    
    // デカールボックスの範囲内か
    // X = 0.6だとしたら0.5 - 0.6になりマイナスの値になるため、クリップされる
    clip(0.5f - abs(localPosToDecal.xyz));
    
    
    
    // ==========================================================
    // テクスチャが引き伸ばされないようにする処理（今は端で途切れるようにしている）
    // ==========================================================
    // ローカル法線を計算
    float3 ddxLocal = ddx(localPosToDecal.xyz);
    float3 ddyLocal = ddy(localPosToDecal.xyz);
    float3 localSurfaceNormal = normalize(cross(ddxLocal, ddyLocal));

    // デカールの投影方向（ローカルのZ軸）と、面の法線がどれくらい向かい合っているか
    // 1.0に近いほど正面、0.0に近いほど側面（引き伸ばされる面）
    float faceAngle = abs(localSurfaceNormal.z);

    // 角度がキツい場合はクリップして描画しない
    clip(faceAngle - 0.2f);
    // ==========================================================

    // UV座標へ変換
    float2 decalUV = localPosToDecal.xy;
    decalUV += 0.5f;    // 0.0 ～ 1.0
    decalUV.y = 1.0f - decalUV.y;
    
    // テクスチャのサンプリング*********************************************
    
    // アルベドテクスチャ
    float4 albedTex = g_tAlbedTexture.Sample(g_sClampSampler, decalUV);
    clip(albedTex.a - 0.1f);    // 透明部分の切り捨て
    
    
    // 法線テクスチャ
    float4 normTex = g_tNormalTexture.Sample(g_sClampSampler, decalUV) ;
    normTex.xyz = normTex.xyz * 2.0f - 1.0f; // -1～1に
    // ワールド空間の法線を求める
    float3 decalWorldNormal = normalize(mul(normTex.xyz, (float3x3) cb_Transform));
    decalWorldNormal = decalWorldNormal * 0.5f + 0.5f; // 0～1戻す
    
    albedColor.rgb = albedTex.rgb * cb_DecalColor * cb_DiffuseColor.rgb;
    albedColor.a = albedTex.a;
    normalColor.rgb = decalWorldNormal.rgb;
    normalColor.a = normTex.a;
    
    // アルベドと法線出力
    output.Albedo = albedColor;
    output.Normal = normalColor;
    
    return output;
}