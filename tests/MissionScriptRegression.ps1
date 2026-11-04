$ErrorActionPreference = 'Stop'
$testDirectory = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../Output_Engine/Release/ScriptPathTests'))
[IO.Directory]::CreateDirectory($testDirectory) | Out-Null
$sourceEncoding = [Text.Encoding]::GetEncoding(932)
$scriptSource = [IO.File]::ReadAllText((Join-Path $PSScriptRoot '../DX11MyEngine/Source/Cpp/ScriptManager.cpp'), $sourceEncoding)
$directorSource = [IO.File]::ReadAllText((Join-Path $PSScriptRoot '../DX11MyEngine/Source/Cpp/MissionDirector.cpp'), $sourceEncoding)
function Get-TestMethod($source, $method) {
    $match = [regex]::Match($source, '(?s)bool ' + [regex]::Escape($method) + '\([^\r\n]*\)\r?\n\{.*?\r?\n\}')
    if (-not $match.Success) { throw "Method not found: $method" }
    return $match.Value
}
$prefix = @'
#define NOMINMAX
#include <windows.h>
#include <angelscript.h>
#include "scriptbuilder.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
std::vector<std::wstring> errors;
void ErrorMessage(const std::wstring& message, const std::wstring&) { errors.push_back(message); }
std::wstring StringToWstring(const char* text) {
    const int count = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    std::wstring result(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, text, -1, result.data(), count);
    return result;
}
class ScriptManager {
public:
    asIScriptEngine* m_pEngine = asCreateScriptEngine();
    asIScriptContext* m_pContext = m_pEngine->CreateContext();
    ~ScriptManager() { m_pContext->Release(); m_pEngine->ShutDownAndRelease(); }
    bool LoadScript(const std::string&, const std::string&);
    bool ExcuteModuleFunction(const std::string&, const std::string&);
};
struct Master { static inline ScriptManager* m_pScriptManager = nullptr; };
class MissionDirector {
    bool m_IsScriptLoaded = false;
    bool m_IsScriptReady = false;
    const std::string ModuleName = "MissionModule";
public:
    bool LoadMissionScript(const std::string&);
    bool ExcuteScriptMissionSetup();
    bool ExcuteScriptMissionMain();
};
void Check(bool condition, const char* label) {
    if (!condition) throw std::runtime_error(label);
    std::cout << "PASS: " << label << '\n';
}
void WriteScript(const std::filesystem::path& path, const char* source) {
    std::ofstream file(path, std::ios::binary);
    file << source;
    if (!file) throw std::runtime_error("write test script");
}
'@
$methods = @(
    (Get-TestMethod $scriptSource 'ScriptManager::LoadScript'),
    (Get-TestMethod $scriptSource 'ScriptManager::ExcuteModuleFunction'),
    (Get-TestMethod $directorSource 'MissionDirector::LoadMissionScript'),
    (Get-TestMethod $directorSource 'MissionDirector::ExcuteScriptMissionSetup'),
    (Get-TestMethod $directorSource 'MissionDirector::ExcuteScriptMissionMain')
) -join "`r`n"
$suffix = @'
int main() {
    const auto originalDirectory = std::filesystem::current_path();
    try {
        const auto distribution = originalDirectory / "Output_Engine/Release/ScriptPathTests" /
            L"\u63d0\u51fa" / L"\u5352\u696d\u5236\u4f5c\u30d7\u30ec\u30a4\u30e4\u30d6\u30eb10\u6708\u672b";
        const auto scripts = distribution / "Resource/MISSION_AS";
        std::filesystem::create_directories(scripts);
        WriteScript(scripts / "common.as", "int initialEnemies() { return 1; }");
        WriteScript(scripts / "MISSION_01.as", "#include \"common.as\"\nint enemies = 0; void mission_setup() { enemies = initialEnemies(); } void mission_main() { enemies += 2; }");
        std::filesystem::current_path(distribution);
        ScriptManager manager;
        Master::m_pScriptManager = &manager;
        {
            CScriptBuilder oldBuilder;
            Check(oldBuilder.StartNewModule(manager.m_pEngine, "OldRelative") >= 0, "create old-path test module");
            const int result = oldBuilder.AddSectionFromFile("Resource/MISSION_AS/MISSION_01.as");
            if (GetACP() != CP_UTF8) Check(result < 0, "reproduce original failure in Japanese directory");
            else std::cout << "SKIP: system ANSI code page is already UTF-8\n";
            manager.m_pEngine->DiscardModule("OldRelative");
        }
        MissionDirector director;
        Check(director.LoadMissionScript("Resource/MISSION_AS/MISSION_01.as"), "production loader reads Japanese path and relative include");
        Check(!director.ExcuteScriptMissionMain(), "mission cannot run before setup");
        Check(director.ExcuteScriptMissionSetup(), "setup execution returns true");
        Check(director.ExcuteScriptMissionMain(), "main execution returns true");
        auto module = manager.m_pEngine->GetModule("MissionModule");
        auto counter = static_cast<int*>(module->GetAddressOfGlobalVar(module->GetGlobalVarIndexByName("enemies")));
        Check(*counter == 3, "setup and main both execute enemy logic");
        Check(!director.LoadMissionScript("Resource/MISSION_AS/MISSING.as"), "missing script fails");
        Check(!errors.empty() && errors.back().find(distribution.filename().wstring()) != std::wstring::npos && errors.back().find(distribution.root_name().wstring()) != std::wstring::npos, "read error includes Japanese absolute path");
        const auto errorCount = errors.size();
        Check(!director.ExcuteScriptMissionSetup() && !director.ExcuteScriptMissionMain(), "failed load blocks setup and main");
        Check(errors.size() == errorCount, "failed load avoids cascading missing-function errors");
        Check(manager.m_pEngine->GetModule("MissionModule") == nullptr, "failed load discards incomplete module");
        WriteScript(scripts / "broken.as", "void mission_setup( { syntax error");
        Check(!director.LoadMissionScript("Resource/MISSION_AS/broken.as"), "compilation error fails loading");
        Check(!director.ExcuteScriptMissionSetup() && !director.ExcuteScriptMissionMain(), "compilation error blocks execution");
        WriteScript(scripts / "exception.as", "int denominator() { return 0; } void mission_setup() { int n = 1 / denominator(); } void mission_main() {}");
        Check(director.LoadMissionScript("Resource/MISSION_AS/exception.as"), "load runtime-exception script");
        Check(!director.ExcuteScriptMissionSetup(), "setup exception is reported as failure");
        Check(!director.ExcuteScriptMissionMain(), "setup exception blocks mission main");
        Check(director.LoadMissionScript("Resource/MISSION_AS/MISSION_01.as") && director.ExcuteScriptMissionSetup() && director.ExcuteScriptMissionMain(), "mission can reload after failures");
        std::filesystem::current_path(originalDirectory);
        std::cout << "All mission script regression checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::filesystem::current_path(originalDirectory);
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
'@
[IO.File]::WriteAllText((Join-Path $testDirectory 'MissionScriptFixRegression.cpp'), $prefix + "`r`n" + $methods + "`r`n" + $suffix, [Text.UTF8Encoding]::new($false))
$workspace = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$compile = @"
@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl /nologo /std:c++20 /EHsc /MD /utf-8 /I"$workspace\angelscript\sdk\angelscript\include" /I"$workspace\DX11MyEngine\Source\Lib\angelscript\add_on\scriptbuilder" "$testDirectory\MissionScriptFixRegression.cpp" "$workspace\DX11MyEngine\Source\Lib\angelscript\add_on\scriptbuilder\scriptbuilder.cpp" /Fe:"$testDirectory\MissionScriptFixRegression.exe" /Fo:"$testDirectory\\" /link "$workspace\angelscript\sdk\angelscript\lib\angelscript64.lib"
exit /b %errorlevel%
"@
[IO.File]::WriteAllText((Join-Path $testDirectory 'BuildRegression.cmd'), $compile, [Text.Encoding]::ASCII)
& $env:ComSpec /d /c (Join-Path $testDirectory 'BuildRegression.cmd')
if ($LASTEXITCODE -ne 0) { throw 'Regression build failed' }
& (Join-Path $testDirectory 'MissionScriptFixRegression.exe')
if ($LASTEXITCODE -ne 0) { throw 'Regression execution failed' }
