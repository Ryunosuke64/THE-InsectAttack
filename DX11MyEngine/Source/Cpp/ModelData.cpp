#include "pch.h"
#include "ModelData.h"
#include "ResourceManager.h"
#include "RendererEngine.h"


using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace VERTEX;
using namespace Tool;
using namespace DirectX;

//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
ModelData::ModelData() :
    m_VertexNum(0),
    m_IndexNum(0),
    m_MeshNum(0),
    m_BoneNum(0),
    m_AnimNum(0),
    m_MaterialNum(0),
    m_pScene(nullptr),
    //m_pConstanrBufferBonesData(nullptr),
    //m_pCBTransformSet(nullptr),
    //m_pCBMaterialDataSet(nullptr),
    m_pMeshes(nullptr),
    m_HasLocalBounds(false),
    m_ShaderType(SHADER_TYPE::NONE),
    m_ShadowShaderType(SHADER_TYPE::NONE)
{
    m_BoneList.clear();
    m_pMaterialList.clear();
    m_BoneIndexMap.clear();
    m_pNodeList.clear();
    m_pAnimations.clear();
}

//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
ModelData::~ModelData()
{

}

//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】モデルデータのセットアップ     ※ 必ず最初に呼ぶ
//      ファイルパス名は日本語非対応
// 
//* 引数：1.RendererEngine
//* 引数：2.ファイルパス
//* 返値：bool セットアップが完了したか
//*----------------------------------------------------------------------------------------
bool ModelData::Setup(RendererEngine &renderer, const char *filePath)
{
    // aiProcess_FlipUVs ＵＶ値をdirect3Dに合うようにしてくれる (Ｖ軸の反転)
    // 他のフラグもあったほうがいいやつらしい https://qiita.com/dpals39/items/1681d9101e58b5aefa27
    u_int flag =
        aiProcess_CalcTangentSpace |        // タンジェント空間パラメータの計算
        aiProcess_GenNormals |              // 法線の生成（すでにある場合は無視される）
        aiProcess_MakeLeftHanded |          // 左手座標系に変換
        aiProcess_FlipWindingOrder |        // 頂点の順番を反転
        aiProcess_FlipUVs |                 // UV反転
        aiProcess_OptimizeMeshes |          // メッシュの最適化
        aiProcess_Triangulate |             // 三角形化
        aiProcess_JoinIdenticalVertices |   // 重複頂点の結合
        aiPrimitiveType_LINE |              // 線分も読み込む
        aiPrimitiveType_POINT |             // 点も読み込む
        aiProcess_GenBoundingBoxes;         // バウンディングボックス生成（mAABBに格納される）

    // 不要なノードを勝手に追加しないようにする https://qiita.com/24ban/items/3cdb37188b74bcf1028c
    m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    /* ↓日本語は無理っぽい ↓*/
    m_pScene = m_Importer.ReadFile(filePath, flag);	// ファイルの読み込み

    // 読み込み失敗なら返す
    if (m_pScene == nullptr)return false;

    // マテリアル情報の抽出
    //MatrialExtraction(m_pScene);

    // アニメーション情報の抽出
    if (m_pScene->HasAnimations()) {
        AnimationExtraction(m_pScene);
    }

    // ノード情報の抽出
    NodeExtraction(m_pScene->mRootNode, -1);

    // メッシュデータを取得し、セットアップ
    if (m_pScene->HasMeshes())
    {
        m_MeshNum = m_pScene->mNumMeshes;	// メッシュ数取得
        m_pMeshes = new ModelMesh[m_MeshNum];

        bool hasBounds = false;         // ローカル境界を保持しているか
        DirectX::XMFLOAT3 localMin{};   
        DirectX::XMFLOAT3 localMax{};


        for (u_int meshIdx = 0; meshIdx < m_MeshNum; ++meshIdx)
        {
            aiMesh *pMeshData = m_pScene->mMeshes[meshIdx];


            // 頂点を持つメッシュだけ境界へ含める
            if (pMeshData->HasPositions() &&
                pMeshData->mNumVertices > 0)
            {
                // コリジョンメッシュセットアップ
                CollisionMeshSetup(pMeshData);

                const aiVector3D& meshMin = pMeshData->mAABB.mMin;
                const aiVector3D& meshMax = pMeshData->mAABB.mMax;

                if (!hasBounds)
                {
                    localMin = {
                        meshMin.x,
                        meshMin.y,
                        meshMin.z
                    };

                    localMax = {
                        meshMax.x,
                        meshMax.y,
                        meshMax.z
                    };

                    hasBounds = true;
                }
                // 既に設定している場合は、設定している境界と現在のメッシュの境界を比べる
                else
                {
                    localMin.x = std::min(localMin.x, meshMin.x);
                    localMin.y = std::min(localMin.y, meshMin.y);
                    localMin.z = std::min(localMin.z, meshMin.z);

                    localMax.x = std::max(localMax.x, meshMax.x);
                    localMax.y = std::max(localMax.y, meshMax.y);
                    localMax.z = std::max(localMax.z, meshMax.z);
                }
            }

            // ボーン情報の抽出
            if (pMeshData->HasBones()) {
                BoneExtraction(pMeshData);
            }

            // メッシュのセットアップ
            if (m_pMeshes[meshIdx].Setup(renderer, pMeshData) == false) {
                return false;
            }

            m_VertexNum += pMeshData->mNumVertices;
            m_BoneNum += pMeshData->mNumBones;
        }

        // ローカル境界の中心位置と、大きさを求める
        if (hasBounds)
        {
            const DirectX::XMFLOAT3 center = {
                (localMin.x + localMax.x) * 0.5f,
                (localMin.y + localMax.y) * 0.5f,
                (localMin.z + localMax.z) * 0.5f
            };

            const DirectX::XMFLOAT3 extents = {
                (localMax.x - localMin.x) * 0.5f,
                (localMax.y - localMin.y) * 0.5f,
                (localMax.z - localMin.z) * 0.5f
            };

            m_LocalBounds = DirectX::BoundingBox(center, extents);
            m_HasLocalBounds = true;
        }
    }

    m_AnimNum = m_pScene->mNumAnimations;

    m_pMaterialList.resize(m_pScene->mNumMaterials);

    // ボーン変換用定数バッファの作成
    if (CreateBonesCBuffer(renderer) == false)
    {
        return false;
    }

    // マテリアル定数バッファの作成
    if (CreateMaterialCBuffer(renderer) == false)
    {
        return false;
    }

    // ワールド定数バッファの作成
    if (CreateTransformCBuffer(renderer) == false)
    {
        return false;
    }

    return true;
}


//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】テクスチャマップの設定
//   ※ 本当はAssimpの処理で内部のパスを参照し、テクスチャを自動で適応できるようにする予定だったけど、
//      mixamoから持ってきたモデルのパスがおかしかったため、手動で付けるようにした。
//      そこまで大きなモデルファイルを使う予定がないので、
//      マテリアル番号はモデルビューアなどで確認して・・・(m´・ω・｀)m ｺﾞﾒﾝ…
//* 引数：1.マップタイプ
//* 引数：2.マテリアル番号
//* 引数：3.ファイルパス
//* 返値：bool
//*----------------------------------------------------------------------------------------
bool ModelData::SetupTextureMap(std::shared_ptr<Material> pMatData, int matIndex)
{
    Material mat{};

    // テクスチャ読み込み
    //auto texture = Master::m_pResourceManager->LoadTexture(path);
    //if (texture == nullptr) {
    //    return false;
    //}

    // 範囲外アクセスチェック
    if (m_pMaterialList.size() <= matIndex) {
        return false;
    }

    m_pMaterialList[matIndex] = pMatData;

    //// 対応したマップにテクスチャを入れる
    //switch (mapType)
    //{
    //case TEXTURE_MAP_NONE:
    //    break;
    //case TEXTURE_MAP_DIFFUSE:
    //    m_MaterialList[matIndex].Diffuse.Texture = texture;
    //    break;
    //case TEXTURE_MAP_NORMAL:
    //    m_MaterialList[matIndex].Normal.Texture = texture;
    //    break;
    //case TEXTURE_MAP_SPECULAR:
    //    m_MaterialList[matIndex].Specular.Texture = texture;
    //    break;
    //default:
    //    break;
    //}

    //return true;

    return true;
}


//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】アニメーション情報の抽出
// 
//* 引数：1.モデルシーン Assimp
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelData::AnimationExtraction(const aiScene *scene)
{
    for (size_t i = 0; i < scene->mNumAnimations; i++)
    {
        AnimationData *animData = new AnimationData();// 抽出データ格納先
        aiAnimation *anim = scene->mAnimations[i]; // アニメーション情報

        animData->Name = anim->mName.C_Str(); // アニメーション名
        animData->Duration = anim->mDuration; // アニメーションの長さ
        animData->TicksPerSecond = anim->mTicksPerSecond; // 1秒あたりのティック数
        if (animData->TicksPerSecond < 0.0)
        {
            animData->TicksPerSecond = 1.0;
        }
        // チャンネル情報を取得する ===============================================================
        for (size_t j = 0; j < anim->mNumChannels; j++)
        {
            NodeAnimChannel channelData{};                     // チャンネルデータ格納先
            aiNodeAnim *channel = anim->mChannels[j];          // チャンネル情報
            channelData.NodeName = channel->mNodeName.C_Str(); // 影響を受けるノード名

            // 座標キーフレームを取得 ===============================================================
            for (size_t key = 0; key < channel->mNumPositionKeys; key++)
            {
                PositionKey posKey{};
                posKey.Time = channel->mPositionKeys[key].mTime; // 時間
                posKey.Value.x = channel->mPositionKeys[key].mValue.x; // X座標
                posKey.Value.y = channel->mPositionKeys[key].mValue.y; // Y座標
                posKey.Value.z = channel->mPositionKeys[key].mValue.z; // Z座標
                channelData.PositionKeys.push_back(posKey); // チャンネルに追加
            }

            // 回転キーフレームを取得 ===============================================================
            for (size_t key = 0; key < channel->mNumRotationKeys; key++)
            {
                RotationKey rotKey{};
                rotKey.Time = channel->mRotationKeys[key].mTime; // 時間
                rotKey.Value = channel->mRotationKeys[key].mValue; // クォータニオン値
                channelData.RotationKeys.push_back(rotKey); // チャンネルに追加
            }

            // スケールキーフレームを取得 ===============================================================
            for (size_t key = 0; key < channel->mNumScalingKeys; key++)
            {
                ScalingKey sclKey{};
                sclKey.Time = channel->mScalingKeys[key].mTime; // 時間
                sclKey.Value.x = channel->mScalingKeys[key].mValue.x; // Xスケール
                sclKey.Value.y = channel->mScalingKeys[key].mValue.y; // Yスケール
                sclKey.Value.z = channel->mScalingKeys[key].mValue.z; // Zスケール
                channelData.ScalingKeys.push_back(sclKey); // チャンネルに追加
            }

            animData->Channels.push_back(channelData); // アニメーションに追加
        }

        m_pAnimations.push_back(animData); // アニメーションリストに追加
    }
}


//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】ノード情報の抽出  再帰関数
// 
//* 引数：1.ノード    Assimp
//* 引数：2.親ノードのインデックス
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelData::NodeExtraction(const aiNode *pNode, int parentIdx)
{
    NodeInfo *nodeData = new NodeInfo();    // 格納先
    nodeData->Name = pNode->mName.C_Str();  // 名前
    nodeData->ParentIndex = parentIdx;      // 親のインデックス

    aiMatrix4x4 tempMat = pNode->mTransformation;   // ローカル変換行列
    nodeData->Transformation = XMMatrixTranspose(
        XMMATRIX(
            tempMat.a1, tempMat.a2, tempMat.a3, tempMat.a4,
            tempMat.b1, tempMat.b2, tempMat.b3, tempMat.b4,
            tempMat.c1, tempMat.c2, tempMat.c3, tempMat.c4,
            tempMat.d1, tempMat.d2, tempMat.d3, tempMat.d4
        )
    );


    // 現在のノードインデックスを保持
    int myIndex = static_cast<int>(m_pNodeList.size());
    m_pNodeList.push_back(nodeData);

    // 親ノードに現在のインデックスを子として登録
    // ルート（-1）以外
    if (parentIdx != -1)
    {
        m_pNodeList[parentIdx]->ChildrenIndices.push_back(myIndex);
    }

    // 子ノードを再帰的に処理
    for (size_t i = 0; i < pNode->mNumChildren; i++)
    {
        // 現在のインデックスを親として渡す
        NodeExtraction(pNode->mChildren[i], myIndex);
    }
}


//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】ボーン情報の抽出
// 
//* 引数：1.メッシュ情報 Assimp
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelData::BoneExtraction(aiMesh *mesh)
{
    //mesh->mBones[i]->mName;           // ボーン名
    //mesh->mBones[i]->mNumWeights;     // このボーンの影響を受ける頂点数
    //mesh->mBones[i]->mWeights;        // 影響度
    //mesh->mBones[i]->mOffsetMatrix;   // オフセット行列

    for (size_t i = 0; i < mesh->mNumBones; i++)
    {
        aiBone *bone = mesh->mBones[i]; // メッシュに含まれるボーン
        BoneInfo extractionData{};     // 抽出データ格納先

        // ボーン名取得
        extractionData.Name = bone->mName.C_Str();

        // オフセット行列を転置して取得
        extractionData.OffsetMtx = XMMatrixTranspose(
            XMMATRIX(
                bone->mOffsetMatrix.a1, bone->mOffsetMatrix.a2, bone->mOffsetMatrix.a3, bone->mOffsetMatrix.a4,
                bone->mOffsetMatrix.b1, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.b4,
                bone->mOffsetMatrix.c1, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.c4,
                bone->mOffsetMatrix.d1, bone->mOffsetMatrix.d2, bone->mOffsetMatrix.d3, bone->mOffsetMatrix.d4
            )
        );

        extractionData.WeightList.reserve(bone->mNumWeights); // 影響度リストの容量確保

        // 各頂点への影響度を取得
        for (size_t j = 0; j < bone->mNumWeights; j++)
        {
            aiVertexWeight weight = bone->mWeights[j];
            VertexBoneWeightData weightData{};

            weightData.VertexID = weight.mVertexId;  // 影響を受ける頂点ID取得
            weightData.Weight = weight.mWeight;      // 影響度（0.0～1.0）取得
            extractionData.WeightList.push_back(weightData);    // ボーン情報に追加
        }

        // ボーンインデックスマップに登録
        m_BoneIndexMap[extractionData.Name] = static_cast<int>(i);

        // ボーンリストに追加
        m_BoneList.push_back(extractionData);
    }
}

//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】マテリアル情報の抽出
//* ※本来はここで自動的にパスを参照する予定だったけど、抽出したパスが絶対パス？になっていたため、
//*     set_TextureMapで手動でつけてね(｡-人-｡) ｺﾞﾒﾝﾈ
//* 
//* 引数：1.メッシュ情報 Assimp
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelData::MatrialExtraction(const aiScene *scene)
{

}

//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】ボーン変換用定数バッファの作成
// 
//* 引数：1.RendererEngine 
//* 返値：bool
//*----------------------------------------------------------------------------------------
bool ModelData::CreateBonesCBuffer(RendererEngine &renderer)
{
    //auto device = renderer.get_Device();	// デバイス取得
    //m_pConstanrBufferBonesData = new CB_BONES_DATA_SET;

    //HRESULT hr = S_OK;

    //// リソース設定
    //D3D11_BUFFER_DESC desc;
    //ZeroMemory(&desc, sizeof(desc));
    //desc.Usage = D3D11_USAGE_DEFAULT;
    //desc.ByteWidth = sizeof(CB_BONES_DATA);
    //desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    //desc.CPUAccessFlags = 0;
    //desc.MiscFlags = 0;
    //desc.StructureByteStride = 0;
    //for (size_t i = 0; i < ARRAYSIZE(m_pConstanrBufferBonesData->Data.BonesMatrices); i++)
    //{
    //    m_pConstanrBufferBonesData->Data.BonesMatrices[i] = DirectX::XMFLOAT4X4(
    //        1.0f, 0.0f, 0.0f, 0.0f,
    //        0.0f, 1.0f, 0.0f, 0.0f,
    //        0.0f, 0.0f, 1.0f, 0.0f,
    //        0.0f, 0.0f, 0.0f, 1.0f
    //    );
    //}

    //// 定数バッファ作成
    //hr = device->CreateBuffer(&desc, NULL, &m_pConstanrBufferBonesData->pBuff);
    //if (FAILED(hr))return false;

    return true;
}

//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】マテリアル用定数バッファの作成
// 
//* 引数：1.RendererEngine 
//* 返値：bool
//*----------------------------------------------------------------------------------------
bool ModelData::CreateMaterialCBuffer(RendererEngine &renderer)
{
    //auto pDevice = renderer.get_Device();

    //// 定数バッファの設定
    //D3D11_BUFFER_DESC bd{};
    //ZeroMemory(&bd, sizeof(bd));
    //bd.Usage = D3D11_USAGE_DEFAULT;						// 標準設定
    //bd.ByteWidth = sizeof(CB_MATERIAL);				    // バッファのサイズ
    //bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;			// 定数バッファとして使う
    //bd.CPUAccessFlags = 0;								// CPUから書き込みしない
    //bd.MiscFlags = 0;
    //bd.StructureByteStride = 0;

    //// マテリアル情報定数バッファの生成
    //m_pCBMaterialDataSet = new CB_MATERIAL_SET;
    //if (m_pCBMaterialDataSet == nullptr) {
    //    return false;
    //}

    //// 定数バッファの生成
    //HRESULT hr = pDevice->CreateBuffer(&bd, nullptr, &m_pCBMaterialDataSet->pBuff);
    //if (FAILED(hr)) {
    //    return false;
    //}

    return true;
}

//*---------------------------------------------------------------------------------------
//* @:ModelData Class 
//*【?】ワールド変換用定数バッファの作成
// 
//* 引数：1.RendererEngine 
//* 返値：bool
//*----------------------------------------------------------------------------------------
bool ModelData::CreateTransformCBuffer(RendererEngine &renderer)
{
    //auto pDevice = renderer.get_Device();

    //// ワールド変換定数バッファの生成
    //m_pCBTransformSet = new CB_TRANSFORM_SET;
    //if (m_pCBTransformSet == nullptr) {
    //    return false;
    //}

    //// 定数バッファの設定
    //D3D11_BUFFER_DESC bd{};
    //ZeroMemory(&bd, sizeof(bd));
    //bd.Usage = D3D11_USAGE_DEFAULT;						// 標準設定
    //bd.ByteWidth = sizeof(CB_TRANSFORM);				// バッファのサイズ
    //bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;			// 定数バッファとして使う
    //bd.CPUAccessFlags = 0;								// CPUから書き込みしない
    //bd.MiscFlags = 0;
    //bd.StructureByteStride = 0;

    //// 定数バッファの生成
    //HRESULT hr = pDevice->CreateBuffer(&bd, nullptr, &m_pCBTransformSet->pBuff);
    //if (FAILED(hr)) {
    //    return false;
    //}

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】衝突判定用メッシュデータのセットアップ
//*
//* [引数]
//* *mesh : Assimpメッシュ
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void ModelData::CollisionMeshSetup(aiMesh* mesh)
{
    // このMeshを追加する前の頂点数
    uint32_t baseVertex =
        static_cast<uint32_t>(m_CollisionVertices.size());

    // 頂点
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        const aiVector3D& pos = mesh->mVertices[i];

        CollisionVertex vertex;
        vertex.position = VEC3(
            pos.x,
            pos.y,
            pos.z
        );

        m_CollisionVertices.push_back(vertex);
    }

    // インデックス
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];

        if (face.mNumIndices != 3)
            continue;

        m_CollisionIndices.push_back(baseVertex + face.mIndices[0]);
        m_CollisionIndices.push_back(baseVertex + face.mIndices[1]);
        m_CollisionIndices.push_back(baseVertex + face.mIndices[2]);
    }
}
