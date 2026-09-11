#include "pch.h"
#include "MissionDirector.h"
#include "RendererEngine.h"
#include "Component_DirectionalLight.h"

using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
MissionDirector::MissionDirector()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MissionDirector::~MissionDirector()
{
}

//*---------------------------------------------------------------------------------------
//*【?】ミッションデータの読み込み（json）
//*
//* [引数] 
//* _filePath : jsonのファイルパス
//* &_outData : 出力先 
//* [返値]
//* true  : 読みとり成功
//* false : 読みとり失敗
//*----------------------------------------------------------------------------------------
bool MissionDirector::LoadMissionData(const std::string& _filePath, UtilityData::MissionData& _outData)
{
    using json = nlohmann::json;
    std::ifstream ifs(_filePath);
    if (!ifs.is_open()) return false;

    json j;
    ifs >> j;
}

//*---------------------------------------------------------------------------------------
//*【?】ミッションスクリプトのセットアップ関数実行
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MissionDirector::ExcuteScriptMissionSetup()
{
    Master::m_pScriptManager->ExcuteModuleFunction(ModuleName, "void mission_setup()");
}

//*---------------------------------------------------------------------------------------
//*【?】ミッションスクリプトのメイン関数実行
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MissionDirector::ExcuteScriptMissionMain()
{
    Master::m_pScriptManager->ExcuteModuleFunction(ModuleName, "void mission_main()");
}

//*---------------------------------------------------------------------------------------
//*【?】ミッションスクリプトの読み込み
//*
//* [引数] 
//* _filePath : ファイルパス
//* [返値]
//* true  : 読みとり成功
//* false : 読みとり失敗
//*----------------------------------------------------------------------------------------
bool MissionDirector::LoadMissionScript(const std::string& _filePath)
{
    // =====================================
    // スクリプト読み込み
    // =====================================
    bool isRes = false;
    isRes = Master::m_pScriptManager->LoadScript(ModuleName, _filePath.c_str());
    if (!isRes)
    {
        return false;
    }

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】ステージ環境の設定
//*
//* [引数] 
//* &param : 環境データ
//* 
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void MissionDirector::SetStageEnvironmentParam(const UtilityData::StageEnvironmentParam& param)
{
    m_EnvironmentParam = param;

    RendererEngine* renderer = Master::m_pDataManager->get_RendererEngine();

    if (renderer == nullptr){
        return;
    }

    /*
    *  DOFとフォグの設定
    */
    renderer->set_DofParam(param.dofStart, param.dofEnd);
    renderer->set_FogParam(param.fogColor, param.fogStart, param.fogEnd);

    // ディレクションライトの取得
    auto dirLightObj = Master::m_pGameObjectManager->get_ObjectByTag("DirectionLight");
    if (!dirLightObj){
        return;
    }
    auto dirLight = dirLightObj->get_Component<DirectionalLight>();

    /*
    * ディレクションライトの設定
    */
    dirLight->set_LightColor(param.dirLightColor);
    dirLight->set_Intensity(param.dirLightIntensity);
}