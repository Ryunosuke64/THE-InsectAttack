#pragma once
#include "IComponent.h"
#include "ConstantUIData.h"

class SpriteRenderer;

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Button Class --- */
//
//  ★継承：IComponent ★
//
// 【?】Unity風のボタン
//		とりあえず、見た目の変化は無しでクリック処理のみ
//		
// ***************************************************************************************
class ButtonUI : public IComponent
{
public:


private:
	std::array<VECTOR4::VEC4, UINT_CAST(UIData::STATE::NUM)> m_StateColor;	// それぞれのステートごとのカラ;
	VECTOR2::VEC2 m_TextOffsetPos;					// テキストの位置補正用
	UIData::STATE m_CrntState;						// 現在の状態
	UIData::STATE m_PrevState;						// 前の状態
	UIData::STATE m_InputValidationState;			// 入力判定とするステート
	std::function<void()> m_OnClick;				// クリックされた際の処理
	std::string m_Text;								// テキスト
	std::weak_ptr<RectTransform> m_pMyTransform;	// 自分のトランスフォーム
	std::weak_ptr<SpriteRenderer> m_pSprite;		// スプライト
	float m_FadeDuration;							// ステート間の色の遷移時間
	int m_InputSoundID;								// 入力されたときのサウンドID
	int m_InputWaitFrame;							// 入力後の待機時間カウンター
	int m_RepeatInputInterval;						// 入力の連続判定の間隔
	bool m_IsInteractable;							// 入力を受け付けるかどうか
	bool m_IsPressedInside;							// ボタン内でクリックが開始されたかを判定するフラグ
	bool m_AllowRepeatInput;						// 長押しによる連続入力を許可するか
	int m_CurrentRepeatTimer;						// 連続入力用の内部タイマー



	/* ナビゲーション用のリンク先 */
	ButtonUI* m_pNavUp;
	ButtonUI* m_pNavDown;
	ButtonUI* m_pNavLeft;
	ButtonUI* m_pNavRight;
	bool m_IsFocused = false;// 現在選択されているか

	//*****************************************************************************************
	//	UIManagerからのアクセスを許可
	//*****************************************************************************************
	friend class UIManager;

public:
	ButtonUI(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~ButtonUI();

	void Start(RendererEngine &renderer) override;		// 初期化
	void Update(RendererEngine &renderer) override;		// 更新処理
	void Draw(RendererEngine &renderer)override;		// 描画処理
	
	void ParamReset();	// パラメータリセット

	/// <summary>
	/// クリック処理を実行
	/// </summary>
	void InvokeClick();

	// ナビゲーションの紐づけ設定関数
	void set_Navigation(
		ButtonUI* _up,
		ButtonUI* _down,
		ButtonUI* _left,
		ButtonUI* _right);


	/// <summary>
	/// ボタン用のスプライト設定
	/// </summary>
	/// <param name="_pComp"></param>
	void set_Sprite(const std::shared_ptr<SpriteRenderer> &_pComp) { m_pSprite = _pComp; }

	/// <summary>
	/// 現在の状態を取得
	/// </summary>
	/// <returns></returns>
	UIData::STATE get_State()const { return m_CrntState; }

	/// <summary>
	/// クリックされた際の処理を登録
	/// </summary>
	/// <param name="_onClick">クリックされた際の処理が記述してある関数</param>
	/// <param name="_inputValidationState">処理を実行するタイミングのステート</param>
	void OnClickFunc(const std::function<void()> &_onClick, UIData::STATE _inputValidationState = UIData::STATE::PRESSED) {
		m_OnClick = _onClick; 
		m_InputValidationState= _inputValidationState;
	};

	/// <summary>
	/// 有効状態の設定
	/// </summary>
	/// <param name="_isIntertactable">フラグ</param>
	void set_IsInteractable(bool _isIntertactable) { m_IsInteractable = _isIntertactable; }

	/// <summary>
	/// 有効状態の取得
	/// </summary>
	/// <returns>有効フラグ</returns>
	bool get_IsInteractable()const { return m_IsInteractable; }

	/// <summary>
	/// ステート間の色の遷移時間設定
	/// </summary>
	/// <param name="_duration">遷移時間</param>
	void set_FadeDuration(float _duration) { m_FadeDuration = _duration; }

	/// <summary>
	/// ステート間の色の遷移時間取得
	/// </summary>
	/// <returns>遷移時間</returns>
	float get_FadeDuration()const { return m_FadeDuration; }

	/// <summary>
	/// ボタンに表示する文字を設定
	/// </summary>
	/// <param name="_text">表示する文字</param>
	void set_Text(const std::string &_text) { m_Text = _text; }

	/// <summary>
	/// ボタンに表示する文字の取得
	/// </summary>
	/// <returns>表示する文字</returns>
	std::string get_Text()const { return m_Text; }

	/// <summary>
	/// テキストの位置補正値の設定
	/// </summary>
	/// <param name="_v">補正値</param>
	void set_TextOffsetPos(const VECTOR2::VEC2 &_v) { m_TextOffsetPos = _v; }

	/// <summary>
	/// テキストの位置補正値の取得
	/// </summary>
	/// <returns>補正値</returns>
	VECTOR2::VEC2 get_TextOffsetPos()const { return m_TextOffsetPos; };

	/// <summary>
	/// 入力判定時のサウンドIDの設定
	/// </summary>
	/// <param name="_id">サウンドID</param>
	void set_InputSoundID(int _id) { m_InputSoundID = _id; }

	/// <summary>
	/// 入力判定時のサウンドIDの取得
	/// </summary>
	/// <returns>サウンドID</returns>
	int get_InputSoundID()const { return m_InputSoundID; }

	/// <summary>
	/// 入力の連続判定の間隔の設定
	/// </summary>
	/// <param name="_flag"></param>
	void set_RepeatInputInterval(int _interval) { m_RepeatInputInterval = _interval; }

	/// <summary>
	/// 入力の連続判定の間隔の取得
	/// </summary>
	/// <returns></returns>
	int get_RepeatInputInterval()const { return m_RepeatInputInterval; }


	/// <summary>
	/// 入力時、何フレーム待ってから次の入力を受け付けるかのカウンターの設定
	/// </summary>
	/// <param name="_flag"></param>
	void set_InputWaitFrame(int _counter) { m_InputWaitFrame = _counter; }

	/// <summary>
	/// 入力時、何フレーム待ってから次の入力を受け付けるかのカウンターの取得
	/// </summary>
	/// <returns></returns>
	int get_InputWaitCounter()const { return m_InputWaitFrame; }

	/// <summary>
	/// 状態ごとのカラー値を設定
	/// </summary>
	/// <param name="_color">カラー</param>
	/// <param name="_state">設定する状態</param>
	void set_Color(const VECTOR4::VEC4& _color, UIData::STATE _state);

	void set_IsFocused(bool _focused) { m_IsFocused = _focused; }
	bool get_IsFocused() const { return m_IsFocused; }

	// 連続入力を許可する設定用セッター
	void set_AllowRepeatInput(bool allow) { m_AllowRepeatInput = allow; }
};

