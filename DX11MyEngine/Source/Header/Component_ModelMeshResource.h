#pragma once
#include "IComponent.h"
#include "ModelData.h"



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ModelMeshResource Class --- */
//
//  ★継承：IComponent ★
// 
// 【?】モデルメッシュ情報のコンポーネント
//
// ***************************************************************************************
class ModelMeshResource : public IComponent
{
private:
    std::array<LODModelData, MAX_REGISTER_LOD> m_LODModels;

public:
    ModelMeshResource(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
    ~ModelMeshResource();

    // モデルデータの取得
    // 引数はLODを設定している場合に使う
    const std::weak_ptr<class ModelData> get_ModelData(LOD_QUALITY _lodIndex = LOD_QUALITY::ULTRA)const
    { 
        return m_LODModels[INT_CAST(_lodIndex)]._pModelData;
    }
    // モデルデータの設定
    // LODを使用しないなら、配列の0番目固定で入れる
    void set_ModelData(std::shared_ptr<class ModelData> pData)
    { 
        m_LODModels[0]._pModelData = pData; 
    };


    // 出力データ
    struct OutLODModelData  {
        std::shared_ptr<ModelData> _pModelData;
        LOD_QUALITY _quality;
    };

    // LODモデルデータの取得
    // 距離から取得するデータを決定
    OutLODModelData get_LODModelData(float _distanceSq)
    {
        for (int i = static_cast<int>(m_LODModels.size()) - 1; i >= 0; --i)
        {
            float lodDist = m_LODModels[i]._distance;
            if (_distanceSq >= lodDist * lodDist)
            {
                // モデルデータがなければ、飛ばす
                if (m_LODModels[i]._pModelData.expired())
                {
                    continue;
                }

                return 
                { 
                    m_LODModels[i]._pModelData.lock() ,
                    static_cast<LOD_QUALITY>(i)
                };
            }
        }

        return {};
    }

    // arrayで取得
    const std::array<LODModelData, MAX_REGISTER_LOD>& get_LODModelDataArray()const
    {
        return m_LODModels;
    }
    // LODを設定する場合はこっち
    void set_LODModelData(const LODModelData& _LODData, LOD_QUALITY _quality)
    {
        m_LODModels[INT_CAST(_quality)] = _LODData;
    };
    // 配列丸ごと入れる
    void set_LODModelData(std::array<LODModelData, MAX_REGISTER_LOD> _arrayData)
    {
        m_LODModels = _arrayData;
    };

};

