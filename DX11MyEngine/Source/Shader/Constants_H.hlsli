/* ◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇
// 
/* - @:定数の定義ヘッダ - */
//
//  【?】定数の定義してるよ(バッファの方じゃないよ)
// 
/* ◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇ */
#ifndef CONSTANTS_HLSLI
#define CONSTANTS_HLSLI


static const int POINTLIGHT_MAX_NUM     = 100;                      // ポイントライトの最大数
static const int SPOTLIGHT_MAX_NUM      = 20;                       // スポットライトの最大数
static const int DIRECTIONLIGHT_MAX_NUM = 1;                        // ディレクションライトの最大数
static const float SHADOW_STRENGTH      = 0.3f;                     // 影の濃さ（color * SHADOW_STRENGTH）
static const float3 AMBIENT_COLOR       = float3(0.3f, 0.3f, 0.3f); // アンビエントカラー


#endif