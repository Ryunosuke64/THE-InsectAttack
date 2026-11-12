#pragma once
#include "EditWindowBase.h"
class HierarchyWindow :   public EditWindowBase
{
private:
    std::unordered_map<std::string, std::unique_ptr<class EditorBase>> m_EditorMap;
    char m_NameSearch[128] = {};

public:
    HierarchyWindow();
    ~HierarchyWindow() override;
    bool Init(RendererEngine &renderer) override;
    void Update(RendererEngine &renderer)override;

private:
    void DrawObjectGroups(const std::vector<std::shared_ptr<GameObject>>& objects)const;
    void SelectObjectsDraw(const std::vector<std::shared_ptr<GameObject>>& objects)const;

};

