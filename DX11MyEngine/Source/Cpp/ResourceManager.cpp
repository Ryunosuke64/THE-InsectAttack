//--------------------------------------------------------------------------------------
//      * Includes *
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "ResourceManager.h"    // リソース
#include "RendererEngine.h"
#include "Texture.h"
#include "ModelData.h"
#include <DirectXTex.h>
//#include <locale>
//#include <codecvt>

// CSV読み込み用
#include <fstream>
#include <sstream>

using namespace DirectX;

ResourceManager::~ResourceManager()
{
}

//--------------------------------------------------------------------------------------
//      * ResourceManager - 初期化 - *
// 引数：デバイスの共有ポインタ
//--------------------------------------------------------------------------------------
bool ResourceManager::Init(std::shared_ptr<RendererEngine> renderer)
{
    m_pRenderer = renderer;

    return true;
}


//--------------------------------------------------------------------------------------
//      * ResourceManager - 終了 - *
//--------------------------------------------------------------------------------------
void ResourceManager::Term()
{
    //m_TexturesMap.clear();
    //m_RTTextureMap.clear();
    //m_ModelsMap.clear();
    //m_pMaterialMap.clear();
}


//*---------------------------------------------------------------------------------------
//* @:ResourceManager Class 
//*【?】テクスチャのロード
//* 引数：1.ファイルパス
//* 返値：ロードしたTexture
//*----------------------------------------------------------------------------------------
std::shared_ptr<Texture> ResourceManager::LoadWIC_Texture(const std::wstring &path)
{
    auto it = m_TexturesMap.find(path);

    // 既に登録済みならそれを返す
    if (it != m_TexturesMap.end())
    {
        return it->second;
    }

    // shared_ptrを受け取る
    auto render = m_pRenderer.lock();

    // 参照が切れていないなら
    if (render)
    {
        // 登録されていないなら、作成して登録
        auto texture = std::make_shared<Texture>();

        // 読み込み
        if (FAILED(texture->Load_WIC(path, *render)))
        {
            std::string errMsg = "のロードに失敗";
            errMsg = Tool::WStringToString(path.c_str()) + errMsg;
            int result = MessageBoxA(NULL, errMsg.c_str() , "Texture LoadError", MB_ABORTRETRYIGNORE | MB_ICONWARNING);

            return nullptr;
        }
        // 配列に登録
        m_TexturesMap[path] = std::move(texture);

        return m_TexturesMap[path];
    }


    return nullptr;
}


//*---------------------------------------------------------------------------------------
//* @:ResourceManager Class 
//*【?】キューブマップ用 DDSフォーマットテクスチャのロード
//* 引数：1.ファイルパス
//* 返値：ロードしたTexture
//*----------------------------------------------------------------------------------------
std::shared_ptr<Texture> ResourceManager::LoadDDS_CubeMap_Texture(const std::wstring& path)
{
    auto it = m_TexturesMap.find(path);

    // 既に登録済みならそれを返す
    if (it != m_TexturesMap.end())
    {
        return it->second;
    }

    // shared_ptrを受け取る
    auto render = m_pRenderer.lock();

    // 参照が切れていないなら
    if (render)
    {
        // 登録されていないなら、作成して登録
        auto texture = std::make_shared<Texture>();

        // 読み込み
        if (FAILED(texture->Load_DDS_CubeMap(path, *render)))
        {
            std::string errMsg = "のロードに失敗";
            errMsg = Tool::WStringToString(path.c_str()) + errMsg;
            int result = MessageBoxA(NULL, errMsg.c_str(), "DDS Texture LoadError", MB_ABORTRETRYIGNORE | MB_ICONWARNING);
            return nullptr;
        }
        // 配列に登録
        m_TexturesMap[path] = std::move(texture);

        return m_TexturesMap[path];
    }
    return nullptr;
};


//*---------------------------------------------------------------------------------------
//* @:ResourceManager Class 
//*【?】モデルのロード
//* 引数：1.ファイルパス
//* 返値：ロードしたモデル情報
//*----------------------------------------------------------------------------------------
std::shared_ptr<ModelData> ResourceManager::LoadModel(const char *path)
{
    auto it = m_ModelsMap.find(path);

    // 既に登録済みならそれを返す
    if (it != m_ModelsMap.end())
    {
        return it->second;
    }

    // shared_ptrを受け取る
    auto render = m_pRenderer.lock();

    // 参照が切れていないなら
    if (render)
    {
        // 登録されていないなら、作成して登録
        auto model = std::make_shared<ModelData>();

        // 読み込み
        if (!model->Setup(*render, path))
        {
            std::string errMsg = "のロードに失敗";
            errMsg = path + errMsg;
            int result = MessageBoxA(NULL, errMsg.c_str(), "Model Load Error", MB_ABORTRETRYIGNORE | MB_ICONWARNING);
            return nullptr;
        }

        // 配列に登録
        m_ModelsMap[path] = std::move(model);

        return m_ModelsMap[path];
    }
    return nullptr;
}



//*---------------------------------------------------------------------------------------
//* @:ResourceManager Class 
//*【?】SRVからtextureに変換 （）
//* 引数：1.タグ（TextureはあくまでResourceManagerが持つので取り出したいときなどに）
//* 引数：2.変換したいRSVのポインタ
//* 引数：3.横幅
//* 引数：4.縦幅
//* 返値：変換したTexture
//*----------------------------------------------------------------------------------------
std::shared_ptr<Texture> ResourceManager::Convert_SRVToTexture(const std::string &tag, const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> &pSrv, UINT w, UINT h)
{
    auto it = m_RTTextureMap.find(tag);

    // 既に登録済みならそれを返す
    if (it != m_RTTextureMap.end())
    {
        return it->second;
    }

    // shared_ptrを受け取る
    auto render = m_pRenderer.lock();

    // 参照が切れていないなら
    if (render)
    {
        // 登録されていないなら、作成して登録
        auto texture = std::make_shared<Texture>();

        // 読み込み
        texture->set_SRV_ComPtr(pSrv);
        texture->set_Width(w);
        texture->set_Height(h);

        // 配列に登録
        m_RTTextureMap[tag] = std::move(texture);

        return m_RTTextureMap[tag];
    }
    return nullptr;
}

//*---------------------------------------------------------------------------------------
//* @:ResourceManager Class 
//*【?】マテリアルの登録
//* 引数：1.タグ
//* 返値：マテリアルデータ
//*----------------------------------------------------------------------------------------
std::shared_ptr<Material> ResourceManager::FindMaterial(const std::string& tag)
{
    auto it = m_pMaterialMap.find(tag);

    // 登録済みならそれを返す
    if (it != m_pMaterialMap.end())
    {
        return it->second;
    }

    // 登録されていないならデフォルトの物を返す
    MessageBoxA(NULL, "マテリアルが登録されていません", "Material", MB_OK);
    return m_pMaterialMap.find("Default")->second;
}


bool ResourceManager::ImportCSV_AllMaterialData(const std::string& _path)
{
    std::ifstream file(_path);
    std::string line;           // 行を格納する変数

    int i = 0;

    if (file.is_open())
    {
        std::getline(file, line);   // ヘッダ行飛ばす

        while (std::getline(file, line)) // 1行ずつ読み込む
        {
            i++;

            std::stringstream ss(line); // 行をストリームに変換
            Material data;
            std::string toStr;
            std::string tag;    // キーになるタグ
            std::string path;

            // パラメータの取得↓↓↓

            /* タグ */
            std::getline(ss, tag, ',');

            /* ディフューズカラー */
            std::getline(ss, toStr, ',');
            data.m_DiffuseColor.x = std::stof(toStr); //x
            std::getline(ss, toStr, ',');
            data.m_DiffuseColor.y = std::stof(toStr); //y
            std::getline(ss, toStr, ',');
            data.m_DiffuseColor.z = std::stof(toStr); //z
            std::getline(ss, toStr, ',');
            data.m_DiffuseColor.w = std::stof(toStr); //w
            
            /* スペキュラカラー*/
            std::getline(ss, toStr, ',');
            data.m_SpecularColor.x = std::stof(toStr); //x
            std::getline(ss, toStr, ',');
            data.m_SpecularColor.y = std::stof(toStr); //y
            std::getline(ss, toStr, ',');
            data.m_SpecularColor.z = std::stof(toStr); //z
            std::getline(ss, toStr, ',');
            data.m_SpecularColor.w = std::stof(toStr); //w

            /* スペキュラ強度 */
            std::getline(ss, toStr, ',');
            data.m_SpecularPower = std::stof(toStr);

            /* エミッシブカラー */
            std::getline(ss, toStr, ',');
            data.m_EmissiveColor.x = std::stof(toStr); //x
            std::getline(ss, toStr, ',');
            data.m_EmissiveColor.y = std::stof(toStr); //y
            std::getline(ss, toStr, ',');
            data.m_EmissiveColor.z = std::stof(toStr); //z

            /* 環境反射強度 */
            std::getline(ss, toStr, ',');
            data.m_EnvironmentReflectionStrength = std::stof(toStr);

            /* ディフューズマップ */
            std::getline(ss, path, ',');
            if (path != "-1") {
                data.m_DiffuseMap.Texture = LoadWIC_Texture(Tool::StringToWstring(path));
            }

            /* ノーマルマップ */
            std::getline(ss, path, ',');
            if (path != "-1") {
                data.m_NormalMap.Texture = LoadWIC_Texture(Tool::StringToWstring(path));
            }

            /* スペキュラマップ */
            std::getline(ss, path, ',');
            if (path != "-1") {
                data.m_SpecularMap.Texture = LoadWIC_Texture(Tool::StringToWstring(path));
            }

            /* ブレンドモード */
            std::getline(ss, toStr, ',');
            data.m_BlendMode = static_cast<BLEND_MODE>(std::stoi(toStr));

            // 登録
            m_pMaterialMap[tag] = std::make_shared<Material>(data);
        }

        file.close();   // ファイル閉じる
    }
    else
    {
        MessageBoxA(NULL, "CSVが読み込めませんでした", "Load Error", MB_OK);
        return false;
    }

    return true;
}



//*---------------------------------------------------------------------------------------
//* @:ResourceManager Class 
//*【?】マテリアルの登録
//* 引数：1.タグ
//* 引数：2.作成するマテリアルのデータ
//* 返値：void
//*----------------------------------------------------------------------------------------
void ResourceManager::RegisterMaterialData(const std::string& tag, const Material& mat)
{
    auto it = m_pMaterialMap.find(tag);

    // 既に登録済み
    if (it != m_pMaterialMap.end())
    {
        MessageBoxA(NULL, "既に登録済みのマテリアルです", "Material", MB_OK);
        return;
    }

    // 作成 データを引き継ぐ
    std::shared_ptr<Material> pMatData = std::make_shared<Material>(mat);

    // 登録
    m_pMaterialMap[tag] = pMatData;
}
