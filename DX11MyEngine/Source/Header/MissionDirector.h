#pragma once

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BuildingManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】ミッションの管理
//
// ***************************************************************************************
class MissionDirector
{
private:
    std::vector<UtilityData::MissionData> m_MissionDataList;    // ミッションデータの保持
    UtilityData::StageEnvironmentParam m_EnvironmentParam;      // ステージ環境パラメータ
    const std::string ModuleName = "MissionModule";             // モジュール名

public:
    MissionDirector();
    ~MissionDirector();

    bool LoadMissionData(const std::string& _filePath, UtilityData::MissionData& _outData);                     // ミッションデータの読み込み   
    bool LoadMissionScript(const std::string& _filePath);                                                       // ミッションスクリプトの読み込み
    void SetStageEnvironmentParam(const UtilityData::StageEnvironmentParam& param);                             // ステージ環境の設定
    const UtilityData::StageEnvironmentParam& GetStageEnvironmentParam()const { return m_EnvironmentParam; }    // ステージ環境の取得
    void ExcuteScriptMissionSetup();                                                                            // ミッションスクリプトのセットアップ関数実行
    void ExcuteScriptMissionMain();                                                                             // ミッションスクリプトのメイン関数実行

private:
    // コピー禁止
    MissionDirector(const MissionDirector&) = delete;
    MissionDirector& operator=(const MissionDirector&) = delete;
};

