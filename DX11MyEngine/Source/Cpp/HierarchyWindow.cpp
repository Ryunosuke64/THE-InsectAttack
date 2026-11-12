#include "pch.h"
#include "HierarchyWindow.h"
#include "EditorBase.h"
#include "RendererEngine.h"
#include "GameObject.h"
#include <cctype>

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
HierarchyWindow::HierarchyWindow() : EditWindowBase()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
HierarchyWindow::~HierarchyWindow()
{
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool HierarchyWindow::Init(RendererEngine &renderer)
{
    m_EditorMap[""];

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void HierarchyWindow::Update(RendererEngine& renderer)
{
    using namespace Tool;

    const auto& opaqueObjects3D = Master::m_pGameObjectManager->get_3DOpaque_Objects();
    const auto& transparentObjects3D = Master::m_pGameObjectManager->get_3DTransparent_Objects();
    const auto& transparentObjects2D = Master::m_pGameObjectManager->get_2DTransparent_Objects();

    std::vector<std::shared_ptr<GameObject>> objects;
    objects.reserve(opaqueObjects3D.size() + transparentObjects3D.size() + transparentObjects2D.size());
    objects.insert(objects.end(), opaqueObjects3D.begin(), opaqueObjects3D.end());
    objects.insert(objects.end(), transparentObjects3D.begin(), transparentObjects3D.end());
    objects.insert(objects.end(), transparentObjects2D.begin(), transparentObjects2D.end());

    const std::string windowName = std::string(U8ToChar(u8"オブジェクトリスト")) + "###Hierarchy";
    if (!Master::m_pDebugger->BeginDebugWindow(windowName, 0))
    {
        Master::m_pDebugger->EndDebugWindow();
        return;
    }

    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"全体 : %zu"), objects.size());
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputTextWithHint("##NameSearch", U8ToChar(u8"名前で検索"), m_NameSearch, sizeof(m_NameSearch));

    // 管理リストは変更せず、表示する参照だけを毎フレーム現在の状態で振り分ける。
    std::vector<std::shared_ptr<GameObject>> activeObjects;
    std::vector<std::shared_ptr<GameObject>> inactiveObjects;
    size_t activeCount = 0;
    size_t inactiveCount = 0;
    const std::string nameSearch = m_NameSearch;
    for (const auto& obj : objects)
    {
        const bool isActive = obj->get_IsStatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        if (isActive)
        {
            ++activeCount;
        }
        else
        {
            ++inactiveCount;
        }

        const auto& tag = obj->get_Tag();
        if (!nameSearch.empty() && std::search(tag.begin(), tag.end(), nameSearch.begin(), nameSearch.end(),
            [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }) == tag.end())
        {
            continue;
        }

        (isActive ? activeObjects : inactiveObjects).push_back(obj);
    }

    const auto drawSection = [this, &nameSearch](const char* title, const char* id,
        const std::vector<std::shared_ptr<GameObject>>& sectionObjects, size_t totalCount, bool isActive)
    {
        std::string label = std::string(title) + " (" + std::to_string(sectionObjects.size());
        if (!nameSearch.empty())
        {
            label += " / " + std::to_string(totalCount);
        }
        label += ")###" + std::string(id);

        // 件数や検索結果が変わっても開閉状態を維持する。非アクティブは初期状態で閉じる。
        const bool isOpen = ImGui::CollapsingHeader(label.c_str(), isActive ? ImGuiTreeNodeFlags_DefaultOpen : 0);
        if (!isActive && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", Tool::U8ToChar(u8"非アクティブでも画面に表示される場合があります。"));
        }
        if (!isOpen)
        {
            return;
        }

        ImGui::PushID(id);
        if (!isActive)
        {
            // 操作は無効化せず、文字色だけで非アクティブを区別する。
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }
        if (sectionObjects.empty())
        {
            ImGui::TextDisabled("%s", Tool::U8ToChar(u8"該当するオブジェクトはありません。"));
        }
        else
        {
            DrawObjectGroups(sectionObjects);
        }
        if (!isActive)
        {
            ImGui::PopStyleColor();
        }
        ImGui::PopID();
    };

    drawSection(U8ToChar(u8"アクティブ"), "ActiveObjects", activeObjects, activeCount, true);
    drawSection(U8ToChar(u8"非アクティブ"), "InactiveObjects", inactiveObjects, inactiveCount, false);

    Master::m_pDebugger->EndDebugWindow();
}

// 両方の状態で、従来の同名タグのグループ化と単独オブジェクトの描画順を共用する。
void HierarchyWindow::DrawObjectGroups(const std::vector<std::shared_ptr<GameObject>>& objects)const
{
    std::map<std::string, std::vector<std::shared_ptr<GameObject>>> nameByObjectsMap;
    for (const auto& obj : objects)
    {
        nameByObjectsMap[obj->get_Tag()].push_back(obj);
    }
    std::vector<std::pair<std::string, std::vector<std::shared_ptr<GameObject>>>> nodeObjects(nameByObjectsMap.begin(), nameByObjectsMap.end());
    std::vector<std::shared_ptr<GameObject>> singleObjects;

    std::sort(nodeObjects.begin(), nodeObjects.end(), [](const auto& a, const auto& b)
        {
            return a.second.size() > b.second.size();
        });

    std::erase_if(nodeObjects,
        [&singleObjects](const auto& value) {
            if (value.second.size() == 1)
            {
                singleObjects.push_back(value.second.front());
                return true;
            }
            return false;
        }
    );

    std::sort(singleObjects.begin(), singleObjects.end(),
        [](const auto& a, const auto& b)
        {
            return a->get_LayerRank() < b->get_LayerRank();
        });

    for (auto& pair : nodeObjects)
    {
        if (ImGui::TreeNode(pair.first.c_str(), "%s (%zu)", pair.first.c_str(), pair.second.size()))
        {
            SelectObjectsDraw(pair.second);
            ImGui::TreePop();
        }
    }

    SelectObjectsDraw(singleObjects);
}

void HierarchyWindow::SelectObjectsDraw(const std::vector<std::shared_ptr<GameObject>>& objects)const
{
    const auto selectedObject = Master::m_pEditorManager->get_crntSelectedObject();
    for (const auto& obj : objects)
    {
        // 並び順に依存しないIDと既存の選択参照を使い、検索や状態の変化で選択を失わないようにする。
        ImGui::PushID(obj.get());
        const bool isSelected = selectedObject == obj;
        const std::string label = std::to_string(obj->get_LayerRank()) + " " + obj->get_Tag() + "###Object";
        if (Master::m_pDebugger->DG_Selectable(label, isSelected, ImGuiSelectableFlags_None, VECTOR2::VEC2(0, 20)))
        {
            Master::m_pEditorManager->set_crntSelectedObject(obj);
        }
        ImGui::PopID();
    }
}
