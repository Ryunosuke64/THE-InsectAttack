#pragma once

/* ウインドウの大きさ */
#define WND_RECT_TOP		0		// 上部
#define WND_RECT_BOTTOM		1080	// 下部
#define WND_RECT_RIGHT		1920	// 右側
#define WND_RECT_LEFT		0		// 左側
#define WND_CENTER_X			((WND_RECT_RIGHT - WND_RECT_LEFT) / 2)	// 中心X座標
#define WND_CENTER_Y			((WND_RECT_BOTTOM - WND_RECT_TOP) / 2)	// 中心Y座標

// ウインドウのタイトルバーのとこ
constexpr LPCWSTR g_WindowTitle = L"THE INSECT ATTACK";

// ウインドウクラス名 内部的な識別名？
constexpr LPCWSTR g_WindowClassNameW = L"DX11_3DGAME_SL";
constexpr LPCWSTR  g_WindowClassNameA = L"DX11_3DGAME_SL";

// FPS
constexpr float g_Fps = 60.0f;

/// <summary>
/// ウインドウモード
/// </summary>
enum class WINDOW_MODE
{
    WINDOW,
    BORDERLESS,
    FULLSCREEN
};