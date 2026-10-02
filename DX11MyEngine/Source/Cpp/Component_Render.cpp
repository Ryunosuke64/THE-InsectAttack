#include "pch.h"
#include "Component_Render.h"


//*---------------------------------------------------------------------------------------
//* @:Render Class 
//*【?】コンストラクタ
//* 引数：1.オーナーオブジェクト
//* 引数：2.更新レイヤー
//*----------------------------------------------------------------------------------------
Render::Render(std::weak_ptr<GameObject> pOwner, int updateRank) : IComponent(pOwner, updateRank) 
,m_IsEnable(true)
{

};

