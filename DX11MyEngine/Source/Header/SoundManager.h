#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// XAudio2関連
#pragma comment(lib, "xaudio2.lib")
#include<xaudio2.h>

// 3Dサウンド用
#include<x3daudio.h>

// マルチメディア関連（waveファイルの読み込み用）
#pragma comment ( lib, "winmm.lib" )
#include<mmsystem.h>

#define MAX_WAV 100 //WAVサウンド最大数


// サウンドIDをintに変換するマクロ
#define SOUND_ID_TO_INT(_id) (static_cast<int>(_id))


constexpr int NUM_SE_SVPOOL         = 32;  // SV(SourceVoice)のプール数
constexpr int NUM_VOICE_SVPOOL      = 32;	// ボイス用
constexpr int NUM_SOUND_3D_SVPOOL   = 64;  // 3D用


/// <summary>
/// 音声の種類
/// </summary>
enum class SOUND_TYPE
{
    SE = 0,     
    BGM,
    VOICE,
};

/// <summary>
/// 既存の音源登録・数値JSONとの互換用ID（再生時はSoundIDへ変換する）
/// </summary>
enum class SOUND_ID
{
    NONE = -1,

    // SE
    /* 銃声 */
    GUN_FIRE01,            
    GUN_FIRE02,
    GUN_FIRE03,
    GUN_FIRE04,
    GUN_FIRE_LASER01,

    /* 武器チェンジ */
    GUN_CHANGE01,

    /* 跳弾 */
    ROCOCHET01,
    ROCOCHET02,

    /* 爆発 */
    EXPLOSION01,

    /* 建物 */
    BUILDING_DESTRUCTION,
    BUILDING_FALL,


    /* 兵士 */
    SOLDIER_R_JUMP_IN,       // ジャンプ - 開始
    SOLDIER_R_JUMP_LAND,     // ジャンプ - 着地


    /* 敵 */
    ENEMY_ANT_HIT01,         // アリ被弾 
    ENEMY_ANT_DEAD,          // アリやられる
    ENEMY_ANT_ACID_SHOOT,    // 酸射出

	ENEMY_OCTAHEDRON_SHOT01, // 八面体発射01
	ENEMY_OCTAHEDRON_SHOT02, // 八面体発射02
	ENEMY_OCTAHEDRON_DEAD,   // 八面体やられる
	ENEMY_OCTAHEDRON_ALWAYS, // 八面体の常時音

    /* アイテム */
    ITEM_GET,                // 取得
    POINT_GET,               // 取得


    
    /* システム */
    SYSTEM_MOVING_CURSOR01,  // 項目にカーソルがホバーしている状態
	SYSTEM_DECISION01,       // 決定音 1 
	SYSTEM_DECISION02,       // 決定音 2
	SYSTEM_BACK01,           // 戻る
    
    TEST,

    
    NUM
};

/// <summary>
/// 声の再生用ID
/// </summary>
enum class VOICE_ID
{
    NONE = -1,

    /* 掛け声的なやつ */
    SOLDIER_R_SHOUT_01,
    SOLDIER_R_SHOUT_02,
    SOLDIER_R_SHOUT_03,

    /* 被弾 */
    SOLDIER_R_HIT_01,
    SOLDIER_R_HIT_02,
    SOLDIER_R_HIT_03,


    NUM
};

/// <summary>
/// BGMの再生用ID
/// </summary>
enum class BGM_ID
{
    NONE = -1,

    // BGM
    BGM_TITLE_01,
	BGM_GAME_01,
    BGM_VICTORY01,
    BGM_HANGEKI,

    NUM
};

/// <summary>
/// 登録済みの音源を識別するID。初期化し直した場合はFindSoundで取得し直す。
/// </summary>
struct SoundID
{
    std::size_t value = static_cast<std::size_t>(-1);
    std::uint64_t generation = 0; // 解放前のIDを、再初期化後の別の音源に使わせない。
};

/// <summary>
/// 再生中の3D音声1つを識別するハンドル。音源そのもののSoundIDとは別物。
/// 終了・停止・SoundManagerの解放後は無効になる。初期値も無効。
/// </summary>
struct SoundHandle
{
    std::size_t slot = static_cast<std::size_t>(-1);
    std::uint64_t playbackGeneration = 0;
    std::uint64_t managerGeneration = 0;
};

/// <summary>
/// wavデータ
/// </summary>
struct WaveResource
{
    WAVEFORMATEX _waveFormat;   // waveファイルのフォーマット
    BYTE* _waveBuffer;          // 波形データ
    DWORD _size;                // 波形データのサイズ

    WaveResource() :
        _waveFormat(),
        _waveBuffer(nullptr),
        _size(0)
    {
    };

    // 波形バッファはこのリソースだけが所有する。
    WaveResource(const WaveResource&) = delete;
    WaveResource& operator=(const WaveResource&) = delete;
    ~WaveResource() { delete[] _waveBuffer; }
};

/// <summary>
/// 再生に必要なデータ
/// </summary>
struct SoundInstance
{
    IXAudio2SourceVoice* _pSourceVoice; // 音の起点
    VECTOR3::VEC3 _pos;                 // 3Dサウンド用の位置情報
    VECTOR3::VEC3 _velocity;            // 3Dサウンド用の移動ベクトル情報
    bool _isUsed;                       // 再生中かどうか
    bool _isLoop;                       // ループ再生するかどうか
    float _volumeFactor;                // 音量調整用の係数（0.0f～1.0f  1.0fなら元の音量）
    float _volumeDefault;               // デフォルト音量
    float _radius;                      // 音が聞こえる半径
    SOUND_TYPE _soundType;              // 音声の種類（SE、BGM、ボイス）
    int _soundID;                       // どの音声データを再生しているかのID

    SoundInstance() :
        _pSourceVoice(nullptr),
        _pos(VECTOR3::VEC3()),
        _velocity(VECTOR3::VEC3()),
        _isUsed(false),
        _isLoop(false),
        _soundID(-1),
        _soundType(SOUND_TYPE::SE),
        _volumeFactor(1.0f),
        _volumeDefault(0.0f),
        _radius(0.0f)
    {
    };
    ~SoundInstance() {};
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SoundManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】音声管理
//      Wavファイルの読み込み、再生、停止などの役割を持つ
// ※ファイルフォーマットについて
//     ここでは、WAVEファイルのフォーマットはPCMで、サンプリングレート44100Hz、ビット深度16bit、
//     3D用サウンドやSEはモノラル（1チャンネル）、BGMはステレオ（２チャンネル）を前提としています。
// 
//      
// 参考サイト：https://zenn.dev/kd_gamegikenblg/articles/4f955822601484
// 
// X3DAudio：https://learn.microsoft.com/ja-jp/windows/win32/xaudio2/x3daudio-overview
//           https://rastertek.com/dx11win10tut58.html
// ***************************************************************************************
class SoundManager
{
private:
    // 本体
    IXAudio2* m_pXAudio2;


    // 最終的な音路出力する
    IXAudio2MasteringVoice* m_pMasteringVoice; 


    // リスナー（カメラ）
    X3DAUDIO_LISTENER m_Listener;              


    // 音声の詳細情報
    XAUDIO2_VOICE_DETAILS m_DeviceDetails;     


    // X3DAudioCalculateからの処理結果を受け取るためのもの！
    // DSP:音の加工に使うもの（？）
    X3DAUDIO_DSP_SETTINGS m_DSPSettings;

    // X3Dインスタンスハンドル
    X3DAUDIO_HANDLE m_X3DInstanceHandle;


    std::unordered_map<SOUND_ID, WaveResource> m_SE_WaveResourceMap;    // リソース - SE
    std::unordered_map<VOICE_ID, WaveResource> m_Voice_WaveResourceMap; // リソース - ボイス
    std::unordered_map<BGM_ID, WaveResource> m_BGM_WaveResourceMap;     // リソース - BGM

    std::array<SoundInstance, NUM_SE_SVPOOL>       m_SoundSlotArray;         // 主にSE用のソースボイス
    std::array<SoundInstance, NUM_VOICE_SVPOOL>    m_VoiceSoundSlotArray;    // ボイス用のソースボイス
    std::array<SoundInstance, NUM_SOUND_3D_SVPOOL> m_3DSoundSlotArray;       // 3Dサウンド用のソースボイス
    // スロットを再利用するたびに進め、古いハンドルによる別の音の操作を防ぐ。
    std::array<std::uint64_t, NUM_SOUND_3D_SVPOOL> m_3DPlaybackGenerations{};
    SoundInstance m_BGM_SoundSlot;      // BGM用 基本一つだけ再生するので


    // 既存の音源管理との接続情報。
    struct SoundEntry
    {
        SOUND_TYPE type;
        int existingId;
    };
    // string_viewでも検索できるため、文字列版Playで一時的なstringを作らずに済む。
    struct SoundNameHash
    {
        using is_transparent = void;
        std::size_t operator()(std::string_view key) const noexcept
        {
            return std::hash<std::string_view>{}(key);
        }
    };
    std::unordered_map<std::string, SoundID, SoundNameHash, std::equal_to<>> m_SoundNameToId;
    std::vector<SoundEntry> m_SoundEntries; // 追加のみ。登録済みIDの並びは変えない。
    std::uint64_t m_SoundGeneration = 1;
    
    // 音量調整用
    float m_SEVolume;
    float m_BGMVolume;
    float m_VoiceVolume;
public:
    SoundManager();
    ~SoundManager();

    // 初期化
    bool InitXA2Sound(void);        

    // 解放
    void UninitXA2Sound(void);
    
    // 更新
    bool Update(RendererEngine& renderer);

    // 新規音源はキー・ファイル・分類で登録する。失敗時は無効IDを返す。
    SoundID Load_Wav(std::string_view key, const char* filename, SOUND_TYPE type);
    SoundID RegisterSoundName(std::string_view key, SOUND_TYPE type, int existingId);
    SoundID FindSound(std::string_view key) const;
    bool IsValid(SoundID id) const;

    // 既存enum・数値JSONを読み込む箇所だけで使う移行用の変換。
    SoundID FindSound(SOUND_ID id) const;
    SoundID FindSound(VOICE_ID id) const;
    SoundID FindSound(BGM_ID id) const;

    // 文字列版は名前を検索してID版へ渡す。頻繁に鳴らす音はIDを保持しておく。
    bool Play(SoundID id, bool loop = false);
    bool Play(std::string_view key, bool loop = false);
    bool Play_3D(SoundID id, const VECTOR3::VEC3& pos, float radius,
        const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);
    bool Play_3D(std::string_view key, const VECTOR3::VEC3& pos, float radius,
        const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);

    // pitchRangeは従来と同じ単位。100で約±5%、0で変化なし（0～1998）。
    bool Play_RandPitch(SoundID id, int pitchRange, bool loop = false);
    bool Play_RandPitch(std::string_view key, int pitchRange, bool loop = false);
    bool Play_RandPitch_3D(SoundID id, const VECTOR3::VEC3& pos, float radius,
        int pitchRange, const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);
    bool Play_RandPitch_3D(std::string_view key, const VECTOR3::VEC3& pos, float radius,
        int pitchRange, const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);

    // 追従させたい音は一度だけ再生して、返されたハンドルを呼び出し側で保持する。
    SoundHandle Play_3DInstance(SoundID id, const VECTOR3::VEC3& pos, float radius,
        const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);
    SoundHandle Play_3DInstance(std::string_view key, const VECTOR3::VEC3& pos, float radius,
        const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);
    SoundHandle Play_RandPitch_3DInstance(SoundID id, const VECTOR3::VEC3& pos, float radius,
        int pitchRange, const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);
    SoundHandle Play_RandPitch_3DInstance(std::string_view key, const VECTOR3::VEC3& pos, float radius,
        int pitchRange, const VECTOR3::VEC3& velocity = VECTOR3::VEC3(), bool loop = false);

    // 次のUpdateで反映。速度から位置を自動計算せず、物体の現在位置を毎回渡す。
    // 終了済みハンドル・不正な座標はfalse。更新による再生のやり直しは行わない。
    bool Set3DAttributes(SoundHandle handle, const VECTOR3::VEC3& pos,
        const VECTOR3::VEC3& velocity = VECTOR3::VEC3());
    bool IsPlaying(SoundHandle handle) const;
    bool Stop(SoundHandle handle); // この再生1つだけを停止する。

    // 連番に依存せず、候補一覧から選ぶ。空・無効ID・未登録名を含む一覧は再生しない。
    bool Play_Rand(std::span<const SoundID> candidates, bool loop = false);
    bool Play_Rand(std::span<const std::string_view> keys, bool loop = false);
    bool PlayBGM(SoundID id, bool loop = true);
    bool PlayBGM(std::string_view key, bool loop = true);

    // 同じ音源の再生をすべて停止する。個々の再生インスタンスを表すIDではない。
    bool Stop(SoundID id);
    bool Stop(std::string_view key);
    bool StopBGM(SoundID id);
    bool StopBGM(std::string_view key);

	void set_Volume(SOUND_TYPE _type, float _volume);    // 音量の設定
    float get_Volume(SOUND_TYPE _type)const;        // 音量の取得

private:
    // コピー禁止
    SoundManager(const SoundManager &) = delete;
    SoundManager &operator=(const SoundManager &) = delete;


    // WAV読み込みと、分類ごとのボイスプールへの再生処理。
    bool Internal_Load_Wav(const char* _filename, WaveResource& _out, int _channelNum = 2, int _sampleLate = 44100, int _bitsPersample = 16);
    bool Internal_SoundPlay(SOUND_TYPE _type, int _id, bool _loop = false);
    bool Internal_SoundPlay_RandPitch(SOUND_TYPE _type, int _id, int _pitchRange, bool _loop = false);

    // 通常・ランダムピッチ・追従用の公開APIが共用する3D再生処理。
    SoundHandle Internal_Play3D(SoundID id, const VECTOR3::VEC3& pos, float radius,
        int pitchRange, const VECTOR3::VEC3& velocity, bool loop);
    SoundHandle Internal_Start3D(SOUND_TYPE type, int id, const VECTOR3::VEC3& pos,
        float radius, int pitchRange, const VECTOR3::VEC3& velocity, bool loop);
    const SoundInstance* GetPlaying3DInstance(SoundHandle handle) const;
    bool Update3DSounds();
    static bool IsFiniteVector(const VECTOR3::VEC3& value);

    // 分類と旧IDを扱う処理は内部に閉じ込める。
    bool Load_Wav(const char* filename, SOUND_TYPE type, int existingId);
    bool Internal_PlayBGM(BGM_ID id, bool loop);
    bool StopInstance(SoundInstance& instance);
    const SoundEntry* GetSoundEntry(SoundID id) const;
    SoundID FindLegacySound(SOUND_TYPE type, int existingId) const;

    bool SoundTypeAndIDConvertToResource(SOUND_TYPE _type, int _id, WaveResource *&_outResource, std::span<SoundInstance> &_outInstArray);
    bool SerchResource(SOUND_TYPE _type, int _id, WaveResource *&_outResource);

};

