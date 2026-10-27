#pragma once
//--------------------------------------------------------------------------------------
//      * Includes *
//--------------------------------------------------------------------------------------
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <queue>
#include "GameObject.h"
#include "IComponent.h"
#include "ModelData.h"



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Component_SkinnedMeshAnimator Class --- */
//
//  ★継承：IComponent ★
// 
// 【?】スキンメッシュアニメーション用コンポーネント
//
// ***************************************************************************************
class SkinnedMeshAnimator : public IComponent
{
private:
	std::weak_ptr<class ModelMeshResource> m_pMeshResource;	// メッシュ情報を持つ

	//CB_BONES_DATA_SET *m_ConstanrBufferBonesData;			// ボーン変換用定数バッファ

	/***********アニメーション関連***********/
	std::vector<BoneInfo> m_BoneList;				     // ボーンリスト
	std::vector<NodeInfo *> m_NodeList;					 // Nodeツリー情報
	std::unordered_map<std::string, uint32_t> m_BoneIndexMap; // Node名からBoneListのインデックスを引くためのマップ
	std::vector<AnimationData*> m_Animations;			 // 読み取ったアニメーション

	double m_AnimationTime = 0.0;						 // 現在のアニメーション時間(ticks)
	int m_PrevAnimIndex = -1;							 // 一つ前のアニメーションインデックス
	int m_CurrentAnimIndex = -1;						 // 現在のアニメーションインデックス
	bool m_IsAnimationFlag;								 // アニメーションさせるかどうか（とりまデバッグ用）
	float m_AnimProcTime;								 // 更新用カウンタ
	float m_ShadowAnimProcTime;							 // シャドウ用更新カウンタ
	bool m_IsPlaybackPaused = false;
	float m_PlaybackSpeed = 1.0f;
	float m_AimPitchAngle;
	float m_AimYawAngle;


	// --- アニメーションブレンド用 ---
	bool  m_IsBlending = false;           // ブレンド中かどうかのフラグ
	float m_BlendTime = 0.0f;             // ブレンド経過時間
	float m_BlendDuration = 0.2f;         // ブレンド完了までの総時間（例: 0.2秒）
	float m_PrevAnimProcTime = 0.0f;      // 一つ前のアニメーションの再生時間

	bool m_IsAnimationLODEnabled;		// アニメーションLODを有効にするか
	int m_AnimationLODFrameCounter;

public:
	SkinnedMeshAnimator(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~SkinnedMeshAnimator();

	void Start(RendererEngine& renderer)override;
	void Update(RendererEngine& renderer)override;
	void Draw(RendererEngine& renderer)override;

	void PlayAnim(float _time);	// アニメーションの再生
	void ChangeAnim(int newAnimIndex, float blendDurationInSeconds = 0.0f);
	void SeekAnim(float timeInSeconds); // Cancel blending and seek within the looping clip.
	void set_PlaybackSpeed(float speed);
	void set_IsPlaybackPaused(bool paused) { m_IsPlaybackPaused = paused; }
	float get_PlaybackSpeed()const { return m_PlaybackSpeed; }
	bool get_IsPlaybackPaused()const { return m_IsPlaybackPaused; }
	bool get_IsBlending()const { return m_IsBlending; }
	float get_AnimDurationSeconds(int animIndex)const;

	void BoneTransformsUpdate(RendererEngine &renderer, float timeInSeconds, int animIdx);	// ボーンの更新
	void set_MeshResource(std::weak_ptr<class ModelMeshResource> meshResource);	// リソースの設定

	void set_AnimTime(float t) { m_AnimationTime = t; };				// アニメーション再生時間の設定
	void set_AnimProcTime(float t) { m_AnimProcTime = t; };				// アニメーション更新用カウンタの設定
	void set_ShadowAnimProcTime(float t) { m_ShadowAnimProcTime = t; };	// アニメーション更新用カウンタの設定
	void set_AnimIndex(int idx) { m_CurrentAnimIndex = idx; };			// アニメーションインデックスの設定
	void set_PrevAnimIndex(int idx) { m_PrevAnimIndex = idx; };			// 前のアニメーションインデックスとして設定
	void set_IsAnim(bool f) { m_IsAnimationFlag = f; };					// アニメーションフラグ
	void set_IsAnimationLODEnabled(bool f) { m_IsAnimationLODEnabled = f; };// アニメーションLODを有効にするか

	float get_AnimProcTime()const { return m_AnimProcTime; }
	float get_ShadowAnimProcTime()const { return m_ShadowAnimProcTime; }
	int getAnimIndex()const { return m_CurrentAnimIndex; }
	int get_PrevAnimIndex()const { return m_PrevAnimIndex; }
	bool get_IsAnim()const { return m_IsAnimationFlag; };
	bool get_IsAnimationLODEnabled()const { return m_IsAnimationLODEnabled; };
	bool get_IsCrntAnimationHasFinished()const;		// アニメーションが終了しているか
	

	DirectX::XMMATRIX& get_BoneLocalWorldMatrix(const std::string& _nodeName);	// ノード名からボーンのワールド変換行列を取得する

	std::vector<AnimationData *> get_AnimationDataList()const { return m_Animations; }

	void CrossFadeAnim(int newAnimIndex, float blendDurationInSeconds);
	void RefreshPose(RendererEngine& renderer);	// ボーン行列を計算する
private:
	void TransformBone(float animTimeTicks, UINT nodeIdx, const DirectX::XMMATRIX &parent, int animIdx); // ボーン変換行列の更新
	const NodeAnimChannel *FindNodeAnim(const AnimationData *pAnim, const std::string &nodeName);	// アニメーションがあるか確認
	UINT FindPosition(float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel);		// 対応するPosキーのインデックスを探す
	UINT FindRotation(float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel);		// 対応するRotキーのインデックスを探す
	UINT FindScaling(float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel);			// 対応するSclキーのインデックスを探す
	void CalcInterpolatedPosition(DirectX::XMMATRIX &out, float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel);		// Pos補間
	void CalcInterpolatedRotation(DirectX::XMMATRIX &out, float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel);		// Rot補間
	void CalcInterpolatedScaling(DirectX::XMMATRIX &out, float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel);			// Scl補間


	DirectX::XMVECTOR CalcInterpolatedScalingVec(float animTimeTicks, const NodeAnimChannel* pNodeAnim);
	DirectX::XMVECTOR CalcInterpolatedRotationQuat(float animTimeTicks, const NodeAnimChannel* pNodeAnim);
	DirectX::XMVECTOR CalcInterpolatedPositionVec(float animTimeTicks, const NodeAnimChannel* pNodeAnim);
};

