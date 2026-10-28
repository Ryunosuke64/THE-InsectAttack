#pragma once

//--------------------------------------------------------------------------------------
//      * マクロ定義 *
//--------------------------------------------------------------------------------------
#define PAD_MAX 16      //パッドキーの最大数
#define KEY_MAX 256     // キーの種類

/// <summary>
/// 入力タイプ
/// </summary>
enum class INPUT_FLAG_TYPE
{
    DOWN,       // 押された瞬間
    UP,         // 離された瞬間
    HOLD,       // 長押し
    HOLD_REPEAT // 長押し（入力判定になるまでのフレーム指定用）
};


enum class INPUT_TYPE
{
    KEYBORAD_AND_MOUSE,
    GAMEPAD,
};

//--------------------------------------------------------------------------------------
//      * 列挙 ENUM *
//--------------------------------------------------------------------------------------
/* コンフィグのボタン定義 */
enum class CONFIG_INPUT {
    LEFT,
    RIGHT,
    UP,
    DOWN,
    MOVE_F,
    MOVE_B,
    MOVE_L,
    MOVE_R,
    JUMP,
    C,
    PAUSE,

    MOVE_FORWARD,

    MAX,
};

/// <summary>
/// ゲームプレイでのコンフィグ設定
/// </summary>
enum class GAME_CONFIG
{
    // 移動
    MOVE_FORWARD,   // 前進
    MOVE_BACK,      // 後退
    MOVE_LEFT,      // 左
    MOVE_RIGHT,     // 右
    MOVE_JUMP,      // ジャンプ
    MOVE_DASH,      // ダッシュ
    MOVE_ROLLING,   // ローリング

    SWITCH_CONTROLL_TARGET, // 操作対象切り替え
    DURGA_ROT_RIGHT,        // ドルガ - 右回転
    DURGA_ROT_LEFT,         // ドルガ - 左回転
    DURGA_LIFT,             // ドルガ - 持ち上げる
    DURGA_THROW,            // ドルガ - 投げる

    // 視点移動
    VIEW_UP,        
    VIEW_DOWN,
    VIEW_LEFT,
    VIEW_RIGHT,

    // 武器
    WEAPON_FIRE,    // 発射
    WEAPON_RELOAD,  // リロード
    WEAPON_CHANGE1, // 武器切り替え１
    WEAPON_CHANGE2, // 武器切り替え２
    WEAPON_ZOOM,    // ズーム

    
    DECITION, // 決定
    PAUSE,    // ポーズ画面
    EXIT,     // 終了

    /* デバッグキー */
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    NUM,
};

enum class SYSTEM_CONFIG
{

};

/// <summary>
/// キー
/// </summary>
enum class KEY_STATE
{
    // アルファベット
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // 数字
    D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,

    // 特殊キー
    ESCAPE,
    SPACE,
    ENTER,
    LSHIFT,
    RSHIFT,
    LCTRL,
    RCTRL,
    ALT,
    TAB,

    // 矢印
    UP,
    DOWN,
    LEFT,
    RIGHT,

    MAX_KEYS // 配列サイズ確保用
};

/// <summary>
/// マウスのボタン
/// </summary>
enum class MOUSE_BUTTON_STATE
{
    NONE = -1,

    LEFT,   // 左クリック
    RIGHT,  // 右クリック
    CENTER, // ホイールクリック

    // その他
    OTHER01,

    // ホイール
    WHEEL_UP,   
    WHEEL_DOWN,

    NUM,
};

/// <summary>
/// ゲームパッド
/// </summary>
enum class GAMEPAD_STATE
{
    NONE = -1,
    NUM,
};
