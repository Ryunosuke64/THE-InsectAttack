#include "pch.h"
#include "Helper.h"

//*---------------------------------------------------------------------------------------
//*y?zVEC3 Œ^‚Ì“Ç‚İæ‚è
//*
//* [ˆø”] 
//* &_json : json
//* &_tag : ƒ^ƒO
//* &_outData : o—Íæ 
//* [•Ô’l]
//* true : “Ç‚İ‚Æ‚è¬Œ÷
//* false : “Ç‚İ‚Æ‚è¸”s
//*----------------------------------------------------------------------------------------
void Tool::Json::LoadVEC3Data(const nlohmann::json& _json, const std::string& _tag, VECTOR3::VEC3& _outData)
{
    if (_json.contains(_tag) &&                                                      
        _json[_tag].is_array() &&
        _json[_tag].size() == 3)
    {
        _outData.x = _json[_tag][0].get<float>();
        _outData.y = _json[_tag][1].get<float>();
        _outData.z = _json[_tag][2].get<float>();
    }
}

//*---------------------------------------------------------------------------------------
//*y?zVEC4 Œ^‚Ì“Ç‚İæ‚è
//*
//* [ˆø”] 
//* &_json : json
//* &_tag : ƒ^ƒO
//* &_outData : o—Íæ 
//* [•Ô’l]
//* true : “Ç‚İ‚Æ‚è¬Œ÷
//* false : “Ç‚İ‚Æ‚è¸”s
//*----------------------------------------------------------------------------------------
void Tool::Json::LoadVEC4Data(const nlohmann::json& _json, const std::string& _tag, VECTOR4::VEC4& _outData)
{
    if (_json.contains(_tag) &&                                                      
        _json[_tag].is_array() &&
        _json[_tag].size() == 4)
    {
        _outData.x = _json[_tag][0].get<float>();
        _outData.y = _json[_tag][1].get<float>();
        _outData.z = _json[_tag][2].get<float>();
        _outData.w = _json[_tag][3].get<float>();
    }
}
