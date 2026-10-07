/* ◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇
// 
/* - @:ライトヘッダー - */
//
//  【?】ライト処理をまとめたもの
// 
/* ◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇ */
#ifndef LIGHTFUNCTIONS_HLSLI
#define LIGHTFUNCTIONS_HLSLI

#include "ConstantBuffers_H.hlsli"
    
/* ライティング処理 */
/* ////////////////////////////////////////////////////////////////////////////////////////////
*  頂点の法線とライトの向きの内積を取り、値が小さいならライトの向きと近いということであり、
*  近いということはライトの効果が及びにくい場所なので、暗くする。
*  計算した内積を反転したものをクランプし、カラー値にしている。
*/ ////////////////////////////////////////////////////////////////////////////////////////////
  


//*---------------------------------------------------------------------------------------
//*【?】正規化 Lambert拡散反射光（ディフューズ）の計算
//* [引数]
//* _ligDir     : ライトの向き
//* _ligCol     : ライトの色
//* _norm       : 頂点法線
//*
//* [返値]
//* float3 : ディフューズ色
//*----------------------------------------------------------------------------------------
float3 LambertDiffuseLightCalc(float3 _ligDir, float3 _ligCol, float3 _norm)
{
    float3 finalDfs = float3(0.0f, 0.0f, 0.0f);
    
    // ライトの向きと法線の内積を求める（ポリゴンとライトがどれくらい当たっているか）
    float diffuseFactor = saturate(dot(-_ligDir, _norm));
    
    // 拡散反射求める
    finalDfs = _ligCol * diffuseFactor;
    
    /*
    *   ・正規化Lambert反射
    *   エネルギー保存の法則を守るため、反射結果をπで除算する
    */
    finalDfs /= 3.1415926f;
    
    return (finalDfs);    
    //return saturate(finalDfs);
}


//*---------------------------------------------------------------------------------------
//*【?】Phong鏡面反射光（スペキュラ）の計算
//* [引数]
//* _ligDir     : ライトの向き（ライトからサーフェイス）
//* _eyePos     : カメラ座標（ワールド）
//* _LigCol     : ライト色
//* _spcPower   : スペキュラ強度
//* _worldPos   : サーフェイス座標（ワールド）
//* _norm       : サーフェイス法線（ワールド）
//*
//* [返値]
//* float3 : スペキュラ色
//*----------------------------------------------------------------------------------------
float3 PhongSpecularLightCalc(float3 _ligDir, float3 _eyePos, float3 _spcCol, float _spcPower, float3 _worldPos, float3 _norm)
{
    float3 finalSpc = float3(0.0f, 0.0f, 0.0f);

    // 反射ベクトル
    float3 refVec = reflect(_ligDir, _norm);
    
    // サーフェイスから視点に向かって伸びるベクトル
    float3 toEye = _eyePos.xyz - _worldPos.xyz;
    toEye = normalize(toEye);             // 正規化
    
    
    // 内積を求める（鏡面反射の強さ）
    float refFactor = max(0.0, saturate(dot(toEye, refVec)));
    
    
    // ※ ポイントライトがおかしくなっていたのはここが原因…？
    // ここを元々0.0だったところを1.0にしたら円形にはなったけど、果たしてこれでいいのだろうか？
    // 魔導書などを見ても、0.0にするように書いてあったのでモヤモヤ...
    // まあ確かに0にしたら0だからカメラと反対側は消えるけども…
    //refFactor = max(0.0, refFactor);      // マイナスにならないよう
    // ※11/28 追記
    // 結論、y座標が床と同じだったから変な感じに見えてる説が濃厚
    // もうそういうもんだと思うことにする。
    
    // ※12/1 追記
    // https://shadered.org/app?fork=qtAFNPvHIE
    // で値を変えて確認してみた感じ、スペキュラの強さに入れる値が
    // 小さすぎたのが原因の可能性特大
    // 試しに以下の_spcPower部分を0.1と100で入れて比べてみるとわかる。
    refFactor = pow(refFactor, _spcPower); // 反射の強さを絞る

    // 鏡面反射求める
    finalSpc = _spcCol * refFactor;
    
    return finalSpc;
}


//*---------------------------------------------------------------------------------------
//*【?】Blinn_Phong鏡面反射光（スペキュラ）の計算
//*     こっちはPhongの改良版みたいなやつらしい
//*     ハーフベクトルを使って計算
//*
//* [引数]
//* _ligDir     : ライトの向き
//* _eyePos     : カメラ座標（ワールド）
//* _LigCol     : ライトの色
//* _spcPower   : 反射の強さ
//* _worldPos   : 頂点ワールド座標
//* _norm       : 頂点法線
//*
//* [返値]
//* float3 最終色
//*----------------------------------------------------------------------------------------
float3 Blinn_PhongSpecularLightCalc(float3 _ligDir, float3 _eyePos, float3 _spcCol, float _spcPower, float3 _worldPos, float3 _norm)
{
    float3 finalSpc = float3(0.0f, 0.0f, 0.0f);
    
    // サーフェイスから視点に向かって伸びるベクトル
    float3 toEye = _eyePos.xyz - _worldPos.xyz;
    toEye = normalize(toEye);               // 正規化

    float3 halfVec = normalize(_ligDir + toEye);     // ライトの向きと視線でハーフベクトルを求める
    float refFactor = saturate(dot(halfVec,_norm )); // 内積を求める（鏡面反射の強さ）
    
    refFactor = pow(refFactor, _spcPower); // 反射の強さを絞る

    // 鏡面反射求める
    finalSpc = _spcCol * refFactor;
    
    return finalSpc;
}


struct OUT_DiffAndSpec
{
    float3 Diffuse;
    float3 Specular;
};

//*---------------------------------------------------------------------------------------
//*【?】ディレクションライトの計算
//*
//* [引数]
//* _ligData    : ライト情報
//* _eyePos     : カメラ座標
//* _spcCol     : スペキュラカラー
//* _spcPow     : スペキュラ強度
//* _worldPos   : サーフェイスのワールド座標
//* _norm       : サーフェイスのワールド法線
//*
//* [返値]
//* OUT_DiffAndSpec ディフューズ＆スペキュラ
//*----------------------------------------------------------------------------------------
OUT_DiffAndSpec DirectionLightCalc(DirectionalLightData _ligData, float3 _eyePos, float3 _spcCol, float _spcPow, float3 _worldPos, float3 _norm)
{
    float3 finalLig = float3(0.0f, 0.0f, 0.0f);
    
    float3 ligDir = normalize(_ligData.Direction);
    
    // 拡散（ディフューズ）反射
    float3 diffuseLig = LambertDiffuseLightCalc(ligDir, _ligData.DiffuseColor, _norm);
    
    // 鏡面（スペキュラ）反射
    float3 specularLig = PhongSpecularLightCalc(ligDir, _eyePos, _spcCol, _spcPow, _worldPos, _norm);
    
    // Blinn版
    //float3 specularLig = Blinn_PhongSpecularLightCalc(-ligDir, _eyePos, _spcCol, _spcPow, _worldPos, _norm);
    
    // ディフューズが0の場合に、スペキュラだけ浮いて見えてしまうので、ディフューズの強さも考慮してスペキュラの強さを決める
    float diffFactor = abs(sign(_ligData.DiffuseIntensity));
    
    specularLig *= _ligData.SpecularIntensity * diffFactor;
    diffuseLig *= _ligData.DiffuseIntensity; 
    
    OUT_DiffAndSpec outData;
    outData.Diffuse = diffuseLig;
    outData.Specular = specularLig;
    
    return outData;
}


//*---------------------------------------------------------------------------------------
//*【?】ポイントライトの計算
//*
//* [引数]
//* _ligData    : ライト情報
//* _eyePos     : カメラ座標
//* _spcCol     : スペキュラカラー
//* _spcPow     : スペキュラ強度
//* _worldPos   : サーフェイスのワールド座標
//* _norm       : サーフェイスのワールド法線
//*
//* [返値]
//* OUT_DiffAndSpec ディフューズ＆スペキュラ
//*----------------------------------------------------------------------------------------
OUT_DiffAndSpec PointLightCalc(PointLightData _ligData, float3 _eyePos, float3 _spcCol, float _spcPow, float3 _worldPos, float3 _norm)
{
    float3 finalLig = float3(0, 0, 0);

    float3 ligDir = _worldPos -_ligData.Pos;    // 光の向きを求める（ライト→サーフェイス）
    float distance = length(ligDir);            // サーフェイス→ライトの距離
    ligDir = normalize(ligDir);                 // 正規化
    
    // ディフューズ計算
    float3 diffPoint = LambertDiffuseLightCalc(ligDir, _ligData.DiffuseColor, _norm);
    
    // フォン反射
    float3 spcPoint = Blinn_PhongSpecularLightCalc(ligDir, _eyePos, _spcCol, _spcPow, _worldPos, _norm);
    
    // そのままだと薄すぎる場合があるので少し補正
    //diffPoint += 0.3f;
    //spcPoint += 0.1f;
    
    // 影響度計算
    float affect = 1.0f - min(1.0f, distance / _ligData.Range);
    
    // マイナスにならないように
    affect = max(0.0, affect);
    affect = pow(affect, 1.0);
    
    // 影響度を拡散・鏡面に反映させる
    diffPoint *= affect;
    spcPoint  *= affect;
    
    OUT_DiffAndSpec outData;
    outData.Diffuse  = diffPoint * _ligData.DiffuseIntensity;
    outData.Specular = spcPoint  * _ligData.SpecularIntensity;
    return outData;
}

//*---------------------------------------------------------------------------------------
//*【?】スポットライトの計算
//*
//* [引数]
//* _ligData    : ライト情報
//* _eyePos     : カメラ座標
//* _spcCol     : スペキュラカラー
//* _spcPow     : スペキュラ強度
//* _worldPos   : サーフェイスのワールド座標
//* _norm       : サーフェイスのワールド法線
//*
//* [返値]
//* OUT_DiffAndSpec ディフューズ＆スペキュラ
//*----------------------------------------------------------------------------------------
OUT_DiffAndSpec SpotLightCalc(SpotLightData _ligData, float3 _eyePos, float3 _spcCol, float _spcPow, float3 _worldPos, float3 _norm)
{
    float3 finalLig = float3(0, 0, 0);

    float3 ligDir = _worldPos - _ligData.Pos; // 光の向きを求める（ライト→サーフェイス）
    float distance = length(ligDir); // サーフェイス→ライトの距離
    ligDir = normalize(ligDir); // 正規化
    
    // ディフューズ計算
    float3 diffuse = LambertDiffuseLightCalc(ligDir, _ligData.DiffuseColor, _norm);
    
    // フォン反射
    float3 specular = Blinn_PhongSpecularLightCalc(ligDir, _eyePos, _spcCol, _spcPow, _worldPos, _norm);
    
    // 影響度計算
    float affect = 1.0f - min(1.0f, distance / _ligData.Range);
    
    // マイナスにならないように
    affect = max(0.0, affect);
    affect = pow(affect, 16.0);
    
    // 影響度を拡散・鏡面に反映させる
    diffuse *= affect;
    specular *= affect;
    
    // 入射角と射出方向の角度を求める
    float spotAngle = max(dot(ligDir, _ligData.Direction),0.0f);
    
    // acosで角度を求める
    spotAngle = abs(acos(spotAngle));
    
    // 角度に比例して小さくなっていく影響率を計算
    affect = 1.0f - 1.0f / _ligData.Angle * spotAngle;
    
    // マイナスにならないように
    affect = max(0.0f, affect);
    
    diffuse *= affect;
    specular *= affect;
    
    OUT_DiffAndSpec outData;
    outData.Diffuse = diffuse * _ligData.DiffuseIntensity;
    outData.Specular = specular * _ligData.SpecularIntensity;
    return outData;
}

//*---------------------------------------------------------------------------------------
//*【?】半球ライト計算
//* [引数]
//* _norm    : 頂点法線
//* [返値]
//* float3 半球色
//*----------------------------------------------------------------------------------------
float3 HemisphereLightCalc(float3 _norm)
{
    float3 finalLig     = float3(0, 0, 0);
    float3 groundNorm   = float3(0.0f, 1.0f, 0.0f);
    float3 skyColor     = float3(0.1f, 0.1f, 0.25f);
    float3 groundColor  = float3(0.16f, 0.16f, 0.1f);
    
    float factor = dot(_norm, groundNorm);
    
    // 0～1に変換
    factor = (factor + 1.0f) * 0.5f;
    
    // 地面色と天球色を補間
    finalLig = lerp(groundColor, skyColor, factor);
    
    return finalLig;
}


//*---------------------------------------------------------------------------------------
//*【?】法線情報取得
//*     ノーマルマップの凹凸情報は接空間（TBN空間）として入っているので、
//*     ライティング計算時にはワールド空間に変換しなければならない。
//*     まあテクスチャ情報だからワールド空間として入ってるわけないよねっていう
//*
//*     ・接ベクトルがテクスチャの U 座標
//*     ・従ベクトルがテクスチャの V 座標
//*
//* [引数]
//* _normMap    :  法線マップ
//* _tan        :  接ベクトル
//* _biNorm     :  従ベクトル
//* _localNorm  :  サーフェイスのローカル法線
//*
//* [返値]
//* float3 最終色
//*----------------------------------------------------------------------------------------
float3 GetNorm(float4 _normMap, float3 _tan, float3 _biNorm, float3 _localNorm)
{
    // 今のままだと0～1の値になっているので、-1～1に変換
    _normMap = _normMap * 2.0f - 1.0f;
    
    // タンジェントスペースの法線をワールドスペースに変換
    return normalize(
             _tan       * _normMap.x +
             _biNorm    * _normMap.y +
             _localNorm * _normMap.z);
};
#endif