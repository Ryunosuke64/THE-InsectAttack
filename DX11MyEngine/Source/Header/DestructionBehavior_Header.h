#pragma once


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:IDestructionBehavior Class --- */
//
//  ššš’ŠÛƒNƒ‰ƒXššš
//
// y?z”j‰ó‚Ì‹““®‚ğ’è‹`‚·‚éƒNƒ‰ƒX
//
// ***************************************************************************************
class IDestructionBehavior
{
private:
    
public:
    IDestructionBehavior() = default;
    virtual ~IDestructionBehavior() = default;

    virtual void Start(
        GameObject* object,
        const VECTOR3::VEC3& hitPos,
        const VECTOR3::VEC3& hitDir) = 0;

    virtual void Update(float deltaTime) {}

    virtual bool IsFinished() const = 0;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PropDestructionBehavior Class --- */
//
//  šŒp³FIDestructionBehavior š
//
// y?z¬•¨—Ş‚Ì”j‰ó‚Ì‹““®‚ğ’è‹`
//
// ***************************************************************************************
class PropDestructionBehavior : public IDestructionBehavior
{
private:
    enum class STATE
    {
        ALIVE,		    // ¶‘¶’†
        DESTROYED,		// •ö‰ó’†
        UNDERWORLD,	    // — ¢ŠE‚Ö
    };
    const float TO_UNDERWORLD_TIME = 10.0f;     // — ¢ŠE‚Ö—‚Æ‚·‚Ü‚Å‚ÌŠÔ
    const float DESTROY_TIME = 15.0f;           // íœ‚Ü‚Å‚ÌŠÔ
    const float IMPULSE_POWER = 30.0f;          // ‚Á”ò‚Ñ—Í
    bool m_IsFinished = false;
    float m_ElapsedTime = 0.0f;		// Œo‰ßŠÔ
    //float m_LifeTime = 0.0f;		    // ¶‘¶ŠÔ
    STATE m_CrntState = STATE::ALIVE;
    std::weak_ptr<class RigidBody> m_pRigidBody;

public:
    void Start(
        GameObject* object,
        const VECTOR3::VEC3& hitPos,
        const VECTOR3::VEC3& hitDir)override;
    
    void Update(float deltaTime)override;
    
    bool IsFinished() const override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ReplaceModelDestructionBehavior Class --- */
//
//  šŒp³FIDestructionBehavior š
//
// y?z”j‰óƒ‚ƒfƒ‹‚Ö‚Ì·‚µ‘Ö‚¦
//      å‚ÉŒš•¨‚Ég—p‚·‚é
//
// ***************************************************************************************
class ReplaceModelDestructionBehavior : public IDestructionBehavior
{
private:
    //const float DESTROY_TIME = 20.0f;   // íœ‚Ü‚Å‚ÌŠÔ
    const float PLAY_COLLAPSING_SOUND_TIME = 4.0f;   // •ö‰óƒTƒEƒ“ƒhÄ¶‚Ü‚Å‚ÌŠÔ
    const float IMPULSE_POWER = 50.0f;  // ‚Á”ò‚Ñ—Í
    bool m_IsFinished = false;
    bool m_IsDestructionStart = false;
    float m_ElapsedTime = 0.0f;
public:
    void Start(
        GameObject* object,
        const VECTOR3::VEC3& hitPos,
        const VECTOR3::VEC3& hitDir)override;
    
    void Update(float deltaTime)override;
    
    bool IsFinished() const override;
};