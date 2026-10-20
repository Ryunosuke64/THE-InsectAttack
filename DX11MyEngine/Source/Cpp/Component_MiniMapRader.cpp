#include "pch.h"
#include "Component_MiniMapRader.h"
#include "RendererEngine.h"
#include "GameObject.h"
#include "Component_3DCamera.h"

using namespace UtilityData;
using namespace Tool;
using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;


//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
MiniMapRader::MiniMapRader(std::weak_ptr<GameObject> pOwner, int updateRank)
	:IComponent(pOwner, updateRank),
    m_pRaderBackSpriteObj(nullptr),
    m_MaxEnemyNum(0),
    m_CrntEnemyNum(0)

{
	this->set_Tag("MiniMapRader");
}


//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MiniMapRader::~MiniMapRader()
{
    // ---------------------------------------
    // プールへ返す
    // ---------------------------------------
    if (m_pRaderBackSpriteObj != nullptr) {
        m_pRaderBackSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }
    for (int i = 0; i < m_pRaderEnemySpriteObjArray.size(); i++) {
        if (m_pRaderEnemySpriteObjArray[i] != nullptr) {
            m_pRaderEnemySpriteObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        }
    }
    for (int i = 0; i < m_pRaderItemSpriteObjArray.size(); i++) {
        if (m_pRaderItemSpriteObjArray[i] != nullptr) {
            m_pRaderItemSpriteObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        }
    }
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void MiniMapRader::Start(RendererEngine& renderer)
{
    m_pPlayerObj = Master::m_pGameObjectManager->get_ObjectByTag("Player");

    UIData::RectTransformData rectData;
	UIData::SpriteUIData spriteData;

    // レーダー背景用スプライト **********************************************
    rectData._size = VEC2(MINIMAP_SIZE, MINIMAP_SIZE);
    rectData._pos = VEC2(MINIMAP_X, MINIMAP_Y);
    rectData._anchorMax = VEC2(0.0f, 0.0f);
    rectData._anchorMin = VEC2(0.0f, 0.0f);
    rectData._pivot = VEC2(0.5f, 0.5f);         // ピボットを中心に設定
    spriteData._tag = "Radar_Back";
    spriteData._imagePath = "Resource/Texture/UI/Radar.png";
    spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE;
    spriteData._layerRank = 100;
    m_pRaderBackSpriteObj = Master::m_pUIManager->GetSprite(renderer, rectData, spriteData);

}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void MiniMapRader::Update(RendererEngine& renderer)
{
    //auto antList = Master::m_pGameObjectManager->get_ObjectsByTag("Ant");
    auto enemys = Master::m_pGameObjectManager->get_ObjectListByFactionAlive(FACTION::ENEMY);    // 生存状態の敵取得
    auto items = Master::m_pGameObjectManager->get_ObjectListByFaction(FACTION::ITEM);         // アイテム取得

    size_t enemySpriteSize = m_pRaderEnemySpriteObjArray.size();
    size_t itemSpriteSize = m_pRaderItemSpriteObjArray.size();

    // 敵のアイコンスプライト確保
    if (enemySpriteSize < enemys.size()) {
        SecuringMiniMapIconSprite(renderer, INT_CAST(enemys.size()), m_pRaderEnemySpriteObjArray, VEC2(MINIMAP_ENEMY_ICON_SIZE), VEC4(1.0f, 0.0f, 0.0f, 1.0f), ENEMY_ICON_LAYER);
    }
    // 少なくなっていたら、プールへ返す
    else if (enemySpriteSize > enemys.size()) {
        m_pRaderEnemySpriteObjArray.back()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        m_pRaderEnemySpriteObjArray.pop_back(); // 除外
    }

    // アイテムのアイコンスプライト確保
    if (itemSpriteSize < items.size()) {
        SecuringMiniMapIconSprite(renderer, INT_CAST(items.size()), m_pRaderItemSpriteObjArray, VEC2(MINIMAP_ENEMY_ICON_SIZE), VEC4(0.0f, 1.0f, 0.0f, 1.0f), ITEM_ICON_LAYER);
    }
    // 少なくなっていたら、プールへ返す
    else if (itemSpriteSize > items.size()) {
        m_pRaderItemSpriteObjArray.back()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        m_pRaderItemSpriteObjArray.pop_back(); // 除外
    }

    std::shared_ptr<Camera3D> camera;
    if (camera = Master::m_pDataManager->get_CameraComponent().lock())
    {
        VEC3 playerPos = m_pPlayerObj.lock()->get_Transform().lock()->get_VEC3ToPos();
        float cameraAngleH = camera->get_Angle_H();

        UpdateRadarIcons(items, m_pRaderItemSpriteObjArray, playerPos, cameraAngleH);   // アイテム
        UpdateRadarIcons(enemys, m_pRaderEnemySpriteObjArray, playerPos, cameraAngleH); // 敵
    }
}




//*---------------------------------------------------------------------------------------
//*【?】レーダー上のアイコンを更新する
//*
//* [引数]
//* &targetObjs : 表示対象
//* &iconSprites : 表示するUI
//* &centerPos : 中心点（プレイヤー等）
//* cameraAngleH : 基準となる角度
//* [返値]なし
//*----------------------------------------------------------------------------------------
void MiniMapRader::UpdateRadarIcons(
    const std::vector<std::shared_ptr<GameObject>>& targetObjs, 
    const std::vector<GameObject*>& iconSprites,
    const VEC3& centerPos,
    float cameraAngleH)
{
    // カメラの水平回転角度に基づいて、サインとコサインを計算（ずれてしまうので1.57で補正）
    float cameraH_s = sinf(-cameraAngleH - G_PI_2_F);
    float cameraH_c = cosf(-cameraAngleH - G_PI_2_F);

    // レーダーの敵の位置更新
    for (int i = 0; i < targetObjs.size(); i++)
    {
        auto& target = targetObjs[i];
        VEC3 targetPos = target->get_Transform().lock()->get_VEC3ToPos();

        // 相対座標の計算（中心位置を原点（0,0）にする）
        float relativeX = targetPos.x - centerPos.x;
        float relativeZ = targetPos.z - centerPos.z;

        // 回転とスケールの適用を一度に行う
        float rotX = (relativeX * cameraH_c - relativeZ * cameraH_s);
        float rotY = (relativeX * cameraH_s + relativeZ * cameraH_c);

        // ミニマップの位置にする
        int targetMapX = static_cast<int>(MINIMAP_X + rotX);
        int targetMapY = static_cast<int>(MINIMAP_Y - rotY);    // 前方の敵をミニマップの上の方に置きたいので、Y反転
        
        // 位置をミニマップの円内に収める
        ClampToMinimap(targetMapX, targetMapY, static_cast<int>(MINIMAP_X), static_cast<int>(MINIMAP_Y), MINIMAP_RADIUS);

        iconSprites[i]->get_RectTransform().lock()->set_RectPosition(VEC2(FLOAT_CAST(targetMapX), FLOAT_CAST(targetMapY)));
    }
}


//*---------------------------------------------------------------------------------------
//*【?】位置をミニマップ内の端に制限する
//*
//* [引数]
//* &_x : 制限したいX座標の参照 
//* &_y : 制限したいY座標の参照
//* _centerX : ミニマップの中心X座標
//* _centerY : ミニマップの中心Y座標
//* _radius : ミニマップの半径
//* [返値]なし
//*----------------------------------------------------------------------------------------
void MiniMapRader::ClampToMinimap(int& _Out_X, int& _Out_Y, int _centerX, int _centerY, float _radius)
{
    float dx = static_cast<float>(_Out_X - _centerX);
    float dy = static_cast<float>(_Out_Y - _centerY);

    float dist = sqrtf(dx * dx + dy * dy);

    if (dist > _radius)
    {
        float scale = _radius / dist;
        _Out_X = _centerX + static_cast<int>(dx * scale);
        _Out_Y = _centerY + static_cast<int>(dy * scale);
    }
}


//*---------------------------------------------------------------------------------------
//*【?】ミニマップに表示する円のスプライトを確保
//*
//* [引数] 
//* &renderer : 描画エンジンの参照
//* _num : 確保する数
//* &_size : サイズ
//* &_color : 色
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MiniMapRader::SecuringMiniMapIconSprite(RendererEngine& renderer, int _num, std::vector<class GameObject*>& iconSprites, const VECTOR2::VEC2& _size, const VECTOR4::VEC4& _color, int _layer)
{
    UIData::RectTransformData rectData;
    UIData::SpriteUIData spriteData;
    // レーダーに映す点用スプライト **********************************************
    rectData._size = _size;
    rectData._pos = VEC2(0.0f, 0.0f);
    rectData._anchorMax = VEC2(0.0f, 0.0f);
    rectData._anchorMin = VEC2(0.0f, 0.0f);
    rectData._pivot = VEC2(0.5f, 0.5f);         // ピボットを中心に設定
    spriteData._tag = "Icon";
    spriteData._imagePath = "Resource/Texture/UI/Circle.png";
    spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE;
    spriteData._layerRank = _layer;
    spriteData._color = _color;
    for (int i = 0; i < _num; i++)
    {
        iconSprites.push_back(Master::m_pUIManager->GetSprite(renderer, rectData, spriteData));
    }
}
