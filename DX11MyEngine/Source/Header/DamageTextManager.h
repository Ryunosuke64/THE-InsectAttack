#pragma once

struct DamageText
{
    VECTOR2::VEC2 screenPos;                    // 実際に描画に使うスクリーン座標
    VECTOR3::VEC3 hitPoint = VECTOR3::VEC3();   // ヒット位置
    float damageValue = 0.0f;                   // ダメージ量
	float elapsedTime = 0.0f;                   // 経過時間
	float addOffsetY = 0.0f;                    // Y軸方向のオフセット
    bool isActive = false;

    void Reset()
    {
        hitPoint = VECTOR3::VEC3();
        screenPos = VECTOR2::VEC2();
        damageValue = 0.0f;           
        elapsedTime = 0.0f;          
        addOffsetY = 0.0f;
        isActive = false;
    }
};

/// <summary>
/// ダメージテキストの表示位置
/// </summary>
enum class DAMAGE_TEXT_POSITION_TYPE
{
    HIT,    // ヒット位置
	FIXED   // 固定位置
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:DamageTextManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】ダメージテキストの管理
//
// ***************************************************************************************
class DamageTextManager
{
private:
	const float DAMAGE_TEXT_LIFETIME = 1.0f; // ダメージテキストの表示時間
	std::vector<DamageText> m_DamageTexts; // ダメージテキストの配列
    DAMAGE_TEXT_POSITION_TYPE m_PositionType;
public:
    DamageTextManager();
    ~DamageTextManager();

    bool Init();
    void Update(float deltaTime);
    void Draw();
    void Register(
        const VECTOR3::VEC3& position,
        float damage,
        DAMAGE_TEXT_POSITION_TYPE positionType = DAMAGE_TEXT_POSITION_TYPE::HIT);
private:
    // コピー禁止
    DamageTextManager(const DamageTextManager&) = delete;
    DamageTextManager& operator=(const DamageTextManager&) = delete;

	VECTOR2::VEC2 ConvertTextPosition_Hit(const VECTOR3::VEC3& position);
    VECTOR2::VEC2 ConvertTextPosition_Fixed(const VECTOR3::VEC3& position);
};

