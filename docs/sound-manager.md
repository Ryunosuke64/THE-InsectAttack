# SoundManagerの使い方

再生・停止には、`SoundID`または文字列キーを渡します。SE・BGM・VOICEの分類は音源側が持つため、再生時に指定する必要はありません。

## 再生と停止

```cpp
auto& sound = *Master::m_pSoundManager;

// 単発の再生は名前で指定できる。
sound.Play("ui.decision_01");

// 頻繁に使う音は、SoundManagerの初期化後にIDを取得してメンバーへ保存する。
SoundID fireSound = sound.FindSound("weapon.gun_fire_01");
if (sound.IsValid(fireSound))
{
    sound.Play_RandPitch(fireSound, 300);
    sound.Play_3D(fireSound, position, 40.0f);
    sound.Play_RandPitch_3D(fireSound, position, 40.0f, 300);
}

sound.PlayBGM("bgm.game_01");           // BGM専用版はループが既定値。
sound.Play("bgm.victory_01", false);   // 通常版からBGMを指定してもよい。
sound.StopBGM("bgm.game_01");
sound.Stop(fireSound);
```

3D再生の引数順は`音源、位置、半径、速度、ループ`です。ランダムピッチ付きの場合は、半径の次にピッチ幅が入ります。速度を省略するとゼロになります。

```cpp
sound.Play_3D(fireSound, position, 40.0f, velocity, true);
sound.Play_RandPitch_3D(fireSound, position, 40.0f, 300, velocity, true);
```

- 未登録の名前、無効なID、空の候補一覧は`false`を返します。
- 3D再生とランダムピッチ再生はSE・VOICE用です。BGMを渡すと`false`を返します。
- `PlayBGM`と`StopBGM`にSE・VOICEを渡した場合も`false`です。
- ピッチ幅は`0～1998`。`0`で変化なし、`100`で約±5%です。従来の単位を維持しています。
- 半径は有限の正の値、位置・速度は有限の値を指定します。
- BGMは同時に1曲で、再生すると以前のBGMを置き換えます。
- `Stop(id)`は、その音源の2D・3D再生をすべて停止します。別の音源は止めません。登録済みで再生中でない音への停止も成功扱いです。
- `SoundID`は音源のIDです。同じ音を複数回鳴らした際の、個々の再生を区別するハンドルではありません。

## 再生中の音を移動する物体に追従させる

`Play_3DInstance`は、個々の再生を識別する`SoundHandle`を返します。音を開始するのは一度だけにし、その後は`Set3DAttributes`で現在のワールド座標を渡します。音源の指定には`SoundID`と文字列の両方を使えます。

```cpp
// 呼び出し側のメンバーとして保持する。初期状態は無効ハンドル。
SoundHandle m_MoveSound;
```

```cpp
// 再生開始時に一度だけ呼ぶ（SoundManagerの初期化後）。
auto& sound = *Master::m_pSoundManager;
m_MoveSound = sound.Play_3DInstance(
    "enemy.octahedron.always", position, 300.0f, velocity, true);

if (!sound.IsPlaying(m_MoveSound))
{
    // 未登録音源・無効な引数・ボイスプール満杯などで、再生できなかった場合。
}
```

```cpp
// 物体の位置が確定したUpdate/LateUpdateで毎フレーム呼ぶ。
// ここでは再びPlayを呼ばない。
sound.Set3DAttributes(m_MoveSound, currentPosition, currentVelocity);
```

```cpp
// 物体の死亡・削除・シーン終了など、不要になった時点で停止する。
// SoundManagerが存在している間に呼ぶこと。
sound.Stop(m_MoveSound);
m_MoveSound = {};
```

- 同じ音素材を複数の物体が鳴らしても、各ハンドルで別々の位置を設定・停止できます。
- `Set3DAttributes`は位置・速度を保存し、次の`SoundManager::Update`で音の方向と距離減衰に反映します。現在のゲームループでは、ゲーム更新後にサウンド更新が呼ばれています。
- 座標は物体側から渡します。SoundManagerはTransformを保持せず、速度から位置を自動で積算しません。速度を省略した場合はゼロになります。
- `Play_RandPitch_3DInstance`も使用できます。引数は`音源、位置、半径、ピッチ幅、速度、ループ`です。追従中も開始時の音程を維持し、ドップラーによる音程変更は行いません。
- 自然終了・停止・SoundManagerの解放後はハンドルが無効になります。古いハンドルでは、再利用されたスロットの音を操作できません。
- 無効なハンドルに対する`IsPlaying`・`Set3DAttributes`・`Stop`は`false`を返します。不正な座標・速度を渡した場合、位置・速度の両方を変更しません。
- ハンドルを捨てるだけでは音は止まりません。ループ音は呼び出し側が終了時に停止してください。操作は既存のSoundManagerと同じゲームスレッドで行います。
- `Stop(SoundHandle)`はその再生だけ、`Stop(SoundID)`／`Stop(文字列)`は同じ音源の全再生を止めます。

既存の`Play_3D`・`Play_RandPitch_3D`は引き続き`bool`を返します。追従が不要な単発音は変更不要です。通常再生・ランダムピッチ再生・追従用再生は、共通の内部3D再生処理を使います。

EnemyControllerなどゲーム側への組み込みは、この変更には含めていません。現在の常時音を追従させる際は、毎フレームのループ再生を上記の開始・更新・停止に分けてください。

## ランダム再生

開始IDと個数による指定を廃止し、候補一覧を渡す形にしました。候補を離れた順番で登録しても、その一覧から選択されます。

```cpp
const std::array<SoundID, 3> shouts = {
    sound.FindSound("voice.shout_01"),
    sound.FindSound("voice.shout_02"),
    sound.FindSound("voice.shout_03")
};
sound.Play_Rand(shouts);

// 文字列の一覧でも指定できる。
constexpr std::array<std::string_view, 3> shoutNames = {
    "voice.shout_01", "voice.shout_02", "voice.shout_03"
};
sound.Play_Rand(shoutNames);
```

無効な候補が1つでもあれば、一覧全体を再生しません。読み込み時にIDを解決しておくと、再生時の名前検索も省けます。

## 新しい音源の追加

```cpp
SoundID reloadSound = sound.Load_Wav(
    "weapon.reload_01",
    "Resource/Sound/SE/Weapon/Reload_01.wav", // 実在するファイルを指定する。
    SOUND_TYPE::SE);

if (sound.IsValid(reloadSound))
    sound.Play(reloadSound);
```

キーの重複、ファイル読み込み失敗、不正な分類の場合は無効IDを返します。番号を手動で決めたり、個々の音源のenumを増やしたりする必要はありません。既存音源に別名を付ける`RegisterSoundName`は、読み込み済みの旧IDに対する移行用APIです。

対応するWAVは従来どおりPCM、44.1kHz、16bitです。SE・VOICEはモノラル、BGMはステレオです。

## IDの寿命と既存データ

`SoundID`は音源登録時に自動で割り当てます。追加登録で既存IDは変わりません。`UninitXA2Sound()`後はすべて無効になり、再初期化後に`FindSound`で取得し直します。解放前のIDが別の音源に化けないよう世代番号も確認しています。同じSoundManagerの初期化期間内だけで使用してください。

IDの数値や世代番号をJSONへ保存しないでください。新しいデータ形式で参照を保存する場合は文字列キーを使います。

今回は既存の数値JSONを変更していません。`SOUND_ID`・`VOICE_ID`・`BGM_ID`は音源登録と旧データの互換用に残しています。必要な場所では、次のように新しいIDへ変換できます。

```cpp
SoundID id = sound.FindSound(SOUND_ID::GUN_FIRE01);
sound.Play(id);
```

既存の武器JSONの読み込み形式は引き続き数値です。文字列JSONの読み込みを追加する場合は、その読み込み処理で`FindSound`を呼びます。

## 組み込みの文字列キー

`SoundManager.cpp`内の`definitions`が、キーとWAVファイルの対応一覧です。

| 用途 | キー |
|---|---|
| カーソル | `ui.cursor` |
| 決定音 | `ui.decision_01`、`ui.decision_02` |
| 戻る | `ui.back` |
| 銃声 | `weapon.gun_fire_01`、`weapon.gun_fire_02` |
| 跳弾 | `weapon.ricochet_01`、`weapon.ricochet_02` |
| 爆発 | `weapon.explosion_01` |
| 武器変更 | `weapon.change_01` |
| 建物破壊・落下 | `building.destruction`、`building.fall` |
| ジャンプ・着地 | `soldier.jump`、`soldier.land` |
| アリの被弾・死亡・酸発射 | `enemy.ant.hit_01`、`enemy.ant.dead`、`enemy.ant.acid_shoot` |
| 八面体の死亡・射撃 | `enemy.octahedron.dead`、`enemy.octahedron.shot_01`、`enemy.octahedron.shot_02` |
| アイテム取得 | `item.get`、`item.point_get` |
| BGM | `bgm.title_01`、`bgm.game_01`、`bgm.victory_01`、`bgm.hangeki` |
| 掛け声 | `voice.shout_01`、`voice.shout_02`、`voice.shout_03` |
