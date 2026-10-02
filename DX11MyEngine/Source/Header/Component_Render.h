#pragma once
#include "IComponent.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Render Class --- */
//
//  šŒp³FIComponent š
//
// y?z•`‰æ‚ğs‚¤ƒRƒ“ƒ|[ƒlƒ“ƒg‚ÌŠî’êƒNƒ‰ƒX
//		
// ***************************************************************************************
class Render : public IComponent
{
private:
	bool m_IsEnable = true;	// •`‰æ‚·‚é‚©‚Ç‚¤‚©

public:
	Render(std::weak_ptr<GameObject> pOwner, int updateRank);
	virtual ~Render() = default;

	bool get_IsEnable()const { return m_IsEnable; }	// •`‰æ‚·‚é‚©‚Ç‚¤‚©‚Ìæ“¾
	void set_IsEnable(bool _flag) { m_IsEnable = _flag; }	// •`‰æ‚·‚é‚©‚Ç‚¤‚©‚Ìİ’è
};

