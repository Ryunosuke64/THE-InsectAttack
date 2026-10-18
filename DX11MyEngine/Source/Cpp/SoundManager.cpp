#include "pch.h"
#include "SoundManager.h"
#include "RendererEngine.h"
#include "Component_3DCamera.h"
#include <windows.h>
#include <cmath>
#include <limits>

using namespace VECTOR3;

constexpr int NUM_SE_CHANNELS    = 1;	    // SEのチャンネル数（モノラル）
constexpr int NUM_VOICE_CHANNELS = 1;	    // ボイスのチャンネル数（モノラル）
constexpr int NUM_BGM_CHANNELS   = 2;	    // BGMのチャンネル数（ステレオ）
constexpr int SAMPLE_RATE		 = 44100;	// サンプル数 44.1kHz
constexpr int BITS_PER_SAMPLE	 = 16;		// サンプルあたりのビット数


constexpr float DEFAULT_SE_VOL    = 0.5f;	// SEのデフォルトの音量
constexpr float DEFAULT_BGM_VOL   = 0.3f;	// BGMのデフォルトの音量
constexpr float DEFAULT_VOICE_VOL = 0.7f;	// ボイスのデフォルトの音量


// 距離減衰のプリセット（直線的な減衰）
// Point(距離, 音量)
// 距離：0.0 音量 = 1.0
// 距離：1.0 音量 = 0.0
//															  距離  音量  距離  音量
static X3DAUDIO_DISTANCE_CURVE_POINT LinearCurvePoints[2] = { 0.0f, 1.0f, 1.0f, 0.0f }; 
static X3DAUDIO_DISTANCE_CURVE LinearCurve = { (X3DAUDIO_DISTANCE_CURVE_POINT *)&LinearCurvePoints[0], 2 };


//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
SoundManager::SoundManager():
	m_pXAudio2(nullptr),
	m_pMasteringVoice(nullptr),
	m_SEVolume(DEFAULT_SE_VOL),
	m_BGMVolume(DEFAULT_BGM_VOL),
	m_VoiceVolume(DEFAULT_VOICE_VOL),
	m_Listener(),
    m_DeviceDetails(),
    m_DSPSettings(),
	m_X3DInstanceHandle(),
	m_BGM_SoundSlot()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
SoundManager::~SoundManager()
{
    UninitXA2Sound();
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
bool SoundManager::InitXA2Sound(void)
{
    if (m_pXAudio2) return true; // 初期化済みのIDとリソースを維持する。
	HRESULT hr = S_OK;

	// ############################################################################

	//							XAudio2関連の初期化
	
	// ############################################################################

	//XAudio2デバイス作成------------------------------------------
	hr = XAudio2Create(&m_pXAudio2, XAUDIO2_DEBUG_ENGINE);
	if (FAILED(hr)) {
		UninitXA2Sound();
		return false;
	}

	
	//マスターボイスの作成------------------------------------------
	hr = m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);
	if (FAILED(hr)) {
		UninitXA2Sound();
		return false;
	}

	// ※ SEと声でプールは分けている。

	// **************************************************************************
	// ソースボイスの作成（SE用）
	// **************************************************************************
	
	for (int i = 0; i < NUM_SE_SVPOOL; i++)
	{
		SoundInstance sound;
		WAVEFORMATEX wf = {};
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = NUM_SE_CHANNELS;	     // 1（モノラル）
		wf.nSamplesPerSec = SAMPLE_RATE;     // 44.1kHz
		wf.wBitsPerSample = BITS_PER_SAMPLE; // 16bit
		wf.cbSize = 0;

		// 1ブロックあたりのバイト数 = (チャンネル数) * (1サンプルあたりのバイト数)
		wf.nBlockAlign = wf.nChannels * (wf.wBitsPerSample / 8);

		// 1秒あたりの平均バイト数 = (サンプリングレート) * (ブロックサイズ)
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

		//ソースボイス作成
		if (m_pXAudio2->CreateSourceVoice(&sound._pSourceVoice, &wf) != S_OK){
			UninitXA2Sound();
			return false;
		}

		// SE
		m_SoundSlotArray.at(i) = sound;
	}


	// **************************************************************************
	// ソースボイスの作成（声用）
	// **************************************************************************
	
	for (int i = 0; i < NUM_VOICE_SVPOOL; i++)
	{
		SoundInstance sound;
		WAVEFORMATEX wf = {};
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = NUM_VOICE_CHANNELS;	 // 1（モノラル）
		wf.nSamplesPerSec = SAMPLE_RATE;     // 44.1kHz
		wf.wBitsPerSample = BITS_PER_SAMPLE; // 16bit
		wf.cbSize = 0;

		// 1ブロックあたりのバイト数 = (チャンネル数) * (1サンプルあたりのバイト数)
		wf.nBlockAlign = wf.nChannels * (wf.wBitsPerSample / 8);

		// 1秒あたりの平均バイト数 = (サンプリングレート) * (ブロックサイズ)
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

		//ソースボイス作成
		if (m_pXAudio2->CreateSourceVoice(&sound._pSourceVoice, &wf) != S_OK) {
			UninitXA2Sound();
			return false;
		}
		// ボイス
		m_VoiceSoundSlotArray.at(i) = sound;
	}

	// **************************************************************************
	// ソースボイスの作成（BGM用）
	// **************************************************************************
	SoundInstance sound;
	WAVEFORMATEX wf = {};
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = NUM_BGM_CHANNELS;	 // 2（ステレオ）
	wf.nSamplesPerSec = SAMPLE_RATE;     // 44.1kHz
	wf.wBitsPerSample = BITS_PER_SAMPLE; // 16bit
	wf.cbSize = 0;
	// 1ブロックあたりのバイト数 = (チャンネル数) * (1サンプルあたりのバイト数)
	wf.nBlockAlign = wf.nChannels * (wf.wBitsPerSample / 8);

	// 1秒あたりの平均バイト数 = (サンプリングレート) * (ブロックサイズ)
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

	//ソースボイス作成
	if (m_pXAudio2->CreateSourceVoice(&m_BGM_SoundSlot._pSourceVoice, &wf) != S_OK) {
		UninitXA2Sound();
		return false;
	}


	// **************************************************************************
	// ソースボイスの作成（3D用）
	// **************************************************************************
	
	for (int i = 0; i < NUM_SOUND_3D_SVPOOL; i++)
	{
		SoundInstance sound;
		WAVEFORMATEX wf = {};
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = NUM_SE_CHANNELS;	     // 1（モノラル）
		wf.nSamplesPerSec = SAMPLE_RATE;     // 44.1kHz
		wf.wBitsPerSample = BITS_PER_SAMPLE; // 16bit
		wf.cbSize = 0;

		// 1ブロックあたりのバイト数 = (チャンネル数) * (1サンプルあたりのバイト数)
		wf.nBlockAlign = wf.nChannels * (wf.wBitsPerSample / 8);

		// 1秒あたりの平均バイト数 = (サンプリングレート) * (ブロックサイズ)
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

		//ソースボイス作成
		if (m_pXAudio2->CreateSourceVoice(&sound._pSourceVoice, &wf) != S_OK) {
			UninitXA2Sound();
			return false;
		}

		m_3DSoundSlotArray.at(i) = sound;
	}

	// ############################################################################

	//							X3DAudioの初期化

	// ############################################################################

	// 出力形式のチャンネルビットマスクを取得する（最終的な出力先のスピーカーがどういう構成になっているかを取得しているっぽい？）
	DWORD dwChannelMask;
	hr = m_pMasteringVoice->GetChannelMask(&dwChannelMask);
	if (FAILED(hr)) {
		UninitXA2Sound();
		return false;
	}


	// X3Dインスタンスハンドル
	hr = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_X3DInstanceHandle);
	if (FAILED(hr)) {
		UninitXA2Sound();
		return false;
	}

	ZeroMemory(&m_Listener, sizeof(&m_Listener));

	m_Listener.Position.x = 0.0f;
	m_Listener.Position.y = 0.0f;
	m_Listener.Position.z = 0.0f;

	m_Listener.OrientFront.x = 0.0f;
	m_Listener.OrientFront.y = 0.0f;
	m_Listener.OrientFront.z = 1.0f;

	m_Listener.OrientTop.x = 0.0f;
	m_Listener.OrientTop.y = 1.0f;
	m_Listener.OrientTop.z = 0.0f;


	// DSP設定に使用する音声の詳細情報の取得
	m_pMasteringVoice->GetVoiceDetails(&m_DeviceDetails);


	// X3DAudioCalculateからの処理結果を受け取るためのもの！
	// DSP:音の加工に使うもの（？）
	FLOAT32* matrix = new FLOAT32[m_DeviceDetails.InputChannels];
	m_DSPSettings.SrcChannelCount = 1;
	m_DSPSettings.DstChannelCount = m_DeviceDetails.InputChannels;
	m_DSPSettings.pMatrixCoefficients = matrix;
    m_DSPSettings.pDelayTimes = nullptr;
	
	// ############################################################################

	//							音声のロード
	//			（使用するすべての音声をここでロードしてしまう）
	
	// ############################################################################

    // 組み込み音源の一覧。キーは外部から使う名前なので、登録順と切り離して管理する。
    struct SoundDefinition
    {
        std::string_view key;
        const char* filename;
        SOUND_TYPE type;
        int existingId;
    };
    const SoundDefinition definitions[] = {
        // システム
        { "ui.cursor", "Resource/Sound/SE/System/MovingCursor_03.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_MOVING_CURSOR01) },
        { "ui.decision_01", "Resource/Sound/SE/System/Decision_01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_DECISION01) },
        { "ui.decision_02", "Resource/Sound/SE/System/Decision_02.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_DECISION02) },
        { "ui.back", "Resource/Sound/SE/System/Back_01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_BACK01) },
        // 武器
        { "weapon.gun_fire_01", "Resource/Sound/SE/Weapon/GunFire_01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::GUN_FIRE01) },
        { "weapon.gun_fire_02", "Resource/Sound/SE/Weapon/GunFire_02.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::GUN_FIRE02) },
        { "weapon.gun_fire_03", "Resource/Sound/SE/Weapon/GunFire_03.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::GUN_FIRE03) },
        { "weapon.gun_fire_04", "Resource/Sound/SE/Weapon/GunFire_04.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::GUN_FIRE04) },
        { "weapon.gun_fire_laser_01", "Resource/Sound/SE/Weapon/GunFire_Laser_01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::GUN_FIRE_LASER01) },
        { "weapon.ricochet_01", "Resource/Sound/SE/Weapon/Ricochet_01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ROCOCHET01) },
        { "weapon.ricochet_02", "Resource/Sound/SE/Weapon/Ricochet_02.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ROCOCHET02) },
        { "weapon.explosion_01", "Resource/Sound/SE/Weapon/Explosion01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::EXPLOSION01) },
        { "weapon.change_01", "Resource/Sound/SE/Weapon/GunChange_01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::GUN_CHANGE01) },
        // 建物
        { "building.destruction", "Resource/Sound/SE/Building/Destruction.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::BUILDING_DESTRUCTION) },
        { "building.fall", "Resource/Sound/SE/Building/Fall.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::BUILDING_FALL) },
        // 兵士
        { "soldier.jump", "Resource/Sound/SE/Ranger/パンチの衣擦れ1.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SOLDIER_R_JUMP_IN) },
        { "soldier.land", "Resource/Sound/SE/Ranger/ジャンプの着地.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SOLDIER_R_JUMP_LAND) },
        // 敵
        { "enemy.ant.hit_01", "Resource/Sound/SE/Enemy/DSGNImpt_Impact Smear Short 04_RSCPC_HV.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_HIT01) },
        { "enemy.ant.dead", "Resource/Sound/SE/Enemy/DSGNImpt_Impact High Spark 04_RSCPC_HV.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_DEAD) },
        { "enemy.ant.acid_shoot", "Resource/Sound/SE/Enemy/DSGNImpt_Impact Smear Short 06_RSCPC_HV.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_ACID_SHOOT) },
        { "enemy.octahedron.dead", "Resource/Sound/SE/Enemy/DSGNImpt_Impact Pure Power 02_RSCPC_HV.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_OCTAHEDRON_DEAD) },
        { "enemy.octahedron.shot_01", "Resource/Sound/SE/Enemy/Octh_Fire01.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_OCTAHEDRON_SHOT01) },
        { "enemy.octahedron.shot_02", "Resource/Sound/SE/Enemy/Octh_Fire02.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_OCTAHEDRON_SHOT02) },
        { "enemy.octahedron.always", "Resource/Sound/SE/Enemy/Octh_Always.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_OCTAHEDRON_ALWAYS) },
        // アイテム
        { "item.get", "Resource/Sound/SE/Game/ItemGet.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ITEM_GET) },
        { "item.point_get", "Resource/Sound/SE/Game/PointGet.wav", SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::POINT_GET) },
        // BGM
        { "bgm.title_01", "Resource/Sound/BGM/Flash_Shadow.wav", SOUND_TYPE::BGM, SOUND_ID_TO_INT(BGM_ID::BGM_TITLE_01) },
        { "bgm.game_01", "Resource/Sound/BGM/MusMus-BGM-170.wav", SOUND_TYPE::BGM, SOUND_ID_TO_INT(BGM_ID::BGM_GAME_01) },
        { "bgm.victory_01", "Resource/Sound/BGM/Victory.wav", SOUND_TYPE::BGM, SOUND_ID_TO_INT(BGM_ID::BGM_VICTORY01) },
        { "bgm.hangeki", "Resource/Sound/BGM/Hangeki.wav", SOUND_TYPE::BGM, SOUND_ID_TO_INT(BGM_ID::BGM_HANGEKI) },
        // ボイス
        { "voice.shout_01", "Resource/Sound/Voice/Ranger/an000_01.wav", SOUND_TYPE::VOICE, SOUND_ID_TO_INT(VOICE_ID::SOLDIER_R_SHOUT_01) },
        { "voice.shout_02", "Resource/Sound/Voice/Ranger/an000_02.wav", SOUND_TYPE::VOICE, SOUND_ID_TO_INT(VOICE_ID::SOLDIER_R_SHOUT_02) },
        { "voice.shout_03", "Resource/Sound/Voice/Ranger/an000_03.wav", SOUND_TYPE::VOICE, SOUND_ID_TO_INT(VOICE_ID::SOLDIER_R_SHOUT_03) },

    };
    for (const auto& definition : definitions)
    {
        if (!Load_Wav(definition.filename, definition.type, definition.existingId)
            || !IsValid(RegisterSoundName(definition.key, definition.type, definition.existingId)))
        {
            UninitXA2Sound();
            return false;
        }
    }

	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】解放
//* ※ 作成した順番と逆に解放
//*
//* [引数]なし
//* [返値]なし
//*----------------------------------------------------------------------------------------
void SoundManager::UninitXA2Sound(void)
{
    auto destroy = [](SoundInstance& slot)
    {
        if (slot._pSourceVoice)
            slot._pSourceVoice->DestroyVoice();
        slot = SoundInstance{};
    };
    for (auto& slot : m_SoundSlotArray) destroy(slot);
    for (auto& slot : m_VoiceSoundSlotArray) destroy(slot);
    for (auto& slot : m_3DSoundSlotArray) destroy(slot);
    destroy(m_BGM_SoundSlot);

    if (m_pMasteringVoice) m_pMasteringVoice->DestroyVoice();
    m_pMasteringVoice = nullptr;
    if (m_pXAudio2) m_pXAudio2->Release();
    m_pXAudio2 = nullptr;
    delete[] m_DSPSettings.pMatrixCoefficients;
    m_DSPSettings = {};

    m_SoundNameToId.clear();
    m_SoundEntries.clear();
    m_SE_WaveResourceMap.clear();
    m_Voice_WaveResourceMap.clear();
    m_BGM_WaveResourceMap.clear();
    ++m_SoundGeneration;
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool SoundManager::Update(RendererEngine& renderer)
{
    if (!m_pXAudio2) return false;
	// ############################################################################
	//							リスナーの更新
	// ############################################################################
	// カメラ情報
	VECTOR3::VEC3 cameraPos;
	VECTOR3::VEC3 cameraUp;
	VECTOR3::VEC3 cameraLook;
	if (auto cameraComp = Master::m_pDataManager->get_CameraComponent().lock())
	{
		cameraPos = cameraComp->get_CameraPos();
		cameraUp = cameraComp->get_UpVec();
		cameraLook = cameraComp->get_LookDir();
	}

    // カメラの位置と向きからリスナーの情報を更新
    m_Listener.Position.x = cameraPos.x;
    m_Listener.Position.y = cameraPos.y;
    m_Listener.Position.z = cameraPos.z;

    m_Listener.OrientFront.x = cameraLook.x;
    m_Listener.OrientFront.y = cameraLook.y;
    m_Listener.OrientFront.z = cameraLook.z;

    m_Listener.OrientTop.x = cameraUp.x;
    m_Listener.OrientTop.y = cameraUp.y;
    m_Listener.OrientTop.z = cameraUp.z;

    m_Listener.pCone = nullptr;	// コーンは使用しない

	// 今回はリスナーは動かない想定なので速度は0
    m_Listener.Velocity.x = 0.0f;
    m_Listener.Velocity.y = 0.0f;
    m_Listener.Velocity.z = 0.0f;

	// BGMの音量を更新
	if (m_BGM_SoundSlot._pSourceVoice) {
		m_BGM_SoundSlot._pSourceVoice->SetVolume(m_BGMVolume);
	}

    return Update3DSounds();
}


//*---------------------------------------------------------------------------------------
//*【?】SE用WAVファイルの読み込み
//* 
//* [注意]
//* ・対応フォーマット
//* 　拡張子：wav 
//* 　チャンネル数：1（モノラル）
//* 　サンプル数：44100（44.1kHz）
//* 　サンプルビット数：16
//* 
//* [引数]
//* *filename：読み込むファイル名 
//* _id：対応するSEのID
//* 
//* [返値]
//* true:成功
//* false:失敗
//*----------------------------------------------------------------------------------------
bool SoundManager::Load_Wav(const char* filename, SOUND_TYPE type, int existingId)
{
    if (!filename || !*filename || existingId < 0)
        return false;

    auto load = [&](auto& resources, auto id, int channels)
    {
        auto [it, inserted] = resources.try_emplace(id);
        if (!inserted)
            return false;
        if (!Internal_Load_Wav(filename, it->second, channels) || it->second._size == 0)
        {
            resources.erase(it);
            return false;
        }
        return true;
    };

    switch (type)
    {
    case SOUND_TYPE::SE: return load(m_SE_WaveResourceMap, static_cast<SOUND_ID>(existingId), 1);
    case SOUND_TYPE::VOICE: return load(m_Voice_WaveResourceMap, static_cast<VOICE_ID>(existingId), 1);
    case SOUND_TYPE::BGM: return load(m_BGM_WaveResourceMap, static_cast<BGM_ID>(existingId), 2);
    default: return false;
    }
}


//*---------------------------------------------------------------------------------------
//*【?】BGMの再生
//*
//* [引数]
//* _id：再生するサウンドのID
//* _loop：ループするかどうか
//* 
//* [返値] 
// true:成功
// false:失敗
//*----------------------------------------------------------------------------------------
bool SoundManager::Internal_PlayBGM(BGM_ID _id, bool _loop)
{
	// 見つからなければそのまま返す
	auto it = m_BGM_WaveResourceMap.find(_id);
	if (it == m_BGM_WaveResourceMap.end() || !m_BGM_SoundSlot._pSourceVoice) {
		return false;
	}

	// BGMは同時に1曲。切り替え前にキューを空にする。
    if (!StopInstance(m_BGM_SoundSlot)) return false;
    auto& resource = it->second;
	HRESULT hr = S_OK;

	// 波形データ
	XAUDIO2_BUFFER buff = {};
	buff.pAudioData = resource._waveBuffer;		// 波形データの先頭アドレス
	buff.AudioBytes = resource._size;			// 波形データのサイズ
	buff.Flags = XAUDIO2_END_OF_STREAM;
	buff.LoopCount = _loop ? XAUDIO2_LOOP_INFINITE : 0;	  // ループさせるか


	// 音量
	hr = m_BGM_SoundSlot._pSourceVoice->SetVolume(m_BGMVolume);
	if (FAILED(hr)) return false;

	// 音声バッファの追加
	hr = m_BGM_SoundSlot._pSourceVoice->SubmitSourceBuffer(&buff);
	if (FAILED(hr)) return false;

	// 音を鳴らす
	hr = m_BGM_SoundSlot._pSourceVoice->Start();
	if (FAILED(hr))
	{
		StopInstance(m_BGM_SoundSlot);
		return false;
	}

    m_BGM_SoundSlot._soundType = SOUND_TYPE::BGM;
    m_BGM_SoundSlot._soundID = static_cast<int>(_id);
    m_BGM_SoundSlot._isLoop = _loop;
    m_BGM_SoundSlot._isUsed = true;
    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】サウンドの再生（内部関数）
//*
//* [引数]
//* _type：サウンドの種類 (SE、BGM、ボイスなど)
//* _id  ：再生するサウンドのID 
//* _loop：ループするかどうか
//* 
//* [返値] 
// true:成功
// false:失敗
//*---------------------------------------------------------------------------------------
bool SoundManager::Internal_SoundPlay(SOUND_TYPE type, int id, bool loop)
{
    return Internal_SoundPlay_RandPitch(type, id, 0, loop);
}


//*---------------------------------------------------------------------------------------
//*【?】サウンドの再生（内部関数）
//*
//* [引数]
//* _type：サウンドの種類 (SE、BGM、ボイスなど)
//* _id  ：再生するサウンドのID 
//* _loop：ループするかどうか
//* 
//* [返値] 
// true:成功
// false:失敗
//*---------------------------------------------------------------------------------------
bool SoundManager::Internal_SoundPlay_RandPitch(SOUND_TYPE _type, int _id, int _pitchRange, bool _loop)
{
	std::span<SoundInstance> typeSoundSlotArray;
    WaveResource *typeWaveResource = nullptr;

	// サウンドの種類とIDから、再生に必要なリソースを取得
	if (!SoundTypeAndIDConvertToResource(_type, _id, typeWaveResource, typeSoundSlotArray))
        return false;

	HRESULT hr = S_OK;

	// 波形データ
	XAUDIO2_BUFFER buff = {};
	buff.pAudioData = typeWaveResource->_waveBuffer;		// 波形データの先頭アドレス
	buff.AudioBytes = typeWaveResource->_size;			// 波形データのサイズ
	buff.Flags = XAUDIO2_END_OF_STREAM;
	buff.LoopCount = _loop ? XAUDIO2_LOOP_INFINITE : 0;	  // ループさせるか

	// 再生が終了しているソースボイスを探して
	for (auto &slot : typeSoundSlotArray) {
        if (!slot._pSourceVoice) continue;
		XAUDIO2_VOICE_STATE state;
		slot._pSourceVoice->GetState(&state);

		// 待機中のバッファが0個 ＝ 再生が終わっている
		if (state.BuffersQueued == 0)
		{
			// ピッチをいじる
			float pitch = _pitchRange == 0 ? 1.0f
            : 1.0f + ((rand() % _pitchRange - (_pitchRange / 2)) / 1000.0f);
			hr = slot._pSourceVoice->SetFrequencyRatio(pitch);
			if (FAILED(hr)) return false;

			// 音量
			hr = slot._pSourceVoice->SetVolume(get_Volume(_type));
			if (FAILED(hr)) return false;

			// 音声バッファの追加
			hr = slot._pSourceVoice->SubmitSourceBuffer(&buff);
			if (FAILED(hr)) return false;

			// 音を鳴らす
			hr = slot._pSourceVoice->Start();
			if (FAILED(hr))
			{
				StopInstance(slot);
				return false;
			}

            // 停止時に「同じ音源だけ」を見つけられるよう、2D再生にも記録する。
            slot._soundType = _type;
            slot._soundID = _id;
            slot._isLoop = _loop;
            slot._isUsed = true;
            return true;
        }
    }

    return false;
}

//*---------------------------------------------------------------------------------------
//*【?】3Dサウンドの再生（内部関数）
//*		ピッチを指定の幅でランダムにする
//*
//* [引数]
//* _type       ：サウンドの種類 (SE、BGM、ボイスなど)
//* _id         ：再生するサウンドのID
//* _pos        ：再生位置
//* _radius     ：聞こえる範囲
//* _pitchRange ：ピッチの幅
//* _loop       ：ループするかどうか
//*
//* [返値]
// 再生に成功した場合はSoundHandle、失敗した場合は無効ハンドル
//*---------------------------------------------------------------------------------------
SoundHandle SoundManager::Internal_Start3D(SOUND_TYPE _type, int _id, const VECTOR3::VEC3& _pos, float _radius, int _pitchRange, const VECTOR3::VEC3& _vel , bool _loop)
{
	HRESULT hr = S_OK;
	WaveResource* typeWaveResource = nullptr;

	// 音声データの取得
	if (!SerchResource(_type, _id, typeWaveResource))
        return {};

	// 再生が終了しているソースボイスを探す
	for (std::size_t index = 0; index < m_3DSoundSlotArray.size(); ++index) {
        auto& slot = m_3DSoundSlotArray[index];
        if (!slot._pSourceVoice) continue;
		XAUDIO2_VOICE_STATE state;
		slot._pSourceVoice->GetState(&state);

		// 使われているスロットは飛ばす
		if (state.BuffersQueued != 0)continue;
		// Update前でも、自然終了済みなら同じスロットを再利用できる。
        if (!StopInstance(slot)) return {};


		if (typeWaveResource == nullptr) {
			continue;
		}

		slot._pos = _pos;		 // 音を鳴らす位置をセット
		slot._velocity = _vel;	 // 音の移動ベクトルをセット
		slot._isLoop = _loop;	 // ループするかどうかをセット
		slot._soundID = _id;	 // 音のIDをセット
		slot._soundType = _type; // 音の種類をセット
		slot._isUsed = false; // 再生開始に成功してから使用中にする
		slot._volumeDefault = get_Volume(_type);	// 音量のデフォルト値をセット
		slot._volumeFactor = 1.0f;					// 音量の倍率を初期化
		slot._radius = _radius;	 // 音の聞こえる範囲

		// エミッター
		X3DAUDIO_EMITTER emitter = {};
		emitter.ChannelCount = 1;		// 基本的に3Dサウンド用はモノラルなので 1 チャンネル
		emitter.CurveDistanceScaler = _radius;
		emitter.DopplerScaler = 1.0f;
		emitter.ChannelRadius = 0.0f;
		emitter.Position.x = slot._pos.x;
		emitter.Position.y = slot._pos.y;
		emitter.Position.z = slot._pos.z;
		emitter.Velocity.x = slot._velocity.x;
		emitter.Velocity.y = slot._velocity.y;
		emitter.Velocity.z = slot._velocity.z;
		emitter.pVolumeCurve = &LinearCurve;


		// 3Dサウンドにするための計算
		X3DAudioCalculate(m_X3DInstanceHandle, &m_Listener, &emitter,
			X3DAUDIO_CALCULATE_MATRIX,
			&m_DSPSettings);

		// 計算結果をソースボイスに反映
		hr = slot._pSourceVoice->SetOutputMatrix(
			m_pMasteringVoice,
			1,
			m_DeviceDetails.InputChannels,
			m_DSPSettings.pMatrixCoefficients
		);

		if (FAILED(hr)) {
			continue;
		}

		// 波形データ
		XAUDIO2_BUFFER buff = {};
		buff.pAudioData = typeWaveResource->_waveBuffer;	// 波形データの先頭アドレス
		buff.AudioBytes = typeWaveResource->_size;			// 波形データのサイズ
		buff.Flags = XAUDIO2_END_OF_STREAM;
		buff.LoopCount = slot._isLoop ? XAUDIO2_LOOP_INFINITE : 0;	// ループさせるか

		float volume = slot._volumeDefault * slot._volumeFactor;

		// ピッチをいじる
		float pitch = _pitchRange == 0 ? 1.0f
            : 1.0f + ((rand() % _pitchRange - (_pitchRange / 2)) / 1000.0f);
		hr = slot._pSourceVoice->SetFrequencyRatio(pitch);
		if (FAILED(hr)) return {};

		// 音量
		hr = slot._pSourceVoice->SetVolume(volume);
		if (FAILED(hr)) {
			return {};
		}



		// 音声バッファの追加
		hr = slot._pSourceVoice->SubmitSourceBuffer(&buff);
		if (FAILED(hr)) {
			return {};
		}

		// 音を鳴らす
		hr = slot._pSourceVoice->Start();
		if (FAILED(hr)) {
            StopInstance(slot);
            return {};
        }

        slot._isUsed = true;
        auto& generation = m_3DPlaybackGenerations[index];
        ++generation;
        if (generation == 0) ++generation; // 0は無効ハンドル用。
        return { index, generation, m_SoundGeneration };
	}

	return {};
}


//*---------------------------------------------------------------------------------------
//*【?】WAVファイルの読み込み（実際の内部的な読み込み関数）
//* 
//* WAVファイルはRIFF形式という方法で構成されていて、
//* チャンクというデータの塊が複数連結している。
//* チャンクは基本的に「ID」「サイズ」「中身」という感じで要素が入っている
//* waveファイルではRIFFが親チャンクとなり、その子としてfmt、dataなどのチャンクが居るっぽい
//*
//* fmt：形式情報
//* data：実際の波形データ
//* 
//* [注意]
//* ・対応フォーマット
//* 　拡張子：wav 
//* 　チャンネル数：2（ステレオ）
//* 　サンプル数：44100（44.1kHz）
//* 　サンプルビット数：16
//* 
//* [引数]
//* *filename：読み込むファイル名 
//* &_out：波形データの出力先
//* 
//* [返値]
//* true:成功
//* false:失敗
//*----------------------------------------------------------------------------------------
bool SoundManager::Internal_Load_Wav(const char* _filename, WaveResource& _out, int _channelNum, int _sampleLate, int _bitsPersample)
{
	HMMIO mmio_handle = NULL;	// mmioハンドル
	DWORD size;
	BYTE* buffer;
	WAVEFORMATEX wfx = {};		// waveファイルのフォーマット情報
	MMCKINFO ckInfo = {};		// チャンク情報
	MMCKINFO rifckInfo = {};	// RIFFチャンク用
	MMRESULT mmhr;

	char str_buffer[512] = {};
	strcpy_s(str_buffer, _filename);

	//	ファイルを開く
	mmio_handle = mmioOpenA(str_buffer, NULL, MMIO_ALLOCBUF | MMIO_READ);

	if (!mmio_handle) {
		MessageBoxA(NULL, "wavファイルを開けませんでした", "SoundManager", MB_OK);
		return false;
	}

	// ##########################################################################################
	//							RIFF（親）チャンクへ進入
	// ##########################################################################################
	mmhr = mmioDescend(mmio_handle, &rifckInfo, NULL, 0);

	// fccType が 'WAVE' であることを確認
	if (mmhr != MMSYSERR_NOERROR || rifckInfo.fccType != mmioFOURCC('W', 'A', 'V', 'E')) {
		MessageBoxA(NULL, "RIFFチャンクへの進入失敗", "SoundManager", MB_OK);
		mmioClose(mmio_handle, 0);
		return false;
	}


	// ##########################################################################################
	//							fmtチャンクへ進入
	// ##########################################################################################
	ckInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	mmhr = mmioDescend(mmio_handle, &ckInfo, &rifckInfo, MMIO_FINDCHUNK);
	if (mmhr != MMSYSERR_NOERROR) {
		mmioClose(mmio_handle, MMIO_FHOPEN);	// ファイルを閉じる
		MessageBoxA(NULL, "fmtチャンクへの進入失敗", "SoundManager", MB_OK);
		return false;
	}

	DWORD readSize = mmioRead(mmio_handle, (HPSTR)&wfx, sizeof(wfx)/* std::min((DWORD)sizeof(WAVEFORMATEX), ckInfo.cksize)*/);	// 読み込み

	wfx.cbSize = 0;

	// フォーマットチェック
	if (wfx.wFormatTag != WAVE_FORMAT_PCM) {
		MessageBoxA(NULL, "フォーマットエラーです", "SoundManager", MB_OK);
		mmioClose(mmio_handle, MMIO_FHOPEN);	// ファイルを閉じる
		return false;
	}
	// チャンネル数チェック
	if (wfx.nChannels != _channelNum) {
		MessageBoxA(NULL, "フォーマットエラーです", "SoundManager", MB_OK);
		mmioClose(mmio_handle, MMIO_FHOPEN);	// ファイルを閉じる
		return false;
	}
	// サンプル数チェック
	if (wfx.nSamplesPerSec != _sampleLate) {
		MessageBoxA(NULL, "フォーマットエラーです", "SoundManager", MB_OK);
		mmioClose(mmio_handle, MMIO_FHOPEN);	// ファイルを閉じる
		return false;
	}
	// ビット数チェック
	if (wfx.wBitsPerSample != _bitsPersample) {
		MessageBoxA(NULL, "フォーマットエラーです", "SoundManager", MB_OK);
		mmioClose(mmio_handle, MMIO_FHOPEN);	// ファイルを閉じる
		return false;
	}

	mmioAscend(mmio_handle, &ckInfo, 0); // fmtチャンクから退出


	// ##########################################################################################
	//							dataチャンクへ進入
	// ##########################################################################################
	ckInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mmhr = mmioDescend(mmio_handle, &ckInfo, &rifckInfo, MMIO_FINDCHUNK);
	if (mmhr != MMSYSERR_NOERROR) {
		mmioClose(mmio_handle, MMIO_FHOPEN);	// ファイルを閉じる
		MessageBoxA(NULL, "dataチャンクへの進入失敗", "SoundManager", MB_OK);
		return false;
	}

	size = ckInfo.cksize;
	buffer = new BYTE[size];
	readSize = mmioRead(mmio_handle, (HPSTR)buffer, size);
	if (readSize != ckInfo.cksize) {
		delete[] buffer;	// バッファの開放
		// 読み込みサイズが一致していません
		mmioClose(mmio_handle, MMIO_FHOPEN);
		return false;
	}


	// 格納
	_out._size = size;
	_out._waveBuffer = buffer;
	_out._waveFormat = wfx;


	// ファイル閉じる
	mmioClose(mmio_handle, MMIO_FHOPEN);

	buffer = nullptr;

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】サウンドの種類とIDから、再生に必要なリソースとソースボイスを取得する関数
//*
//* [引数]
//* _type：サウンドの種類 (SE、BGM、ボイスなど)
//* _id  ：再生するサウンドのID 
//* *&_outResource : サウンドの波形データなどを持つリソース構造体の出力先
//* &_outInstArray : 再生に使用するソースボイスの配列の出力先 （spanとすることで要素数の異なる配列に対応可）
//* 
//* [返値] 
// true:成功
// false:失敗
//*---------------------------------------------------------------------------------------
bool SoundManager::SoundTypeAndIDConvertToResource(SOUND_TYPE _type, int _id, WaveResource *&_outResource, std::span<SoundInstance> &_outInstArray)
{
	switch (_type)
	{
		// ****************************************************************
		//			SE用
		// ****************************************************************
	case SOUND_TYPE::SE:
	{
		_outInstArray = m_SoundSlotArray;

		auto it = m_SE_WaveResourceMap.find(static_cast<SOUND_ID>(_id));
		if (it == m_SE_WaveResourceMap.end()) {
			return false;
		}
		_outResource = &it->second;
		break;
	}
	// ****************************************************************
	//			BGM用
	// ****************************************************************
	case SOUND_TYPE::BGM:
	{
		return false;	// BGMは専用のスロットとリソースを使うので、ここではfalseを返す
	}
	break;

	// ****************************************************************
	//			Voice用
	// ****************************************************************
	case SOUND_TYPE::VOICE:
	{
		_outInstArray = m_VoiceSoundSlotArray;

		auto it = m_Voice_WaveResourceMap.find(static_cast<VOICE_ID>(_id));
		if (it == m_Voice_WaveResourceMap.end()) {
			return false;
		}
		_outResource = &it->second;
	}
	break;
	default:
		return false;
		break;
	}

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】サウンドの種類とIDから、再生に必要なリソースを取得する関数
//*
//* [引数]
//* _type：サウンドの種類 (SE、BGM、ボイスなど)
//* _id  ：再生するサウンドのID 
//* *&_outResource : サウンドの波形データなどを持つリソース構造体の出力先
//* 
//* [返値] 
// true:成功
// false:失敗
//*---------------------------------------------------------------------------------------
bool SoundManager::SerchResource(SOUND_TYPE _type, int _id, WaveResource *&_outResource)
{
    _outResource = nullptr;
	switch (_type)
	{
	case SOUND_TYPE::SE:
	{
		auto it = m_SE_WaveResourceMap.find(static_cast<SOUND_ID>(_id));
		if (it == m_SE_WaveResourceMap.end())
		{
			return false;
		}
		_outResource = &it->second;
		break;
	}
	case SOUND_TYPE::BGM:
	{
		auto it = m_BGM_WaveResourceMap.find(static_cast<BGM_ID>(_id));
		if (it == m_BGM_WaveResourceMap.end())
		{
			return false;
		}
		_outResource = &it->second;
	}
	break;
	case SOUND_TYPE::VOICE:
	{
		auto it = m_Voice_WaveResourceMap.find(static_cast<VOICE_ID>(_id));
		if (it == m_Voice_WaveResourceMap.end())
		{
			return false;
		}
		_outResource = &it->second;
	}
	break;
	default:
        return false;
	}
	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】指定サウンドタイプの音量を設定
//*
//* [引数]
//* _type：サウンドの種類 (SE、BGM、ボイスなど)
//* _vol : 音量（0.0f～1.0f）
//* 
//* [返値] 
//* float : 音量 
//*---------------------------------------------------------------------------------------
void SoundManager::set_Volume(SOUND_TYPE _type, float _vol)
{
	switch (_type)
	{
	case SOUND_TYPE::SE:
		m_SEVolume = _vol;
		break;
	case SOUND_TYPE::BGM:
		m_BGMVolume = _vol;
		break;
	case SOUND_TYPE::VOICE:
		m_VoiceVolume = _vol;
		break;
	default:
		break;
	}
}


//*---------------------------------------------------------------------------------------
//*【?】指定サウンドタイプの音量を取得
//*
//* [引数]
//* _type：サウンドの種類 (SE、BGM、ボイスなど)
//* 
//* [返値] 
//* float : 音量 
//*---------------------------------------------------------------------------------------
float SoundManager::get_Volume(SOUND_TYPE _type)const
{
	switch (_type)
	{
	case SOUND_TYPE::SE:
		return m_SEVolume;
		break;
	case SOUND_TYPE::BGM:
		return m_BGMVolume;
		break;
	case SOUND_TYPE::VOICE:
		return m_VoiceVolume;
		break;
	default:
		return 0.0f;
		break;
	}
}


// 名前の解決と公開API。実際の再生処理は、既存のボイスプールを利用する。
const SoundManager::SoundEntry* SoundManager::GetSoundEntry(SoundID id) const
{
    if (id.generation != m_SoundGeneration || id.value >= m_SoundEntries.size())
        return nullptr;
    return &m_SoundEntries[id.value];
}

bool SoundManager::IsValid(SoundID id) const
{
    return GetSoundEntry(id) != nullptr;
}

SoundID SoundManager::FindSound(std::string_view key) const
{
    const auto it = m_SoundNameToId.find(key);
    return it != m_SoundNameToId.end() ? it->second : SoundID{};
}

SoundID SoundManager::FindLegacySound(SOUND_TYPE type, int existingId) const
{
    for (std::size_t i = 0; i < m_SoundEntries.size(); ++i)
    {
        const auto& entry = m_SoundEntries[i];
        if (entry.type == type && entry.existingId == existingId)
            return { i, m_SoundGeneration };
    }
    return {};
}

SoundID SoundManager::FindSound(SOUND_ID id) const
{
    return FindLegacySound(SOUND_TYPE::SE, static_cast<int>(id));
}

SoundID SoundManager::FindSound(VOICE_ID id) const
{
    return FindLegacySound(SOUND_TYPE::VOICE, static_cast<int>(id));
}

SoundID SoundManager::FindSound(BGM_ID id) const
{
    return FindLegacySound(SOUND_TYPE::BGM, static_cast<int>(id));
}

SoundID SoundManager::RegisterSoundName(std::string_view key, SOUND_TYPE type, int existingId)
{
    if (key.empty() || m_SoundNameToId.find(key) != m_SoundNameToId.end())
        return {};

    WaveResource* resource = nullptr;
    if (!SerchResource(type, existingId, resource)
        || !resource || !resource->_waveBuffer || resource->_size == 0)
        return {};

    // 同じ音源に別名を付ける場合も、内部IDは共通にする。
    SoundID id = FindLegacySound(type, existingId);
    if (!IsValid(id))
    {
        id = { m_SoundEntries.size(), m_SoundGeneration };
        m_SoundEntries.push_back({ type, existingId });
    }
    m_SoundNameToId.emplace(std::string(key), id);
    return id;
}

SoundID SoundManager::Load_Wav(std::string_view key, const char* filename, SOUND_TYPE type)
{
    if (key.empty() || m_SoundNameToId.find(key) != m_SoundNameToId.end())
        return {};

    // 新規音源の番号は、既存enumが使う範囲より後ろから自動で割り当てる。
    constexpr int firstDynamicId = static_cast<int>(SOUND_ID::NUM)
        + static_cast<int>(VOICE_ID::NUM) + static_cast<int>(BGM_ID::NUM);
    if (m_SoundEntries.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)() - firstDynamicId))
        return {};
    const int existingId = firstDynamicId + static_cast<int>(m_SoundEntries.size());
    if (!Load_Wav(filename, type, existingId))
        return {};
    return RegisterSoundName(key, type, existingId);
}

bool SoundManager::Play(SoundID id, bool loop)
{
    const auto* entry = GetSoundEntry(id);
    if (!entry) return false;
    if (entry->type == SOUND_TYPE::BGM)
        return Internal_PlayBGM(static_cast<BGM_ID>(entry->existingId), loop);
    return Internal_SoundPlay(entry->type, entry->existingId, loop);
}

bool SoundManager::Play(std::string_view key, bool loop)
{
    return Play(FindSound(key), loop);
}

bool SoundManager::Play_3D(SoundID id, const VECTOR3::VEC3& pos, float radius,
    const VECTOR3::VEC3& velocity, bool loop)
{
    return Internal_Play3D(id, pos, radius, 0, velocity, loop).playbackGeneration != 0;
}

bool SoundManager::Play_3D(std::string_view key, const VECTOR3::VEC3& pos, float radius,
    const VECTOR3::VEC3& velocity, bool loop)
{
    return Play_3D(FindSound(key), pos, radius, velocity, loop);
}

bool SoundManager::Play_RandPitch(SoundID id, int pitchRange, bool loop)
{
    const auto* entry = GetSoundEntry(id);
    if (!entry || entry->type == SOUND_TYPE::BGM || pitchRange < 0 || pitchRange > 1998)
        return false;
    return Internal_SoundPlay_RandPitch(entry->type, entry->existingId, pitchRange, loop);
}

bool SoundManager::Play_RandPitch(std::string_view key, int pitchRange, bool loop)
{
    return Play_RandPitch(FindSound(key), pitchRange, loop);
}

bool SoundManager::Play_RandPitch_3D(SoundID id, const VECTOR3::VEC3& pos, float radius,
    int pitchRange, const VECTOR3::VEC3& velocity, bool loop)
{
    return Internal_Play3D(id, pos, radius, pitchRange, velocity, loop).playbackGeneration != 0;
}

SoundHandle SoundManager::Internal_Play3D(SoundID id, const VECTOR3::VEC3& pos, float radius,
    int pitchRange, const VECTOR3::VEC3& velocity, bool loop)
{
    const auto* entry = GetSoundEntry(id);
    // 3Dプールはモノラル専用。BGMや不正な座標・半径をX3DAudioへ渡さない。
    if (!entry || entry->type == SOUND_TYPE::BGM || !IsFiniteVector(pos) || !IsFiniteVector(velocity)
        || !std::isfinite(radius) || radius <= 0.0f || pitchRange < 0 || pitchRange > 1998)
        return {};
    if (!m_pMasteringVoice || !m_DSPSettings.pMatrixCoefficients)
        return {};
    return Internal_Start3D(entry->type, entry->existingId, pos, radius, pitchRange, velocity, loop);
}

bool SoundManager::Play_RandPitch_3D(std::string_view key, const VECTOR3::VEC3& pos, float radius,
    int pitchRange, const VECTOR3::VEC3& velocity, bool loop)
{
    return Play_RandPitch_3D(FindSound(key), pos, radius, pitchRange, velocity, loop);
}

bool SoundManager::Play_Rand(std::span<const SoundID> candidates, bool loop)
{
    if (candidates.empty()) return false;
    for (SoundID id : candidates)
        if (!IsValid(id)) return false;
    return Play(candidates[static_cast<std::size_t>(rand()) % candidates.size()], loop);
}

bool SoundManager::Play_Rand(std::span<const std::string_view> keys, bool loop)
{
    if (keys.empty()) return false;
    for (std::string_view key : keys)
        if (!IsValid(FindSound(key))) return false;
    return Play(keys[static_cast<std::size_t>(rand()) % keys.size()], loop);
}

bool SoundManager::PlayBGM(SoundID id, bool loop)
{
    const auto* entry = GetSoundEntry(id);
    return entry && entry->type == SOUND_TYPE::BGM && Play(id, loop);
}

bool SoundManager::PlayBGM(std::string_view key, bool loop)
{
    return PlayBGM(FindSound(key), loop);
}

bool SoundManager::StopInstance(SoundInstance& instance)
{
    if (!instance._pSourceVoice) return true;
    if (FAILED(instance._pSourceVoice->Stop(0))) return false;
    if (FAILED(instance._pSourceVoice->FlushSourceBuffers())) return false;
    // ボイス自体は再利用するので残し、それ以外の再生情報を初期化する。
    auto* voice = instance._pSourceVoice;
    instance = SoundInstance{};
    instance._pSourceVoice = voice;
    return true;
}

bool SoundManager::Stop(SoundID id)
{
    const auto* entry = GetSoundEntry(id);
    if (!entry) return false;

    bool success = true;
    auto stopMatching = [&](SoundInstance& slot)
    {
        if (slot._isUsed && slot._soundType == entry->type && slot._soundID == entry->existingId)
            success = StopInstance(slot) && success;
    };
    // 同じ音源を複数回鳴らしていた場合、2D・3Dの両方をすべて停止する。
    for (auto& slot : m_SoundSlotArray) stopMatching(slot);
    for (auto& slot : m_VoiceSoundSlotArray) stopMatching(slot);
    for (auto& slot : m_3DSoundSlotArray) stopMatching(slot);
    stopMatching(m_BGM_SoundSlot);
    return success;
}

bool SoundManager::Stop(std::string_view key)
{
    return Stop(FindSound(key));
}

bool SoundManager::StopBGM(SoundID id)
{
    const auto* entry = GetSoundEntry(id);
    return entry && entry->type == SOUND_TYPE::BGM && Stop(id);
}

bool SoundManager::StopBGM(std::string_view key)
{
    return StopBGM(FindSound(key));
}

bool SoundManager::IsFiniteVector(const VECTOR3::VEC3& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

SoundHandle SoundManager::Play_3DInstance(SoundID id, const VECTOR3::VEC3& pos, float radius,
    const VECTOR3::VEC3& velocity, bool loop)
{
    return Internal_Play3D(id, pos, radius, 0, velocity, loop);
}

SoundHandle SoundManager::Play_3DInstance(std::string_view key, const VECTOR3::VEC3& pos, float radius,
    const VECTOR3::VEC3& velocity, bool loop)
{
    return Play_3DInstance(FindSound(key), pos, radius, velocity, loop);
}

SoundHandle SoundManager::Play_RandPitch_3DInstance(SoundID id, const VECTOR3::VEC3& pos, float radius,
    int pitchRange, const VECTOR3::VEC3& velocity, bool loop)
{
    return Internal_Play3D(id, pos, radius, pitchRange, velocity, loop);
}

SoundHandle SoundManager::Play_RandPitch_3DInstance(std::string_view key, const VECTOR3::VEC3& pos, float radius,
    int pitchRange, const VECTOR3::VEC3& velocity, bool loop)
{
    return Play_RandPitch_3DInstance(FindSound(key), pos, radius, pitchRange, velocity, loop);
}

const SoundInstance* SoundManager::GetPlaying3DInstance(SoundHandle handle) const
{
    if (handle.managerGeneration != m_SoundGeneration || handle.playbackGeneration == 0
        || handle.slot >= m_3DSoundSlotArray.size()
        || handle.playbackGeneration != m_3DPlaybackGenerations[handle.slot])
        return nullptr;

    const auto& slot = m_3DSoundSlotArray[handle.slot];
    if (!slot._isUsed || !slot._pSourceVoice) return nullptr;

    // 自然終了から次のUpdateまでの間でも、終了済みの音を操作させない。
    XAUDIO2_VOICE_STATE state{};
    slot._pSourceVoice->GetState(&state);
    return state.BuffersQueued != 0 ? &slot : nullptr;
}

bool SoundManager::IsPlaying(SoundHandle handle) const
{
    return GetPlaying3DInstance(handle) != nullptr;
}

bool SoundManager::Set3DAttributes(SoundHandle handle, const VECTOR3::VEC3& pos,
    const VECTOR3::VEC3& velocity)
{
    if (!IsFiniteVector(pos) || !IsFiniteVector(velocity) || !GetPlaying3DInstance(handle))
        return false;

    // コピーするだけなので、毎フレームの名前検索や再生し直しは発生しない。
    auto& slot = m_3DSoundSlotArray[handle.slot];
    slot._pos = pos;
    slot._velocity = velocity;
    return true;
}

bool SoundManager::Stop(SoundHandle handle)
{
    if (!GetPlaying3DInstance(handle)) return false;
    return StopInstance(m_3DSoundSlotArray[handle.slot]);
}

bool SoundManager::Update3DSounds()
{
    if (!m_pXAudio2 || !m_pMasteringVoice || !m_DSPSettings.pMatrixCoefficients)
        return false;

    constexpr unsigned updateFlag = 1;
    bool success = true;
    for (auto& slot : m_3DSoundSlotArray)
    {
        if (!slot._isUsed || !slot._pSourceVoice) continue;

        XAUDIO2_VOICE_STATE state{};
        slot._pSourceVoice->GetState(&state);
        if (state.BuffersQueued == 0)
        {
            success = StopInstance(slot) && success;
            continue;
        }

        // 呼び出し側から受け取った現在位置で、音の方向と距離による減衰を計算する。
        X3DAUDIO_EMITTER emitter{};
        emitter.ChannelCount = 1;
        emitter.CurveDistanceScaler = slot._radius;
        emitter.DopplerScaler = 1.0f;
        emitter.Position = { slot._pos.x, slot._pos.y, slot._pos.z };
        emitter.Velocity = { slot._velocity.x, slot._velocity.y, slot._velocity.z };
        emitter.pVolumeCurve = &LinearCurve;
        // 音程は再生開始時の値を維持する。ドップラーによる音程変更は行わない。
        X3DAudioCalculate(m_X3DInstanceHandle, &m_Listener, &emitter,
            X3DAUDIO_CALCULATE_MATRIX, &m_DSPSettings);
        const HRESULT result = slot._pSourceVoice->SetOutputMatrix(m_pMasteringVoice,
            1, m_DeviceDetails.InputChannels, m_DSPSettings.pMatrixCoefficients, updateFlag);
        if (FAILED(result)) success = false;
    }
    // 途中に失敗があっても、すでに予約した変更はこのフレームで確定する。
    if (FAILED(m_pXAudio2->CommitChanges(updateFlag))) success = false;
    return success;
}
