#pragma once
#include <angelscript.h>
#include "AS_CoreAPI.h"
#include "AS_GameAPI.h"
#include "AS_AudioAPI.h"
#include "AS_MathAPI.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ScriptManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】ASエンジンの生成・終了、
//      モジュール管理、関数実行、エラー処理
//
// ***************************************************************************************
class ScriptManager
{
private:
    asIScriptEngine* m_pEngine = nullptr;
    asIScriptContext* m_pContext = nullptr;

public:
    ScriptManager();
    ~ScriptManager();

    bool Init();
    void Update(float _deltaTime);
    
	void Term();

    bool LoadScript(const std::string& _moduleName, const std::string& _filePath);
    bool StartScript(const std::string& moduleName);
    bool UpdateScript(const std::string& moduleName, float deltaTime);
    void UnloadScript(const std::string& moduleName);

    bool ExcuteModuleFunction(const std::string& _moduleName, const std::string& _funcName);

private:
    // コピー禁止
    ScriptManager(const ScriptManager&) = delete;
    ScriptManager& operator=(const ScriptManager&) = delete;

    void RegisterCoreAPI(asIScriptEngine* engine);
    void RegisterMathAPI(asIScriptEngine* engine);
    void RegisterGameAPI(asIScriptEngine* engine);
    void RegisterMissionAPI(asIScriptEngine* engine);
    void RegisterAudioAPI(asIScriptEngine* engine);
};

