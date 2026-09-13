#pragma once
// =======================================================================================
//
// *---        Helper         ---*
//		便利関数・構造体群
// =======================================================================================
namespace Tool
{
#define _USE_MATH_DEFINES
#include <math.h>	

	constexpr float G_PI_F   = static_cast<float>(M_PI);	// 円周率 
	constexpr float G_PI_2_F = static_cast<float>(M_PI_2);	// 円周率の半分
	constexpr float G_PI_4_F = static_cast<float>(M_PI_4);	// 円周率の四分の一

    /* デグリーからラジアンに変換 */
    inline float DegToRad(float deg) { return deg * (static_cast<float>(M_PI) / 180.0f); }

    /* ラジアンからデグリーに変換 */
    inline float RadToDeg(float rad) { return rad * (180.0f / static_cast<float>(M_PI)); }

    //-----------------------------------------
    /*      乱数    */
    //-----------------------------------------

    // 指定範囲の乱数取得（rand()なので精度は...ね） 
    // ※なるべくRandomManagerのメルセンヌ・ツイスタの乱数生成を使用して…！
    inline float RandRange(float _min, float _max)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return _min + r * (_max - _min);
    }


    //-----------------------------------------
    /*      イージング関数    */
    // 参考サイト：https://easings.net/ja
    // In : 加速
    // Out : 減速
    //-----------------------------------------
    namespace Easing
    {
        // --------------------------------------
        //                  加速 
        // --------------------------------------

        // 軽い加速
        inline float EaseInQuad(float _t) {
            return _t * _t;
        }

        // 強い加速
        inline float EaseInCubic(float _t) {
            return _t * _t * _t;
        }

        // かなり強い加速
        inline float EaseInQuart(float _t) {
            return _t * _t * _t * _t;
        }

        // 超強い加速
        inline float EaseInQuint(float _t) {
            return _t * _t * _t * _t * _t;
        }

        // 自然な加速
        inline float EaseInSine(float _t) {
            return 1.0f - cosf((_t * DirectX::XM_PI) / 2.0f);
        }


        // --------------------------------------
        //                  減速
        // --------------------------------------

        inline float EaseOutSin(float _t){
            return sinf((_t * DirectX::XM_PI) / 2.0f);
        }

        // ヒュッ...シュタッって感じ
        // 2乗
        inline float EaseOutQuad(float _t) {
            float invT = 1.0f - _t;
            return 1.0f - (invT * invT);
        }

        // 3乗
        inline float EaseOutCubic(float _t) {
            float invT = 1.0f - _t;
            return 1.0f - (invT * invT * invT);
        }

        // 4乗
        inline float EaseOutQuart(float _t) {
            float invT = 1.0f - _t;
            return 1.0f - (invT * invT * invT * invT);
        }

        // 5乗
        inline float EaseOutQuint(float _t) {
            float invT = 1.0f - _t;
            return 1.0f - (invT * invT * invT * invT * invT);
        }


        // --------------------------------------
        //          ばねみたいなやつ
        // --------------------------------------

        // 行って戻る
        inline float EaseOutBack(float t) {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;

            // (t-1) を計算
            float t1 = t - 1.0f;
            return 1.0f + c3 * FLOAT_CAST(pow(t1, 3)) + c1 * FLOAT_CAST(pow(t1, 2));
        }

        // バウンドするやつ
        inline float EaseOutBounce(float t) {
            const float n1 = 7.5625f;
            const float d1 = 2.75f;

            if (t < 1.0f / d1) {
                return n1 * t * t;
            }
            else if (t < 2.0f / d1) {
                return n1 * (t -= 1.5f / d1) * t + 0.75f;
            }
            else if (t < 2.5f / d1) {
                return n1 * (t -= 2.25f / d1) * t + 0.9375f;
            }
            else {
                return n1 * (t -= 2.625f / d1) * t + 0.984375f;
            }
        }
    }


    //-----------------------------------------
    /*      シェイカー構造体    */
    //-----------------------------------------
    struct VEC2_Shaker
    {
        float _shakeTimer;               // 継続時間
        float _shakeDuration;           // 継続時間
        VECTOR2::VEC2 _shaleStrength;   // 揺れの強さ
        VECTOR2::VEC2 _shakeOffset;     // シェイクによる揺れベクトル


        VEC2_Shaker() :
            _shakeTimer(0.0f),
            _shakeDuration(0.0f),
            _shaleStrength(VECTOR2::VEC2()),
            _shakeOffset(VECTOR2::VEC2())
        {
        };

        /// <summary>
        /// シェイク開始
        /// </summary>
        /// <param name="duration">シェイクの持続時間</param>
        /// <param name="strength">シェイクの強さ</param>
        void Start(float duration, const VECTOR2::VEC2& strength)
        {
            _shakeDuration = duration;
            _shaleStrength = strength;
            _shakeTimer = 0.0f;
        }

        /// <summary>
        /// 更新
        /// </summary>
        /// <param name="_deltaTime">デルタタイム</param>
        void Update(float _deltaTime)
        {
            if (_shakeTimer < _shakeDuration) {

                float t = _shakeTimer / _shakeDuration;
                float easeOut = 1.0f - Tool::Easing::EaseOutQuad(t);  // イージングで段々弱まるように

                // 新しい揺れオフセットを生成
                _shakeOffset.x = (RandRange(-1.0f, 1.0f) * _shaleStrength.x) * easeOut;
                _shakeOffset.y = (RandRange(-1.0f, 1.0f) * _shaleStrength.y) * easeOut;

                _shakeTimer += _deltaTime;
            }
            // 終了
            else {
                _shakeOffset = VECTOR2::VEC2(0.0f,0.0f);
                _shakeDuration = 0.0f;
                _shakeTimer = 0.0f;
            }
        }

        // シェイクしたベクトルと引数のベクトルを足して返す
        VECTOR2::VEC2 Apply(const VECTOR2::VEC2& originalPos)const
        {
            return originalPos + _shakeOffset;
        }
    };

    //-----------------------------------------
    /*       3Dベクトル用シェイカー    */
    //-----------------------------------------
    struct VEC3_Shaker
    {
        float _shakeTimer;               // 継続時間
        float _shakeDuration;           // 継続時間
        VECTOR3::VEC3 _shaleStrength;   // 揺れの強さ
        VECTOR3::VEC3 _shakeOffset;     // シェイクによる揺れベクトル


        VEC3_Shaker() : 
            _shakeTimer(0.0f),
            _shakeDuration(0.0f),
            _shaleStrength(VECTOR3::VEC3(0.0f)),
            _shakeOffset(VECTOR3::VEC3(0.0f))
        {
        };

        /// <summary>
        /// シェイク開始
        /// </summary>
        /// <param name="duration">シェイクの持続時間</param>
        /// <param name="strength">シェイクの強さ</param>
        void Start(float duration, const VECTOR3::VEC3& strength)
        {
            _shakeDuration = duration;
            _shaleStrength = strength;
            _shakeTimer = 0.0f;
        }

        /// <summary>
        /// 更新
        /// </summary>
        /// <param name="_deltaTime">デルタタイム</param>
        void Update(float _deltaTime)
        {
            if (_shakeTimer < _shakeDuration){

                float t = _shakeTimer / _shakeDuration;
                float easeOut = 1.0f - Tool::Easing::EaseOutQuad(t);  // イージングで段々弱まるように

                // 新しい揺れオフセットを生成
                _shakeOffset.x =  (RandRange(-1.0f,1.0f) * _shaleStrength.x) * easeOut;
                _shakeOffset.y =  (RandRange(-1.0f,1.0f) * _shaleStrength.y) * easeOut;
                _shakeOffset.z =  (RandRange(-1.0f,1.0f) * _shaleStrength.z) * easeOut;

                _shakeTimer += _deltaTime;
            }
            // 終了
            else{
                _shakeOffset = VECTOR3::VEC3(0.0f);
                _shakeDuration = 0.0f;
                _shakeTimer = 0.0f;
            }
        }

        // シェイクしたベクトルと引数のベクトルを足して返す
        VECTOR3::VEC3 Apply(const VECTOR3::VEC3& originalPos)const
        {
            return originalPos + _shakeOffset;
        }
    };


    

    //-----------------------------------------
    /*      線形補間関数    */
    //-----------------------------------------
    inline float Lerp(float start, float end, float t)
    {
        return (1 - t) * start + t * end;
    }

    template<typename T>
    inline T Lerp(const T& start, const T& end, float t)
    {
        return start + (end - start) * t;
    }

    inline VECTOR2::VEC2 VEC2_Lerp(VECTOR2::VEC2 start, VECTOR2::VEC2 end, float t)
    {
        return {
            ((1 - t) * start.x + t * end.x),
            ((1 - t) * start.y + t * end.y)
        };
    }


    //-----------------------------------------
    /*      UV    */
    //-----------------------------------------
    namespace UV
    {
        /* UV変換用構造体 */
        struct SpriteUV
        {
            float u;   // 左上X（U）
            float v;   // 左上Y（V）
            float uw;  // 幅
            float vh;  // 高さ
        };

        /* 主にスプライトのUV変換を行う際に使う */
        /* 引数：1.左上 X座標  2.左上 Y座標  3.幅  4.高さ  5.画像の横幅  6.画像の縦幅*/
        inline SpriteUV MakeSpriteUV(float spriteX, float spriteY, float spriteW, float spriteH, float texW, float texH)
        {
            SpriteUV uv{};
            uv.u = spriteX / texW;
            uv.v = spriteY / texH;
            uv.uw = spriteW / texW;
            uv.vh = spriteH / texH;
            return uv;
        }

        // スプライトの表示に必要な情報 
        struct SPRITE_DATA {
            ID3D11ShaderResourceView* texture;
            VECTOR2::VEC2 pos;
            float width;
            float height;
            SpriteUV uv;
        };
    }

    //--------------------------------------------------------------------------------------
    /*      当たり判定    */
    //--------------------------------------------------------------------------------------
    namespace Collision
    {
        // 衝突した相手の情報
        struct COLLISION_HIT_TARGET_DATA
        {
            VECTOR3::VEC3 Pos;
            bool hitLeft = false;
            bool hitRight = false;
            bool hitTop = false;
            bool hitBottom = false;
            bool Hit = false;
        };











        // bounding circle : 境界円
        inline bool HitCheckBC(VECTOR2::VEC2 src_pos, float src_r, VECTOR2::VEC2 dst_pos, float dst_r)
        {
            VECTOR2::VEC2 result;
            result = dst_pos - src_pos;

            float length = result.LengthSq();
            float r = src_r + dst_r;

            if ((r * r) > length)
            {
                return true;
            }

            return false;
        }

        // bounding box : 境界箱 AABB -> axis aligned bounding box 軸平行境界箱
        inline bool HitCheckBB(VECTOR2::VEC2 src_pos, float src_w, float src_h, VECTOR2::VEC2 dst_pos, float dst_w, float dst_h)
        {
            // 左側の判定
            if ((src_pos.x + (src_w / 2)) < (dst_pos.x - (dst_w / 2))) return false;

            // 右側の判定
            if ((src_pos.x - (src_w / 2)) > (dst_pos.x + (dst_w / 2))) return false;

            // 上側の判定
            if ((src_pos.y + (src_h / 2)) < (dst_pos.y - (dst_h / 2))) return false;

            // 下側の判定
            if ((src_pos.y - (src_h / 2)) > (dst_pos.y + (dst_h / 2))) return false;

            // 当たっている可能性しか残っていない
            return true;
        }

        inline COLLISION_HIT_TARGET_DATA HitCheckBB_RESULT_HIT_DATA(VECTOR2::VEC2 src_pos, float src_w, float src_h, VECTOR2::VEC2 dst_pos, float dst_w, float dst_h)
        {
            COLLISION_HIT_TARGET_DATA hit;

            // それぞれの辺の座標を計算
            float src_left = src_pos.x + src_w / 2;
            float src_right = src_pos.x - src_w / 2;
            float src_top = src_pos.y + src_h / 2;
            float src_bottom = src_pos.y - src_h / 2;

            float dst_left = dst_pos.x - dst_w / 2;
            float dst_right = dst_pos.x + dst_w / 2;
            float dst_top = dst_pos.y - dst_h / 2;
            float dst_bottom = dst_pos.y + dst_h / 2;

            // AABB判定（重なっていない場合は早期リターン）
            if (src_right > dst_left && src_left < dst_right &&
                src_top < dst_bottom && src_bottom > dst_top)
            {
                return hit; // ヒットしていない（全部false）
            }

            hit.Hit = true;

            float dx = (std::min)(src_right, dst_right) - (std::max)(src_left, dst_left);
            float dy = (std::min)(src_top, dst_top) - (std::max)(src_bottom, dst_bottom);

            if (dx < dy) {
                // 左 or 右の衝突
                if (src_pos.x < dst_pos.x)
                    hit.hitRight = true; // srcがdstの左にいる→右に当たった
                else
                    hit.hitLeft = true;
            }
            else {
                // 上 or 下の衝突
                if (src_pos.y < dst_pos.y)
                    hit.hitTop = true;
                else
                    hit.hitBottom = true;
            }

            // 当たっている可能性しか残っていない
            return hit;
        }
    }

    //************************************************************************
    //
    /* 文字変換関連の参考サイト */
    // https://iifx.dev/ja/articles/1421553/std-string%E3%81%A8std-wstring-c-%E6%96%87%E5%AD%97%E3%82%B3%E3%83%BC%E3%83%89%E3%81%AE%E9%81%B8%E3%81%B3%E6%96%B9%E3%81%A8%E3%83%88%E3%83%A9%E3%83%96%E3%83%AB%E8%A7%A3%E6%B1%BA
    //
    //************************************************************************

    // stringからwstringへの変換
    inline std::wstring StringToWstring(const std::string& str)
    {
        std::wstring ret;
        //一度目の呼び出しは文字列数を知るため
        int len = MultiByteToWideChar(CP_UTF8,
            0,
            str.c_str(),//入力文字列
            static_cast<int>(str.length()),
            nullptr,
            0
        );
        assert(len >= 0);
        ret.resize(len);    //文字数分確保する

        //二度目の呼び出しは変換
        MultiByteToWideChar(CP_UTF8,
            0,
            str.c_str(),//入力文字列
            static_cast<int>(str.length()),
            ret.data(),
            static_cast<int>(ret.size())
        );
        return ret;
    }

    // wstringからstringへの変換
    inline std::string WStringToString(const std::wstring& wstr)
    {
        std::string ret;
        //一度目の呼び出しは文字列数を知るため
        int len = WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr.c_str(),//入力文字列
            static_cast<int>(wstr.length()),
            nullptr,
            0,
            nullptr,
            nullptr
        );
        assert(len >= 0);
        ret.resize(len);    //文字数分確保する

        //二度目の呼び出しは変換
        WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr.c_str(),//入力文字列
            static_cast<int>(wstr.length()),
            ret.data(),
            static_cast<int>(ret.size()),
            nullptr,
            nullptr
        );
        return ret;
    }


    /// <summary>
    /// U8文字列から通常の文字列へ変換
    /// </summary>
    /// <param name="str"></param>
    /// <returns></returns>
    inline const char* U8ToChar(const char8_t* str)
    {
        return reinterpret_cast<const char*>(str);
    }

    // value: 変換したい数値 / precision: 小数点以下の表示桁数（デフォルトは1桁）
    inline std::wstring FormatFloat(float value, int precision = 1)
    {
        std::wstringstream stream;
        // std::fixed で固定小数点表記にし、std::setprecision で桁数を指定
        stream << std::fixed << std::setprecision(precision) << value;
        return stream.str();
    }


    /// <summary>
    /// エラーメッセージの表示
    /// </summary>
    /// <param name="_text">本文</param>
    /// <param name="_caption">タイトル</param>
    inline void ErrorMessage(const std::wstring& _text, const std::wstring& _caption)
    {
        MessageBoxW(NULL, _text.c_str(), _caption.c_str(), MB_OK);
    }


    //-----------------------------------------
    /*      クオータニオン系    */
    //-----------------------------------------
    
    /// <summary>
    /// バラつきクオータニオンを求める
    /// </summary>
    /// <param name="_Out">出力先</param>
    /// <param name="_originalQ">バラつかせるクオータニオン</param>
    /// <param name="_accuracy">バラつき具合（rad）</param>
    /// <returns></returns>
    //DirectX::XMVECTOR CalcQuaternionAccuracy(DirectX::XMVECTOR* _Out, const DirectX::XMVECTOR& _originalQ, const VECTOR3::VEC3& _accuracy)
    //{
    //    VECTOR3::VEC3 accuracy = Master::m_pRandomManager->GetVEC3Random(-_accuracy, _accuracy);
    //    // バラつきクォータニオン
    //    DirectX::XMVECTOR spreadQ = DirectX::XMQuaternionRotationRollPitchYaw(accuracy.x, accuracy.y, accuracy.z);

    //    // 最終的なクォータニオン作成
    //    DirectX::XMVECTOR finalRotQ = DirectX::XMQuaternionMultiply(_originalQ, spreadQ);
    //    finalRotQ = DirectX::XMQuaternionNormalize(finalRotQ); // 念のため正規化

    //    // 出力
    //    *_Out = finalRotQ;

    //    return finalRotQ;
    //}


    /// <summary>
	/// 法線から回転を求める
    /// </summary>
    /// <param name="_normal">法線</param>
    /// <param name="_from">元の向き</param>
    inline void GetRotationFromNormal(const VECTOR3::VEC3& _normal, const VECTOR3::VEC3& _from, VECTOR3::VEC3 &_outAxis, float &_outAngle)
    {
        // 回転軸を求める
		VECTOR3::VEC3 axis = VECTOR3::VEC3::Cross(_from, _normal);

		// 回転角度を求める
		float angle = acosf(VECTOR3::VEC3::Dot(_from, _normal));

		// 出力
		_outAxis = axis.Normalize();
		_outAngle = angle;
    }

    /// <summary>
    /// json用ヘルパー
    /// </summary>
    namespace Json
    {
        /// <summary>
		/// jsonからVEC3型のデータを読み取る
        /// </summary>
        /// <param name="_json">json</param>
        /// <param name="_tag">タグ</param>
        /// <param name="_outData">出力先</param>
        void LoadVEC3Data(const nlohmann::json& _json, const std::string& _tag, VECTOR3::VEC3& _outData);

        /// <summary>
        /// jsonからVEC4型のデータを読み取る
        /// </summary>
        /// <param name="_json">json</param>
        /// <param name="_tag">タグ</param>
        /// <param name="_outData">出力先</param>
        void LoadVEC4Data(const nlohmann::json& _json, const std::string& _tag, VECTOR4::VEC4& _outData);
    };


    /// <summary>
    /// 接線・副接線を求める
    /// </summary>
    inline void CalcTangentAndBitangent(VERTEX::VERTEX_Static_N *vertices, UINT vtxNum, const WORD* indices, UINT idxNum)
    {
        using VECTOR2::VEC2;
        using VECTOR3::VEC3;

        for (UINT i = 0; i < vtxNum; i++) {
            vertices[i].tangent = VEC3(0, 0, 0);
            vertices[i].bitangent = VEC3(0, 0, 0);
        }

        // 三角形ごとに計算し、加算していく
        for (UINT i = 0; i < idxNum; i += 3)
        {
            WORD i0 = indices[i];
            WORD i1 = indices[i + 1];
            WORD i2 = indices[i + 2];

            VEC3 E1 = vertices[i1].pos - vertices[i0].pos;
            VEC3 E2 = vertices[i2].pos - vertices[i0].pos;
            VEC2 Delta_UV1 = vertices[i1].uv - vertices[i0].uv;
            VEC2 Delta_UV2 = vertices[i2].uv - vertices[i0].uv;
            float f = 1.0f / (Delta_UV1.x * Delta_UV2.y - Delta_UV2.x * Delta_UV1.y);

            VEC3 tan;
            VEC3 biTan;

            // 接線
            tan.x = f * (Delta_UV2.y * E1.x - Delta_UV1.y * E2.x);
            tan.y = f * (Delta_UV2.y * E1.y - Delta_UV1.y * E2.y);
            tan.z = f * (Delta_UV2.y * E1.z - Delta_UV1.y * E2.z);
            //tan = tan.Normalize();

            // 副接線
            biTan.x = f * (-Delta_UV2.x * E1.x + Delta_UV1.x * E2.x);
            biTan.y = f * (-Delta_UV2.x * E1.y + Delta_UV1.x * E2.y);
            biTan.z = f * (-Delta_UV2.x * E1.z + Delta_UV1.x * E2.z);
            //biTan = biTan.Normalize();

            vertices[i0].tangent += tan;
            vertices[i0].bitangent += biTan;            
            vertices[i1].tangent += tan;
            vertices[i1].bitangent += biTan;            
            vertices[i2].tangent += tan;
            vertices[i2].bitangent += biTan;
        }

        // 全頂点正規化
        for (UINT i = 0; i < vtxNum; i++)
        {
            vertices[i].tangent = vertices[i].tangent.Normalize();
            vertices[i].bitangent = vertices[i].bitangent.Normalize();
        }
    }
}



//--------------------------------------------------------------------------------------
// File: ReadData.h
//
// Helper for loading binary data files from disk
//
// For Windows desktop apps, it looks for files in the same folder as the running EXE if
// it can't find them in the CWD
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//-------------------------------------------------------------------------------------
 
// マイクロソフトのやつ https://learn.microsoft.com/ja-jp/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <system_error>
namespace DX
{
    inline std::vector<uint8_t> ReadData(_In_z_ const wchar_t *name)
    {
        std::ifstream inFile(name, std::ios::in | std::ios::binary | std::ios::ate);

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
        if (!inFile)
        {
            wchar_t moduleName[_MAX_PATH] = {};
            if (!GetModuleFileNameW(nullptr, moduleName, _MAX_PATH))
                throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()), "GetModuleFileNameW");

            wchar_t drive[_MAX_DRIVE];
            wchar_t path[_MAX_PATH];

            if (_wsplitpath_s(moduleName, drive, _MAX_DRIVE, path, _MAX_PATH, nullptr, 0, nullptr, 0))
                throw std::runtime_error("_wsplitpath_s");

            wchar_t filename[_MAX_PATH];
            if (_wmakepath_s(filename, _MAX_PATH, drive, path, name, nullptr))
                throw std::runtime_error("_wmakepath_s");

            inFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
        }
#endif

        if (!inFile)
            throw std::runtime_error("ReadData");

        const std::streampos len = inFile.tellg();
        if (!inFile)
            throw std::runtime_error("ReadData");

        std::vector<uint8_t> blob;
        blob.resize(size_t(len));

        inFile.seekg(0, std::ios::beg);
        if (!inFile)
            throw std::runtime_error("ReadData");

        inFile.read(reinterpret_cast<char *>(blob.data()), len);
        if (!inFile)
            throw std::runtime_error("ReadData");

        inFile.close();

        return blob;
    }
}











