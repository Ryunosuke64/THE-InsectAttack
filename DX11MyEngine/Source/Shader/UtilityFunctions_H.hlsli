/* ◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇
// 
/* - @:汎用関数ヘッダー - */
//
//  【?】汎用関数をまとめたもの
//       いろんなところで使えそうなものをまとめてるよ
// 
/* ◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇ */
#ifndef UTILITYFUNCTIONS_HLSLI
#define UTILITYFUNCTIONS_HLSLI
#include "ConstantBuffers_H.hlsli"

//*---------------------------------------------------------------------------------------
//*【?】平均ブラーを求める
//* 引数：1.ブラーの強さ
//* 引数：2.ディスプレイ横幅
//* 引数：3.ディスプレイ縦幅
//* 引数：4.反映させるテクスチャー
//* 引数：5.UV
//* 引数：6.サンプラー
//* 返値：ブラー後のカラー
//*----------------------------------------------------------------------------------------
float4 AverageBlur(float _blurPower, float _screenW, float _screenH, Texture2D _tex, float2 _uv, SamplerState _samp)
{
    // 基準テクセル＋近傍8テクセルの平均を計算する 
    // 2.5テクセルずらすためのUV値を求める
    float offsetU = _blurPower / _screenW;
    float offsetV = _blurPower / _screenH;

    float4 res = float4(0, 0, 0, 1);
    
    // 基準テクセルからずらす
    res += _tex.Sample(_samp, _uv + float2( offsetU,  0.0f));    // 右
    res += _tex.Sample(_samp, _uv + float2(-offsetU,  0.0f));    // 左
    res += _tex.Sample(_samp, _uv + float2( 0.0f,     offsetV)); // 下
    res += _tex.Sample(_samp, _uv + float2( 0.0f,    -offsetV)); // 上
    res += _tex.Sample(_samp, _uv + float2( offsetU,  offsetV)); // 右下
    res += _tex.Sample(_samp, _uv + float2( offsetU, -offsetV)); // 右上
    res += _tex.Sample(_samp, _uv + float2(-offsetU,  offsetV)); // 左下
    res += _tex.Sample(_samp, _uv + float2(-offsetU, -offsetV)); // 左上
    res /= 9.0f; // 基準テクセルと近傍８テクセルの平均なので9で除算
    
    return res;
}

//*---------------------------------------------------------------------------------------
//*【?】モノクロ化
//* 引数：1.反映させるアルベド色
//* 返値：モノクロ後のカラー
//*----------------------------------------------------------------------------------------
float3 Monochrome(float3 albed)
{
    float3 res = albed;
    
    // モノクロ化
    float Y = 0.299f * albed.r + 0.587f * albed.g + 0.114f * albed.b;
    res.x = Y;
    res.y = Y;
    res.z = Y;
    
    return res;
}

//*---------------------------------------------------------------------------------------
//*【?】深度値からワールド座標に変換
//* 引数：1.深度値
//* 引数：2.UV座標
//* 返値：ワールド座標
//*----------------------------------------------------------------------------------------
float4 GetConvertPosFromDepthToWorld(float depth, float2 uv)
{
    float4 worldPos = float4(0, 0, 0, 0); // ワールド座標
    float4 ndcPos = float4(0, 0, 0, 0); // NDC空間（正規化デバイス座標）
    
    // 正規化デバイス座標系での座標を計算する。
    // z座標は深度テクスチャから引っ張ってくる。
    ndcPos.z = depth;
    
    // ndc()
    // uv[]
    //
    //    (-1.0,1.0) [0.0,0.0]                (1.0,1.0) [1.0,0.0]
    //     --------------------------------------------- 
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     |                                           |
    //     --------------------------------------------- 
    //    (-1.0,-1.0) [0.0,1.0]              (1.0,-1.0) [1.0,1.0]
    //
    
    // xy座標はUV座標からNDC座標への変換
    ndcPos.xy = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    ndcPos.w = 1.0f;
    
    // クリッピング座標
    float4 clipPos = ndcPos;
    
    // クリップ座標にビュープロジェクション行列の逆行列を乗算して、ワールド座標に戻す。
    worldPos = mul(clipPos, cb_ViewProjInvMatrix);
    
    // Wで除算し、3D空間に戻す
    worldPos.xyz /= worldPos.w;
    
    return worldPos;
}



// ディザパターン
static const int ditherPattern[4][4] =
{
    { 0, 32, 8, 40 },
    { 48, 16, 56, 24 },
    { 12, 44, 4, 36 },
    { 60, 28, 52, 20 },
};
//*---------------------------------------------------------------------------------------
//*【?】ディザリングクリップ
//* 引数：1.スクリーン座標
//* 引数：2.閾値
//* 返値：なし
//*----------------------------------------------------------------------------------------
void DitheringClip(float2 screenPos, float threshold_value)
{
    // このピクセルのスクリーン座標系でのX座標、Y座標を4で割った余りを求める
    int x = (int) fmod(screenPos.x, 4.0f);
    int y = (int) fmod(screenPos.y, 4.0f);
    // このピクセルの閾値を取得
    int dither = ditherPattern[x][y];
    // 閾値が 10 以下のピクセルはピクセルキルする
    clip(dither - threshold_value);
}

//*---------------------------------------------------------------------------------------
//*【?】0～1の深度値をView空間の距離にする
//* 引数：1.深度
//* 返値：線形深度値
//*----------------------------------------------------------------------------------------
float LinearizeDepth(float depth)
{
    return cb_Projection._43 /
           (depth - cb_Projection._33);
}

#endif