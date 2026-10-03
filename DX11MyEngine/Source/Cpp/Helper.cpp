#include "pch.h"
#include "RendererEngine.h"
#include "Helper.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;

//*---------------------------------------------------------------------------------------
//*【?】VEC3 型の読み取り
//*
//* [引数] 
//* &_json : json
//* &_tag : タグ
//* &_outData : 出力先 
//* [返値]
//* true : 読みとり成功
//* false : 読みとり失敗
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
//*【?】VEC4 型の読み取り
//*
//* [引数] 
//* &_json : json
//* &_tag : タグ
//* &_outData : 出力先 
//* [返値]
//* true : 読みとり成功
//* false : 読みとり失敗
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

//*---------------------------------------------------------------------------------------
//*【?】3D空間の位置をスクリーン座標に変換する
//*
//* [引数] 
//* &position : ワールド座標
//* [返値]
//* スクリーン座標 
//*----------------------------------------------------------------------------------------
VECTOR2::VEC2 Tool::ConvertWorldToScreen(const VECTOR3::VEC3& position)
{
    float screenWidth = Master::m_pDataManager->get_ScreenWidth();
    float screenHeight = Master::m_pDataManager->get_ScreenHeight();
    const RendererEngine* renderer = Master::m_pDataManager->get_RendererEngine();

    DirectX::XMMATRIX view = renderer->get_ViewMatrix();
    DirectX::XMMATRIX proj = renderer->get_ProjectionMatrix();

	// ビュー行列と射影行列を掛け合わせて、ワールド座標からクリップ空間への変換行列を作成
    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
    DirectX::XMVECTOR vWorld = DirectX::XMVectorSet(position.x, position.y, position.z, 1.0f);

	// ワールド座標をクリップ空間に変換
    VEC4 clip = VEC4::FromXMVECTOR(DirectX::XMVector4Transform(vWorld, viewProj));

	// クリップ空間からNDCに変換
    VEC3 ndc;
    ndc.x = clip.x / clip.w;
    ndc.y = clip.y / clip.w;
    ndc.z = clip.z / clip.w;

	// NDCからスクリーン座標に変換
    float screenX = (ndc.x + 1.0f) * (screenWidth * 0.5f);
    float screenY = (1.0f - ndc.y) * (screenHeight * 0.5f); // Y軸は下方向が正になるよう反転

    return VEC2(screenX, screenY);
}
