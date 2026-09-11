#pragma once
#include "IComponent.h"

/// <summary>
/// 軌跡に必要な位置情報
/// </summary>
struct TrailInfo
{
	VECTOR3::VEC3 _pos;	// 位置
	float _time;		// 時間で徐々に縮めていく

	TrailInfo() :
		_pos(VECTOR3::VEC3()),
		_time(0.0f)
	{};
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:TrailRenderer Class --- */
//
//  ★継承：IComponent ★
//
// 【?】軌跡を描画する
//	参考サイト：https://qiita.com/himego/items/c043ca5d58912b9a58d2#4%E3%81%BE%E3%81%A8%E3%82%81
// 
// ***************************************************************************************
class TrailRenderer : public IComponent
{
private:
	std::deque<TrailInfo> m_TrailInfoList; // 軌跡情報リスト
	VECTOR3::VEC3 m_CrntTrailPos;	// 現在の軌跡位置
	VECTOR4::VEC4 m_Color;			// カラー（エミッシブも一緒に）
	float m_EmissivePower;			// 発光の強さ
	float m_Width;					// 幅
	float m_MinVertexDistance;		// 頂点同士の距離（大きいほどカクカク）
	float m_DrawTime;					// 表示時間
	const UINT MAX_TRAIL_VERTEX_NUM = 256;	// 最大長点数
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertesBuffer;// 頂点バッファ
	//bool m_IsView;								// カメラに向くようにするか（ビルボード）
	CB_MATERIAL_SET *m_pCBMaterialDataSet;			// 定数バッファ(マテリアル用)
	std::shared_ptr<class Texture> m_pTex;
	bool m_IsPosRand;				// 位置にランダム性を持たせるか
	VECTOR3::VEC3 m_PosRandVec;		// 位置のランダム値（最小 最大）


public:
	TrailRenderer(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~TrailRenderer();


	void Start(RendererEngine &renderer) override;		// 初期化
	void LateUpdate(RendererEngine &renderer) override;		// 更新処理
	void Draw(RendererEngine &renderer)override;		// 描画処理

	void set_Width(const float _w) { m_Width = _w; };	// 幅の設定
	float get_Width()const { return m_Width; };			// 幅の取得
	void set_MinVertexDistance(const float _dist) { m_MinVertexDistance = _dist; }	// 頂点同士の距離の設定
	float get_MinVertexDistance()const { return m_MinVertexDistance; }				// 頂点同士の距離の取得
	void set_DrawTime(const float _t) { m_DrawTime = _t; }	// 表示時間の設定
	float get_DrawTime()const { return m_DrawTime; }		// 表示時間の取得
	void set_EmissivePower(const float _power) { m_EmissivePower = _power; }		// エミッシブ設定
	float get_EmissivePower()const { return m_EmissivePower; }						// エミッシブ取得

	void set_Color(const VECTOR4::VEC4& _col) { m_Color = _col; }			//カラー設定
	VECTOR4::VEC4 get_Color()const { return m_Color; }						//カラー取得

	void set_PosRandVec(const VECTOR3::VEC3 &_v) { m_PosRandVec = _v; m_IsPosRand = true; }		// ランダム性を持たせる場合の、ランダム位置座標の最大、最小の設定
	VECTOR3::VEC3 get_PosRandVec()const { return m_PosRandVec; }								// ランダム性を持たせる場合の、ランダム位置座標の最大、最小の取得

	void clear_TrailInfoList() { m_TrailInfoList.clear(); }	// 軌跡情報をクリア

private:
	bool Setup(RendererEngine& renderer);
	bool CreateConstantBuffer(RendererEngine &renderer);
	bool CreateVertexBuffer(RendererEngine &renderer);
	void set_TrailPos(VECTOR3::VEC3 _head,VECTOR3::VEC3 _tail);

	void VertexUpdate(RendererEngine& renderer);
	void ConstantBufferUpdate(RendererEngine &renderer);

	/// <summary>
	/// 軌跡をカメラ方向に向かせる
	/// </summary>
	/// <param name="renderer"></param>
	void CalcTrailSegmentToViewDirection(RendererEngine &renderer);

	/// <summary>
	/// 向き固定ver
	/// </summary>
	/// <param name="renderer"></param>
	void CalcTrailSegment(RendererEngine& renderer);

};

