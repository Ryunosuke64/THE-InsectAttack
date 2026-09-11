#include "pch.h"
#include "ScriptManager.h"
#include <angelscript/add_on/scriptbuilder/scriptbuilder.h>     // ファイル読み込み用
#include <angelscript/add_on/scriptstdstring/scriptstdstring.h> // string型を使えるようにする
#include <angelscript/add_on/scriptarray/scriptarray.h>         // arrayを使えるようにする
#include <angelscript/add_on/scriptmath/scriptmath.h>           // 数学関数を使えるようにする
#include <angelscript/add_on/scripthelper/scripthelper.h>       // 数学関数を使えるようにする

// [interface]がWindows SDKのものと衝突しているので、asbindのインクルード時に一時的に無効にする
#pragma push_macro("interface")  // 現在のマクロを保存
#undef interface                 // 一時的に無効化
#include <asbind20/asbind.hpp>   // バインド用ライブラリ
#pragma pop_macro("interface")   // 元のマクロに戻す

using namespace Tool;
using namespace VECTOR3;
using namespace UtilityData;

#include <fstream>

void GeneratePredefined(
    asIScriptEngine* engine,
    const std::string& filePath)
{
    std::ofstream file(filePath);

    if (!file.is_open())
    {
        return;
    }

    // ============================
    // Object Type
    // ============================

    const asUINT objectCount = engine->GetObjectTypeCount();

    for (asUINT i = 0; i < objectCount; ++i)
    {
        asITypeInfo* type = engine->GetObjectTypeByIndex(i);

        if (type == nullptr)
        {
            continue;
        }

        const std::string typeName = type->GetName();

        file << "class " << typeName << "\n";
        file << "{\n";

        // ----------------------------
        // Constructor
        // ----------------------------

        const asUINT behaviourCount = type->GetBehaviourCount();

        for (asUINT j = 0; j < behaviourCount; ++j)
        {
            asEBehaviours behaviour;

            asIScriptFunction* func =
                type->GetBehaviourByIndex(j, &behaviour);

            if (func == nullptr)
            {
                continue;
            }

            if (behaviour == asBEHAVE_CONSTRUCT)
            {
                std::string decl =
                    func->GetDeclaration(
                        false,
                        false,
                        true
                    );

                // 例:
                // void f(float x, float y)
                //
                // ↓
                //
                // VEC3(float x, float y);

                const size_t paramPos = decl.find('(');

                if (paramPos != std::string::npos)
                {
                    file << "    "
                        << typeName
                        << decl.substr(paramPos)
                        << ";\n";
                }
            }
        }

        // ----------------------------
        // Property
        // ----------------------------

        const asUINT propertyCount =
            type->GetPropertyCount();

        for (asUINT j = 0; j < propertyCount; ++j)
        {
            const char* decl =
                type->GetPropertyDeclaration(j);

            if (decl != nullptr)
            {
                file << "    "
                    << decl
                    << ";\n";
            }
        }

        // ----------------------------
        // Method
        // ----------------------------

        const asUINT methodCount =
            type->GetMethodCount();

        for (asUINT j = 0; j < methodCount; ++j)
        {
            asIScriptFunction* method =
                type->GetMethodByIndex(j);

            if (method == nullptr)
            {
                continue;
            }

            file << "    "
                << method->GetDeclaration(
                    false,
                    false,
                    true
                )
                << ";\n";
        }

        file << "}\n\n";
    }

    // ============================
    // Enum
    // ============================

    const asUINT enumCount = engine->GetEnumCount();

    for (asUINT i = 0; i < enumCount; ++i)
    {
        asITypeInfo* type =
            engine->GetEnumByIndex(i);

        if (type == nullptr)
        {
            continue;
        }

        file << "enum "
            << type->GetName()
            << "\n{\n";

        const asUINT valueCount =
            type->GetEnumValueCount();

        for (asUINT j = 0; j < valueCount; ++j)
        {
            long long value = 0;

            const char* name =
                type->GetEnumValueByIndex(
                    j,
                    &value
                );

            file << "    "
                << name
                << " = "
                << value;

            if (j + 1 < valueCount)
            {
                file << ",";
            }

            file << "\n";
        }

        file << "}\n\n";
    }

    // ============================
    // Global Function
    // ============================

    const asUINT functionCount =
        engine->GetGlobalFunctionCount();

    for (asUINT i = 0; i < functionCount; ++i)
    {
        asIScriptFunction* function =
            engine->GetGlobalFunctionByIndex(i);

        if (function == nullptr)
        {
            continue;
        }

        file << function->GetDeclaration(
            false,
            false,
            true
        )
            << ";\n";
    }
}

// メッセージコールバック
void MessageCallback(const asSMessageInfo* msg, void* param)
{
    const char* type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING) type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION) type = "INFO";

    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}


//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
ScriptManager::ScriptManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
ScriptManager::~ScriptManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数] 
//* なし
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool ScriptManager::Init()
{
    // =====================================
    // AngelScriptエンジン作成
    // =====================================
    m_pEngine = asCreateScriptEngine();
    assert(m_pEngine != nullptr);

    // メッセージコールバック登録
    int r = m_pEngine->SetMessageCallback(
        asFUNCTION(MessageCallback), 
        0, 
        asCALL_CDECL
    );
    assert(r >= 0);

    // =====================================
    // アドオンの登録
    // =====================================
    {
        // arrayアドオンの登録
        RegisterScriptArray(m_pEngine, true);

        // string型を使えるようにする
        RegisterStdString(m_pEngine);

        // 数学
        RegisterScriptMath(m_pEngine);
    }

    // =====================================
    // AS内で使用するAPIの登録
    // =====================================
    {
        // コアAPI
        RegisterCoreAPI(m_pEngine);

        // 数学API
        RegisterMathAPI(m_pEngine);

        // ゲームAPI
        RegisterGameAPI(m_pEngine);

        // ミッションAPI
        RegisterMissionAPI(m_pEngine);
    }

    // コンテキスト作成
    m_pContext = m_pEngine->CreateContext();


    // デバッグ時のみ生成
#ifdef _DEBUG
    // 登録済みAPIを出力
    WriteConfigToFile(m_pEngine, "AngelScriptAPI.txt");
    GeneratePredefined(m_pEngine, "as.predefined");
#endif

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数] 
//* _deltaTime : デルタタイム
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ScriptManager::Update(float _deltaTime)
{
    int r = 0;

}

//*---------------------------------------------------------------------------------------
//*【?】終了
//*
//* [引数] なし
//* [返値] なし 
//*----------------------------------------------------------------------------------------
void ScriptManager::Term()
{
    // クリーンアップ
    m_pContext->Release();
    m_pEngine->ShutDownAndRelease();
}


//*---------------------------------------------------------------------------------------
//*【?】モジュールの関数実行
//*
//* [引数] 
//* _moduleName : モジュール名
//* _funcName   : 関数名
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
bool ScriptManager::ExcuteModuleFunction(const std::string& _moduleName, const std::string& _funcName)
{
    int r = 0;

    // モジュール取得
    asIScriptModule* mod = m_pEngine->GetModule(_moduleName.c_str());
    if (!mod)
    {
        ErrorMessage(L"モジュールが見つかりません\n", L"ScriptManager");
        return false;
    }

    // 関数取得
    asIScriptFunction* func = mod->GetFunctionByDecl(_funcName.c_str());
    if (!func)
    {
        ErrorMessage(L"関数が見つかりません\n", L"ScriptManager");
        return false;
    }

    // コンテキスト作成・準備・実行
    m_pContext->Prepare(func);
    r = m_pContext->Execute();
    if (r != asEXECUTION_FINISHED)
    {
        if (r == asEXECUTION_EXCEPTION)
        {
            ErrorMessage(StringToWstring(m_pContext->GetExceptionString()), L"ScriptManager");
        }
        return false;
    }
}


//*---------------------------------------------------------------------------------------
//*【?】AngelScriptのスクリプトをロードする
//*
//* [引数] 
//* _moduleName : モジュール名
//* _filePath   : ファイルパス
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
bool ScriptManager::LoadScript(const std::string& _moduleName, const std::string& _filePath)
{
    // =====================================
    // スクリプト読み込み
    // =====================================
    // スクリプトビルダーでモジュール作成
    CScriptBuilder builder;
    int r = builder.StartNewModule(m_pEngine, _moduleName.c_str());
    if (r < 0)
    {
        ErrorMessage(L"モジュール作成失敗\n", L"ScriptManager");
        return false;
    }

    r = builder.AddSectionFromFile(_filePath.c_str());
    if (r < 0)
    {
        ErrorMessage(L"ファイル読み込み失敗\n", L"ScriptManager");
        return false;
    }

    // =====================================
    // スクリプトをコンパイル
    // =====================================
    r = builder.BuildModule();
    if (r < 0)
    {
        ErrorMessage(L"コンパイルが出来ませんでした\n", L"ScriptManager");
        return false;
    }

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】
//*
//* [引数] 
//* _moduleName : モジュール名
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
bool ScriptManager::StartScript(const std::string& _moduleName)
{
    return true;
}
//*---------------------------------------------------------------------------------------
//*【?】
//*
//* [引数] 
//* _moduleName : モジュール名
//* deltaTime   : デルタタイム
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
bool ScriptManager::UpdateScript(const std::string& _moduleName, float deltaTime)
{
    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】
//*
//* [引数] 
//* _moduleName : モジュール名
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ScriptManager::UnloadScript(const std::string& _moduleName)
{

}

//*---------------------------------------------------------------------------------------
//*【?】コアAPIの登録
//*
//* [引数] 
//* *engine : ASエンジン
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ScriptManager::RegisterCoreAPI(asIScriptEngine* engine)
{
    int r = 0;

    // =====================================
    // ErrorMesageBox
    // =====================================
    asbind20::global(engine).function(
        "void ErrorMesageBox(const string &in  caption, const string &in msg)",
        &GIGA_Engine::ScriptAPI::Core::ErrorMesageBox
    );

    // =====================================
    // GetDeltaTime
    // =====================================
    asbind20::global(engine).function(
        "float GetDeltaTime()",
        &GIGA_Engine::ScriptAPI::Core::GetDeltaTime
    );

}

//*---------------------------------------------------------------------------------------
//*【?】数学APIの登録
//*
//* [引数] 
//* *engine : ASエンジン
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ScriptManager::RegisterMathAPI(asIScriptEngine* engine)
{
    // VEC3
    asbind20::value_class<VEC3>(
        engine,
        "VEC3",
        asOBJ_APP_CLASS_ALLFLOATS |
        asOBJ_APP_CLASS_MORE_CONSTRUCTORS
    )
        .behaviours_by_traits()
        .constructor<float>("float")
        .constructor<float, float, float>("float, float, float")
        .property("float x", &VEC3::x)
        .property("float y", &VEC3::y)
        .property("float z", &VEC3::z);
}

//*---------------------------------------------------------------------------------------
//*【?】ゲームオブジェクトAPIの登録
//*
//* [引数] 
//* *engine : ASエンジン
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ScriptManager::RegisterGameAPI(asIScriptEngine* engine)
{
    int r = 0;

    // =====================================
    // ENEMY_TYPE
    // =====================================
    asbind20::enum_<EnemyData::ENEMY_TYPE>(
        engine,
        "ENEMY_TYPE"
    )
        .value(EnemyData::ENEMY_TYPE::GIANT_ANT_Normal, "GIANT_ANT_Normal")
        .value(EnemyData::ENEMY_TYPE::OCTAHEDRON, "OCTAHEDRON");
}


//*---------------------------------------------------------------------------------------
//*【?】ミッション用APIの登録
//*
//* [引数] 
//* *engine : ASエンジン
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ScriptManager::RegisterMissionAPI(asIScriptEngine* engine)
{
    // =====================================
    // StageEnvironmentParam
    // =====================================
    asbind20::value_class<StageEnvironmentParam>(
        engine,
        "StageEnvironmentParam",
        asOBJ_APP_CLASS_ALLFLOATS |
        asOBJ_APP_CLASS_MORE_CONSTRUCTORS
    )
        .behaviours_by_traits()
        .property("VEC3 dirLightColor", &StageEnvironmentParam::dirLightColor)
        .property("VEC3 dirLightDirection", &StageEnvironmentParam::dirLightDirection)
        .property("float dirLightIntensity", &StageEnvironmentParam::dirLightIntensity)
        .property("VEC3 fogColor", &StageEnvironmentParam::fogColor)
        .property("float fogStart", &StageEnvironmentParam::fogStart)
        .property("float fogEnd", &StageEnvironmentParam::fogEnd)
        .property("float dofStart", &StageEnvironmentParam::dofStart)
        .property("float dofEnd", &StageEnvironmentParam::dofEnd
        );


    // =====================================
    // SetStageEnvironmentParam
    // =====================================
    asbind20::global(engine).function(
        "void SetStageEnvironmentParam(const StageEnvironmentParam&in param)",
        &GIGA_Engine::ScriptAPI::Game::SetStageEnvironmentParam
    );
    

    // =====================================
    // SpawnEnemy
    // =====================================
    asbind20::global(engine).function(
        "uint32 SpawnEnemy(ENEMY_TYPE type,const VEC3&in pos, const VEC3&in rot, float hp, bool isAggro)",
        &GIGA_Engine::ScriptAPI::Game::SpawnEnemy
    );

    // =====================================
    // SpawnEnemyGroup
    // =====================================
    asbind20::global(engine).function(
        "uint32 SpawnEnemyGroup(ENEMY_TYPE type, const VEC3&in pos, float spawnRadius, uint32 count, float hp, bool isAggro)",
        &GIGA_Engine::ScriptAPI::Game::SpawnEnemyGroup
    );
    
    // =====================================
    // IsEnemyDead
    // =====================================
    asbind20::global(engine).function(
        "bool IsEnemyDead(uint32 enemyID)",
        &GIGA_Engine::ScriptAPI::Game::IsEnemyDead
    );
        
    // =====================================
    // IsEnemyGroupDestroyed
    // =====================================
    asbind20::global(engine).function(
        "bool IsEnemyGroupDestroyed(uint32 groupID)",
        &GIGA_Engine::ScriptAPI::Game::IsEnemyGroupDestroyed
    );

    // =====================================
    // GetEnemyGroupAliveCount
    // =====================================
    asbind20::global(engine).function(
        "int GetEnemyGroupAliveCount(uint32 groupID)",
        &GIGA_Engine::ScriptAPI::Game::GetEnemyGroupAliveCount
    );
        
    // =====================================
    // GetAliveEnemyCount
    // =====================================
    asbind20::global(engine).function(
        "int GetAliveEnemyCount()",
        &GIGA_Engine::ScriptAPI::Game::GetAliveEnemyCount
    );


}


//*---------------------------------------------------------------------------------------
//*【?】オーディオAPIの登録
//*
//* [引数] 
//* *engine : ASエンジン
//* 
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ScriptManager::RegisterAudioAPI(asIScriptEngine* engine)
{
    
}