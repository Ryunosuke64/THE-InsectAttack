#pragma once
#include "VEC.h"

class SpriteRenderer;
class ButtonUI;
class RectTransform; 

namespace UIData
{
    /* ボタン用 */
    enum class STATE { NORMAL, HIGH_LIGHTED, PRESSED, SELECTED, DISABLED, NUM };	// 通常/ハイライト/押されている/選択された/無効中
    enum class TRANSITION { COLOR, SPRITE };										// 遷移オプション 

    /// <summary>
    /// UIトランスフォームに設定するデータ
    /// </summary>
    struct RectTransformData 
    {
        VECTOR2::VEC2 _pos;         // 位置
        VECTOR2::VEC2 _anchorMin;   // 親からの相対的な最小基準点
        VECTOR2::VEC2 _anchorMax;   // 親からの相対的な最大基準点
        VECTOR2::VEC2 _pivot;       // 自身の基準点
        VECTOR2::VEC2 _size;        // 幅
        float _rotZ = 0.0f;         // 基本的にZ回転のみ（デグリー）

        /// <summary>
        /// UIトランスフォームにデータを一括セットする
        /// </summary>
        /// <param name="_out">パラメータを設定するUIトランスフォーム</param>
        /// <param name="_param">パラメータ</param>
        static void SetRectTransformData(RectTransform &_out, const RectTransformData &_param);
    };




    /// <summary>
    /// UIの種類
    /// </summary>
    enum class UI_TYPE
    {
        SPRITE,     // 単一画像
        BUTTON,     // ボタン

        NUM
    };

    /// <summary>
    /// UI生成構造体の基底
    /// </summary>
    struct UIBase 
    {
		int _layerRank = 0;  // 描画順を決めるためのランク（低い順に描画される）
		std::string _tag = "UI_Unknown";    // タグ

        virtual ~UIBase() = default;

        /// <summary>
        /// リセット
        /// </summary>
        virtual void Reset()
        {
            *this = UIBase();
        }
    };

    /// <summary>
    /// スプライト画像の生成データ
    /// </summary>
    struct SpriteUIData : UIBase
    {
        std::string _imagePath;         // 使用する画像のパス（空なら、テクスチャは設定せずにカラーのみ設定する）
        VECTOR4::VEC4 _color = VECTOR4::VEC4(1.0f);	// 頂点カラー
        VECTOR2::VEC2 _UVOffset = VECTOR2::VEC2();	// uvオフセット
		SHADER_TYPE _shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE;		// シェーダーの種類


        /// <summary>
        /// リセット
        /// </summary>
        void Reset()override
        {
            *this = SpriteUIData();
        }

        /// <summary>
        /// スプライト情報の設定
        /// </summary>
        /// <param name="_out">パラメータを設定するスプライト</param>
        /// <param name="_param">パラメータ</param>
        static void SetSpriteData(SpriteRenderer &_out, const SpriteUIData& _param);
    };

    /// <summary>
    /// ボタンの生成データ
    /// スプライトも使用するのでSpriteUIDataを継承
    /// </summary>
    struct ButtonUIData : SpriteUIData
    {
        std::function<void()> _onClicFunction;                            // クリック時の処理
        std::string _text;                                                // 表示する文字
        std::shared_ptr<SpriteRenderer> _pSprite;                         // 使用する画像
        VECTOR2::VEC2 _textOffsetPos = VECTOR2::VEC2();                   // 文字の位置補正
        STATE _inputValidationState = STATE::PRESSED;	                  // 入力判定とするステート（functionの実行タイミング）
        int _inputSoundID = SOUND_ID_TO_INT(SOUND_ID::SYSTEM_DECISION02); // 入力されたときのサウンドID
		int _repeatInputInterval = 1;                                     // 入力のリピート間隔
		int _inputWaitFrame = 0;                                           // 入力受付開始までの待ち時間（フレーム数）

        /// <summary>
        /// リセット
        /// </summary>
        void Reset()override
        {
            *this = ButtonUIData();
        }

        /// <summary>
        /// ボタン情報の設定
        /// </summary>
        /// <param name="_out">パラメータを設定するボタン</param>
        /// <param name="_param">パラメータ</param>
        static void SetButtonData(ButtonUI &_out, const ButtonUIData& _param);
    };
};