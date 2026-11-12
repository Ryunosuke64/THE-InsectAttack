#pragma once

//=========================================================================================
//
//						AngelScript内で使用するオーディオAPI群
//
//=========================================================================================
namespace GIGA_Engine::ScriptAPI::Audio
{
	void PlayBGM(const std::string& soundKey);
	void PlaySound3D(const std::string& soundKey, const VECTOR3::VEC3& pos, float radius);
};