#pragma once

//
/* プリコンパイル済みヘッダ */
//

/* マクロ定義 */
#define SAFE_RELEASE(x)		  {if(x) { (x)->Release(); (x)=nullptr; } }	// 解放
#define SAFE_DELETE(p)		  {if(p) { delete p; p = nullptr; } }	// メモリの開放
#define SAFE_DELETE_ARRAY(p)  {if(p) { delete[] p; p = nullptr; } }	// メモリの解放(配列)
#define CHECK_HRESULT(hr)       do { if (FAILED(hr)) { assert(false); return hr; } } while(0)    // HRESULTチェック do while(0)は複数行マクロの定石らしい？
#define CHECK_HRESULT_NO_BOOL(hr)       do { if (FAILED(hr)) { assert(false); return; } } while(0)    // HRESULTチェック do while(0)は複数行マクロの定石らしい？

#define INT_CAST(x)		(static_cast<int>(x))		// intにキャスト
#define UINT_CAST(x)	(static_cast<unsigned>(x))	// unsignedにキャスト
#define FLOAT_CAST(x)	(static_cast<float>(x))		// floatにキャスト
#define DOUBLE_CAST(x)	(static_cast<double>(x))	// doubleにキャスト

#define DEBUG_MODE false	// 内部的にデバッグモードにするかどうか

#define NOMINMAX    // min STLとwindows.hのマクロがかぶっているのを回避する


// えふぇくしあ
#ifdef _DEBUG
#pragma comment(lib, "Effekseerd.lib")
#pragma comment(lib, "EffekseerRendererDX11d.lib")

#else
#pragma comment(lib, "Effekseer.lib")
#pragma comment(lib, "EffekseerRendererDX11.lib")
#endif


/* 組み込みヘッダ */
#include <d3d11.h>          // directX11
#include <d3d12.h>          // directX12
#include <DirectXMath.h>    // 数学系
#include <DirectXTex.h>     // テクスチャ読み込み
#include <DirectXCollision.h> // コリジョン関係（BoundingBoxやFrusumなど）
#pragma comment(lib, "d3d11.lib")			  // directX11ライブラリのリンク


// COMポインタ用
#include <wrl/client.h>



// 文字表示用
#include <d2d1.h>   // Direct2D用
#include <dwrite.h> // DirectWrite用
#pragma comment(lib,"d2d1.lib")     // Direct2D用
#pragma comment(lib,"dwrite.lib")   // DirectWrite用

//!**************************************************************
//! Texを内部でリンクさせようとすると正体不明のエラーが出るため、
//! プロパティ側で対応
//!**************************************************************
// Texライブラリのリンク
#ifdef _DEBUG
#pragma comment(lib, "DirectXTexd.lib")
#else
#pragma comment(lib, "DirectXTex.lib")
#endif

// シェーダー
#include <d3dcompiler.h>				// シェーダーをコンパイルするためのやつ
#pragma comment(lib, "d3dcompiler.lib") // シェーダコンパイル用の静的ライブラリをリンク

// 入力用
#include <dinput.h>         
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dInput8.lib")

// json読み込み用
#include <nlohmann/json.hpp>

// 標準ヘッダ
#include <unordered_map>    // 連想配列用
#include <typeindex>		// 型をキーに出来るようにするもの
#include <map>      // 連想配列用
#include <memory>   // スマートポインタ用
#include <string>   // 文字列
#include <vector>   // 動的配列
#include <deque>    // 動的配列（両端の要素の追加削除が得意らしい）
#include <algorithm>
#include <array>
#include <span>
#include <variant>	// 共用体
#include <random>	// 乱数生成用
#include <sstream>
#include <iomanip>
#include <unordered_set>
/* 定数 */
constexpr int POINTLIGHT_MAX_NUM		= 100; // ポイントライトの最大数
constexpr int SPOTLIGHT_MAX_NUM			= 20;  // スポットライトの最大数
constexpr int DIRECTIONLIGHT_MAX_NUM	= 1;   // ディレクションライトの最大数


#define IS_WRITE_ENABLE   // DirectWriteを使用しているとRenderDocが使用できないのでそういう時にこれを消す


/* 自作ヘッダ */
#include "ConstantRenderData.h"		// 描画情報
#include "VERTEX.h"					// 頂点情報
#include "SHADER.h"					// シェーダ情報
#include "Window.h"					// ウインドウ情報
#include "Helper.h"					// ヘルパー
#include "IDXResource.h"			// リソース
#include "ConstantBuffer.h"			// 定数バッファ用構造体
#include "ConstantBufferStructs.h"	// 定数バッファ用構造体
#include "Material.h"				// マテリアル
#include "BitFlag.h"				// ビットフラグ関連
#include "Master.h"					// シングルトン
#include "StateMachine.h"			// ステートマシン
#include "EnemyStateFactory.h"	    // ステートの作成
#include "InputFactory.h"           // 入力処理
#include "RenderCommand.h"			// 描画コマンド

//#include "ShaderManager.h"	// シェーダ管理
//#include "Debugger.h"	    // Imguiデバッグ用
//#include "BlendManager.h"   // ブレンドステート用


/* クラス前方宣言 */
class RendererEngine;
class Camera;
class SceneManager;
class GameManager;


