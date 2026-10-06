"use strict";
    const $ = id => document.getElementById(id);
    const own = (object, key) => object !== null && typeof object === "object" && Object.prototype.hasOwnProperty.call(object, key);
    const object = value => value !== null && typeof value === "object" && !Array.isArray(value);
    const clone = value => JSON.parse(JSON.stringify(value));
    const get = (value, path) => path.split(".").reduce((current, key) => own(current, key) ? current[key] : undefined, value);
    const has = (value, path) => get(value, path) !== undefined;
    const put = (value, path, next) => {
      const keys = path.split(".");
      const last = keys.pop();
      let current = value;
      for (const key of keys) { if (!object(current[key])) current[key] = {}; current = current[key]; }
      current[last] = clone(next);
    };
    const remove = (value, path) => {
      const keys = path.split("."); const last = keys.pop();
      const parent = keys.length ? get(value, keys.join(".")) : value;
      if (object(parent)) delete parent[last];
    };
    const make = (tag, className, content) => {
      const element = document.createElement(tag);
      if (className) element.className = className;
      if (content !== undefined) element.textContent = content;
      return element;
    };
    const bulletTypes = [["NORMAL", "通常弾"], ["EXPLOSION", "爆発弾"], ["HORMING", "誘導弾（既存の綴り）"], ["LASER", "レーザー"], ["FLAME", "火炎"], ["ACID", "酸"]];
    const categories = [["PLAYER", "プレイヤー"], ["PLAYER_BULLET", "プレイヤー弾"], ["ENEMY", "敵"], ["ENEMY_BULLET", "敵の弾"], ["BUILDING", "建物"], ["DESTRUCTION_BUILDING", "破壊可能な建物"], ["ITEM", "アイテム"], ["EVERY", "すべて"]];
    const num = (path, label, value, hint = "", min = 0, integer = false, extra = {}) => ({path, label, value, hint, min, integer, max: integer ? 2147483647 : 3.402823466e38, kind: "number", ...extra});
    const text = (path, label, value = "", hint = "") => ({path, label, value, hint, kind: "text"});
    const flag = (path, label, value = false, hint = "") => ({path, label, value, hint, kind: "boolean"});
    const vector = (path, label, value, axes = ["X", "Y", "Z"], hint = "", extra = {}) => ({path, label, value, axes, hint, kind: "vector", min: 0, max: 3.402823466e38, ...extra});
    const choice = (path, label, value, options, hint = "", extra = {}) => ({path, label, value, options, hint, kind: "select", ...extra});
    const common = "bulletParam.common.", move = "bulletParam.movement.", hit = "bulletParam.hit.", visual = "bulletParam.visual.", custom = visual + "custom.";
    const homing = data => get(data, move + "type") === "HOMING";
    const explosion = data => get(data, hit + "type") === "EXPLOSION";
    const direct = data => (get(data, hit + "type") ?? "DIRECT") === "DIRECT";
    const scaleLerp = data => get(data, custom + "type") === "ScaleLerp";
    const sections = [
      {id: "weapon", title: "武器の基本性能", key: "root", hint: "弾数・連射速度・リロードなど、武器全体の設定です。", fields: [
        text("name", "武器名", "新しい武器", "ゲームに表示される名前。日本語を使えます。"),
        num("level", "武器レベル", 0, "未指定を表す -1 も設定できます。", -1, true),
        num("bulletMaxNum", "最大装弾数", 120, "0は弾数を持たない武器として扱われます。", 0, true),
        num("bulletSimultaneousNum", "同時発射弾数", 1, "散弾など、1回の射撃で発射する数。", 1, true),
        num("fireRate", "連射速度", 12, "1秒あたりの発射回数。0より大きい値が必要です。", 0, false, {positive: true}),
        num("reloadTime", "リロード時間", 1.5, "秒"),
        num("accuracy", "弾のばらつき", .03, "ラジアン。小さいほど集弾性が高くなります。"),
        num("zoomLength", "ズーム倍率", 1, "1より大きい値でズームします。"),
        flag("isLaserSight", "レーザーサイト", true),
        num("soundID", "発射音ID", 0, "ゲーム側のSOUND_ID。-1も設定できます。", -1, true),
        text("muzzleFlashEffectTag", "マズルフラッシュのタグ", "MuzzleEffect_01", "空文字で指定なし。候補から選ぶか、登録済みのタグを入力します。"),
        vector("muzzleFlashEffectScale", "マズルフラッシュの大きさ", [.6,.6,.6]),
        choice("bulletType", "武器側の弾種", "NORMAL", bulletTypes, "既存JSONのトップレベルの値です。弾そのものの分類は次の「弾の判定用分類」で設定します。")
      ]},
      {id: "common", title: "弾の基本性能・衝突", key: "bulletParam.common", hint: "判定用分類は材質別ヒット演出にも使われます。武器側の弾種とは独立して設定します。", fields: [
        num(common + "damage", "1発のダメージ", 35),
        num(common + "aliveFrame", "弾の寿命", 60, "60で1秒。60fps基準のフレーム数です。", 0, true),
        num(common + "speed", "初速", 228, "ゲーム内の速度値。"),
        num(common + "maxSpeed", "最大速度", 228),
        num(common + "acceleration", "加速度", 0, "減速を設定する場合は負の値も入力できます。", 0, false, {min: -3.402823466e38}),
        num(common + "gravityScale", "重力倍率", 0, "0で重力なし。負の値も入力できます。", 0, false, {min: -3.402823466e38}),
        num(common + "penetrationsCount", "貫通回数", 0, "整数", 0, true),
        num(common + "collisionSize", "当たり判定サイズ", 0),
        num(common + "knockbackForce", "ノックバック力", 8),
        choice(common + "bulletType", "弾の判定用分類", "NORMAL", bulletTypes, "ロケットでもNORMALの既存データがあります。武器側の弾種とは自動同期しません。", {required: true}),
        choice(common + "collisionCategory", "自身の衝突カテゴリ", "PLAYER_BULLET", categories, "通常はプレイヤー弾。", {required: true}),
        {path: common + "collisionMask", label: "衝突する対象", value: ["ENEMY", "BUILDING", "DESTRUCTION_BUILDING"], options: categories, kind: "mask", hint: "複数選択できます。空配列も保存できます。", wide: true},
        {...num(common + "range", "旧射程値（保持用）", 0, "現在の読み込み処理ではこの値を使用しません。"), when: data => has(data, common + "range")}
      ]},
      {id: "movement", title: "弾の移動", key: "bulletParam.movement", hint: "直進と誘導を選べます。", fields: [
        choice(move + "type", "移動方式", "LINEAR", [["LINEAR", "直進"], ["HOMING", "誘導"]]),
        {...num(move + "turnSpeed", "誘導の旋回速度", .1), when: homing},
        {...num(move + "targetingStartDelay", "誘導開始までの時間", 0, "秒"), when: homing},
        {...num(move + "accelerationStartDelay", "加速開始までの時間", 0, "秒"), when: homing},
        {...num(move + "targetingDuration", "誘導を続ける時間", 3, "秒。現在の実装では0にすると誘導しません。"), when: homing},
        {...num(move + "lockOnRange", "ロックオン距離", 500), when: homing},
        {...num(move + "lockOnHalfAngleDeg", "ロックオンの半角", 90, "度", 0, false, {max: 180}), when: homing}
      ]},
      {id: "hit", title: "命中・爆発", key: "bulletParam.hit", hint: "直接命中か爆発かを設定します。", fields: [
        choice(hit + "type", "命中方式", "DIRECT", [["DIRECT", "直接命中"], ["EXPLOSION", "爆発"]]),
        text(hit + "decalMaterialTag", "弾痕マテリアルのタグ", "Decal_BulletHole"),
        text(hit + "hitEffectTag", "命中エフェクトのタグ", "BulletHit_Standard"),
        {...choice(hit + "environmentResponse", "地形への命中時の動作", "DEACTIVATE", [["DEACTIVATE", "弾を消す"], ["SLIDE", "壁に沿って滑る"], ["BOUNCE", "反射"], ["ATTACH", "貼り付く"], ["PENETRATE", "貫通"]], "", {required: true}), when: direct},
        {...num(hit + "explosionRadius", "爆発半径", 10), when: explosion},
        {...text(hit + "explosionEffectTag", "爆発エフェクトのタグ", "Explosion_01"), when: explosion},
        {...num(hit + "explosionEffectAliveTime", "爆発エフェクトの寿命倍率", 1, "1で元のエフェクトの寿命。"), when: explosion},
        {...flag(hit + "isSmoke", "爆発時に煙を出す", true), when: explosion},
        {...vector(hit + "expLightColor", "爆発ライトの色", [.8,.4,.1], ["R", "G", "B"], "RGBの数値。1を超える値も設定できます。"), when: explosion},
        {...num(hit + "expLightIntensity", "爆発ライトの強さ", 4), when: explosion},
        {...num(hit + "expLightDuration", "爆発ライトの継続時間", 3, "秒"), when: explosion}
      ]},
      {id: "visual", title: "弾の見た目・軌跡・煙", key: "bulletParam.visual", hint: "色や大きさは数値で指定します。RGBの軌跡色は1を超える値も保持できます。", fields: [
        choice(visual + "type", "描画方式", "BILLBOARD", [["BILLBOARD", "ビルボード"], ["MODEL", "3Dモデル"], ["EFFECT", "エフェクト"]]),
        text(visual + "bulletMaterialTag", "弾のマテリアルのタグ", "Bullet_01"),
        {...text(visual + "bulletEffectTag", "弾のエフェクトのタグ", "", "EFFECTで使用します。"), when: data => get(data, visual + "type") === "EFFECT" || has(data, visual + "bulletEffectTag")},
        vector(visual + "scale", "弾の大きさ", [.05,.05,.05]),
        {...vector(visual + "effectColor", "弾エフェクトの色", [255,255,255,255], ["R", "G", "B", "A"], "RGBA・各0〜255。Aは不透明度です。", {max: 255}), when: data => get(data, visual + "type") === "EFFECT" || has(data, visual + "effectColor")},
        num(visual + "trailDrawTime", "軌跡が残る時間", .02, "秒"),
        num(visual + "trailWidth", "軌跡の幅", .8),
        vector(visual + "trailColor", "軌跡の色", [1.5,1.5,1.5], ["R", "G", "B"], "1を超える値も設定できます。"),
        flag(visual + "enableFlightSmoke", "飛行中に煙を出す", false),
        num(visual + "smokeInterval", "煙の発生間隔", .05, "秒。飛行中の煙が有効な場合に使用します。"),
        num(visual + "smokeSize", "煙の大きさ", 0),
        text(visual + "smokeEffectTag", "煙のエフェクトのタグ", "")
      ]},
      {id: "custom", title: "時間経過によるサイズ変化", key: "bulletParam.visual.custom", hint: "ScaleLerpでは開始サイズから終了サイズへ変化します。", fields: [
        choice(custom + "type", "サイズ変化", "", [["", "なし"], ["ScaleLerp", "開始→終了サイズを補間"]]),
        {...vector(custom + "startScale", "開始サイズ", [1,1,1]), when: scaleLerp},
        {...vector(custom + "endScale", "終了サイズ", [2,2,2]), when: scaleLerp},
        {...num(custom + "duration", "変化にかける時間", 1, "秒"), when: scaleLerp}
      ]}
    ];
    const allFields = sections.flatMap(section => section.fields);
    const fieldMap = new Map(allFields.map(field => [field.path, field]));
    // Screen labels and layout are independent of the game's JSON schema.
    const presentation = {
      name: {label: "武器の名前", hint: "ゲームに表示される名前です。", wide: true},
      bulletMaxNum: {label: "装弾数", unit: "発", hint: "リロードするまでに撃てる数。"},
      bulletSimultaneousNum: {label: "1回に撃つ弾の数", unit: "発", hint: "大きくするとショットガンのような撃ち方に。"},
      fireRate: {label: "連射速度", unit: "回/秒", hint: "大きいほど素早く連射します。"},
      reloadTime: {label: "リロード時間", unit: "秒", hint: "弾を補充するまでの時間。"},
      accuracy: {label: "弾の広がり", unit: "rad", hint: "小さいほど狙った方向へまとまって飛びます。0で広がりなし。"},
      zoomLength: {label: "ズーム倍率", unit: "倍", hint: "1より大きくすると、狙うときに拡大します。"},
      isLaserSight: {label: "照準レーザー", hint: "狙う方向をレーザーで表示します。"},
      [common + "damage"]: {label: "1発の威力", hint: "弾が命中したときのダメージ。"},
      [common + "speed"]: {label: "弾の速さ", hint: "大きいほど速く飛びます。"},
      [common + "aliveFrame"]: {label: "弾が消えるまで", unit: "秒", displayScale: 60, hint: "飛び続けられる時間。60分の1秒単位で保存します。"},
      [common + "maxSpeed"]: {label: "弾の最高速度", hint: "加速したときに到達できる速度。"},
      [common + "acceleration"]: {label: "飛行中の加速", hint: "0で一定の速さ。マイナスで減速します。"},
      [common + "gravityScale"]: {label: "重力の影響", hint: "0でまっすぐ飛びます。大きいほど下へ落ちます。"},
      [common + "penetrationsCount"]: {label: "貫通できる回数", unit: "回", hint: "弾が敵などを貫通できる回数。"},
      [common + "knockbackForce"]: {label: "押し戻す強さ", hint: "命中した相手を押し戻す力。"},
      [common + "bulletType"]: {label: "弾の属性", hint: "命中時の演出に使う分類。武器の分類とは個別に設定できます。"},
      [common + "collisionCategory"]: {label: "弾の所属", hint: "通常はプレイヤーの弾を選びます。"},
      [common + "collisionMask"]: {label: "弾が当たる相手", hint: "当たり判定の対象を選びます。複数選択できます。"},
      [move + "type"]: {label: "飛び方", hint: "直進は撃った方向へ、誘導は狙った相手を追いかけます。"},
      [move + "targetingStartDelay"]: {label: "追いかけ始めるまで", unit: "秒", hint: "発射してから誘導を始めるまでの時間。"},
      [move + "accelerationStartDelay"]: {label: "加速し始めるまで", unit: "秒", hint: "発射してから加速を始めるまでの時間。"},
      [move + "targetingDuration"]: {label: "追いかけ続ける時間", unit: "秒", hint: "0では相手を追いかけません。"},
      [move + "turnSpeed"]: {label: "曲がる速さ", hint: "相手に向かって方向を変える速さ。"},
      [move + "lockOnHalfAngleDeg"]: {label: "狙える範囲の半角", unit: "度", hint: "照準を中心に、相手を探す範囲。"},
      [move + "lockOnRange"]: {label: "相手を狙える距離", hint: "誘導の対象を探す距離。"},
      [hit + "type"]: {label: "命中したとき", hint: "弾そのものが当たるか、着弾地点の周りも爆発させるかを選びます。"},
      [hit + "environmentResponse"]: {label: "壁や地面に当たると", hint: "地形に当たった後の弾の動き。"},
      [hit + "explosionRadius"]: {label: "爆発の広さ", hint: "爆発が届く範囲の半径。"},
      [visual + "type"]: {label: "弾の表示方法", hint: "見た目に使う素材の種類。"},
      [visual + "bulletMaterialTag"]: {label: "弾の素材", hint: "候補から選ぶか、ゲームに登録された素材名を入力します。"},
      [visual + "bulletEffectTag"]: {label: "弾のエフェクト", hint: "エフェクト表示で使う演出。"},
      [visual + "scale"]: {label: "弾の大きさ", axes: ["幅", "高さ", "奥行き"], hint: "3方向の大きさを個別に調整できます。"},
      [visual + "trailDrawTime"]: {label: "軌跡が残る時間", unit: "秒", hint: "0で軌跡を残しません。"},
      [visual + "trailWidth"]: {label: "軌跡の太さ", hint: "弾が残す光の線の太さ。"},
      [visual + "trailColor"]: {label: "軌跡の色", hint: "色を選べます。数値を1より大きくすると明るい色になります。"},
      [visual + "enableFlightSmoke"]: {label: "飛行中の煙", hint: "飛んでいる弾から煙を出します。"},
      [visual + "smokeInterval"]: {label: "煙を出す間隔", unit: "秒"},
      [visual + "smokeEffectTag"]: {label: "煙のエフェクト", hint: "ゲームに登録されている煙の演出を選びます。"},
      [custom + "type"]: {label: "飛行中のサイズ変化", options: [["", "変化させない"], ["ScaleLerp", "徐々に大きさを変える"]], hint: "飛び始めと終わりで弾の大きさを変えられます。"},
      [custom + "duration"]: {label: "大きさが変わるまで", unit: "秒"}
    };
    allFields.forEach(field => Object.assign(field, presentation[field.path] || {}));
    const pages = [
      {id: "basic", title: "基本性能", icon: "sliders", description: "威力と撃ち心地を調整します。", groups: [
        {title: "武器の名前", icon: "edit", paths: ["name"]},
        {title: "攻撃性能", icon: "target", paths: [common + "damage", "fireRate", "bulletSimultaneousNum", common + "speed"]},
        {title: "撃ち心地", icon: "sliders", paths: ["bulletMaxNum", "reloadTime", "accuracy", "zoomLength", "isLaserSight"]}
      ]},
      {id: "motion", title: "弾の動き", icon: "route", description: "弾の飛び方と、命中したときの動作を選びます。", groups: [
        {title: "飛び方", icon: "route", paths: [move + "type", common + "aliveFrame", common + "maxSpeed", common + "acceleration", common + "gravityScale"]},
        {title: "相手を追いかける", icon: "target", when: homing, paths: sections[2].fields.slice(1).map(field => field.path)},
        {title: "命中したとき", icon: "burst", paths: [hit + "type", hit + "environmentResponse", hit + "explosionRadius", common + "penetrationsCount", common + "knockbackForce"]}
      ]},
      {id: "appearance", title: "見た目", icon: "spark", description: "弾の素材、光の軌跡、煙をカスタマイズします。", groups: [
        {title: "弾の見た目", icon: "spark", paths: [visual + "type", visual + "bulletMaterialTag", visual + "bulletEffectTag", visual + "scale", visual + "effectColor"]},
        {title: "光の軌跡", icon: "route", paths: [visual + "trailDrawTime", visual + "trailWidth", visual + "trailColor"]},
        {title: "飛行中の煙", icon: "cloud", paths: [visual + "enableFlightSmoke", visual + "smokeInterval", visual + "smokeSize", visual + "smokeEffectTag"]},
        {title: "サイズの変化", icon: "expand", paths: sections[5].fields.map(field => field.path)}
      ]},
      {id: "advanced", title: "詳細設定", icon: "settings", description: "音、命中時の演出、当たり判定などを細かく設定します。", groups: [
        {title: "分類と当たり判定", icon: "target", paths: ["level", "bulletType", common + "bulletType", common + "collisionCategory", common + "collisionSize", common + "collisionMask", common + "range"]},
        {title: "発射時の音と演出", icon: "sound", paths: ["soundID", "muzzleFlashEffectTag", "muzzleFlashEffectScale"]},
        {title: "命中時の演出", icon: "burst", paths: [hit + "decalMaterialTag", hit + "hitEffectTag", hit + "explosionEffectTag", hit + "explosionEffectAliveTime", hit + "isSmoke", hit + "expLightColor", hit + "expLightIntensity", hit + "expLightDuration"]}
      ]}
    ];
    const iconPaths = {
      layers: '<path d="m12 3 9 5-9 5-9-5 9-5Z"/><path d="m3 12 9 5 9-5M3 16l9 5 9-5"/>',
      sliders: '<path d="M4 7h16M4 17h16"/><circle cx="8" cy="7" r="3" fill="currentColor" stroke="none"/><circle cx="16" cy="17" r="3" fill="currentColor" stroke="none"/>',
      route: '<path d="M3 18h7c7 0 3-12 10-12M16 3l4 3-4 3"/><circle cx="3" cy="18" r="1"/>',
      spark: '<path d="m12 3 2.5 6.5L21 12l-6.5 2.5L12 21l-2.5-6.5L3 12l6.5-2.5L12 3Z"/>',
      settings: '<path d="M9 3h6l1 4 4 1 1 5-4 2-1 5h-6l-1-4-5-2-1-5 5-2 1-4Z"/><circle cx="12" cy="12" r="3"/>',
      target: '<circle cx="12" cy="12" r="8"/><circle cx="12" cy="12" r="3"/><path d="M12 2v4M12 18v4M2 12h4M18 12h4"/>',
      burst: '<path d="m12 2 2 6 6-3-3 6 5 3-6 1 1 6-5-4-5 4 1-6-6-1 5-3-3-6 6 3 2-6Z"/>',
      folder: '<path d="M3 7V5h6l2 2h10v13H3V7Z"/><path d="M3 9h18"/>',
      save: '<path d="M12 3v12m-4-4 4 4 4-4M4 16v5h16v-5"/>',
      edit: '<path d="m14 4 6 6M3 21l5-1L21 7l-6-6L2 14l1 7Z"/>',
      reset: '<path d="M3 10a9 9 0 1 1 3 9M3 4v6h6"/>',
      help: '<circle cx="12" cy="12" r="9"/><path d="M9 9a3 3 0 1 1 4 3c-1 1-1 1-1 3M12 18h.01"/>',
      grid: '<rect x="3" y="3" width="7" height="7" rx="1"/><rect x="14" y="3" width="7" height="7" rx="1"/><rect x="3" y="14" width="7" height="7" rx="1"/><rect x="14" y="14" width="7" height="7" rx="1"/>',
      cloud: '<path d="M7 19h11a4 4 0 0 0 0-8 6 6 0 0 0-12-1 4.5 4.5 0 0 0 1 9Z"/>',
      expand: '<path d="M3 9V3h6m6 0h6v6m0 6v6h-6M9 21H3v-6M3 3l5 5m8 8 5 5"/>',
      sound: '<path d="M4 9h4l5-5v16l-5-5H4V9Zm12-1a6 6 0 0 1 0 8m3-11a10 10 0 0 1 0 14"/>',
      weapon: '<path d="M2 8h15v6H8l-3 4H2l2-7M17 10h5M10 14l2 5h4l-2-5M7 5h7v3"/>'
    };
    function icon(name) {
      const span = make("span");
      span.innerHTML = '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true">' + (iconPaths[name] || iconPaths.weapon) + '</svg>';
      return span.firstChild;
    }
    let activePage = "basic", technicalMode = false;
    const drafts = new Map();
    const templates = JSON.parse($("weaponTemplates").textContent);
    let data, baseline, baselineFileName, fileHandle = null, originalFileText = null, sourceLabel = "", legacy = false, busy = false;
    let visibleControls = [];
    const removedValues = new Map();
    const isLegacy = weapon => object(weapon.bulletParam) && !own(weapon.bulletParam, "common") && (own(weapon.bulletParam, "damage") || own(weapon.bulletParam, "speed"));
    const serialize = () => JSON.stringify(data, null, 2) + "\n";
    const dirty = () => JSON.stringify(data) !== JSON.stringify(baseline) || $("fileName").value !== baselineFileName || visibleControls.some(control => !control.validity.valid);
    const showMessage = (content, kind = "") => { $("message").textContent = content; $("message").className = "message " + kind; $("message").hidden = false; };
    const confirmDiscard = () => !dirty() || window.confirm("保存していない変更があります。変更を破棄して切り替えますか？");
    function structuralErrors(weapon) {
      if (!object(weapon) || !object(weapon.bulletParam)) return ["武器のJSONオブジェクトとbulletParamが必要です。SurfaceHitData.jsonは対象外です。"];
      const errors = [];
      const inspectNumbers = value => {
        if (typeof value === "number" && (!Number.isFinite(value) || (Number.isInteger(value) && !Number.isSafeInteger(value)))) errors.push("扱える範囲外の数値が含まれています。");
        else if (value !== null && typeof value === "object") Object.values(value).forEach(inspectNumbers);
      };
      inspectNumbers(weapon);
      const old = isLegacy(weapon);
      if (!old) {
        for (const key of ["common", "movement", "hit", "visual"]) if (!object(weapon.bulletParam[key])) errors.push("bulletParam." + key + "はオブジェクトが必要です。");
        if (!object(get(weapon, visual + "custom"))) errors.push("bulletParam.visual.customはオブジェクトが必要です（空の {} も可）。");
      }
      for (const field of allFields) {
        if (!has(weapon, field.path) || (old && field.path.startsWith("bulletParam."))) continue;
        const value = get(weapon, field.path);
        const type = field.kind;
        if ((type === "number" && typeof value !== "number") || ((type === "text" || type === "select") && typeof value !== "string") || (type === "boolean" && typeof value !== "boolean") || (type === "vector" && (!Array.isArray(value) || value.length !== field.axes.length || value.some(part => typeof part !== "number"))) || (type === "mask" && (!Array.isArray(value) || value.some(part => typeof part !== "string")))) errors.push(field.path + "の型または配列の要素数が不正です。");
      }
      return [...new Set(errors)];
    }
    function acceptWeapon(weapon, filename, source, handle = null, raw = null) {
      const errors = structuralErrors(weapon);
      if (errors.length) throw new Error(errors.join("\n"));
      data = clone(weapon); baseline = clone(weapon); baselineFileName = filename;
      $("fileName").value = filename; sourceLabel = source; fileHandle = handle; originalFileText = raw;
      legacy = isLegacy(data); removedValues.clear();
      drafts.clear(); activePage = "basic";
      if ((handle || raw !== null) && own(templates, filename)) $("templateSelect").value = filename;
      $("message").hidden = true;
      render(); refresh();
      window.scrollTo({top: 0, behavior: "instant"});
    }
    function addDefaults(paths) {
      for (const path of paths) if (!has(data, path)) put(data, path, fieldMap.get(path).value);
    }
    function applyModeDefaults(path) {
      if (path === move + "type" && homing(data)) addDefaults(sections[2].fields.slice(1).map(field => field.path));
      if (path === hit + "type") {
        if (direct(data)) addDefaults([hit + "environmentResponse"]);
        if (explosion(data)) addDefaults(sections[3].fields.filter(field => field.when === explosion).map(field => field.path));
      }
      if (path === visual + "type" && get(data, path) === "EFFECT") addDefaults([visual + "bulletEffectTag", visual + "effectColor"]);
      if (path === custom + "type" && scaleLerp(data)) addDefaults(sections[5].fields.slice(1).map(field => field.path));
    }
    function commit(field, value, rerender = false) {
      put(data, field.path, value);
      const row = document.querySelector('.field[data-path="' + field.path + '"]');
      if (row) { row.classList.remove("absent"); row.querySelector(".presence input").checked = true; }
      if (rerender) { applyModeDefaults(field.path); render(); }
      refresh();
    }
    function numericInput(field, value, onChange, id) {
      const input = make("input"); input.type = "number"; input.id = id;
      const scale = field.displayScale || 1;
      input.step = field.integer && !field.displayScale ? "1" : "any";
      if (field.min !== undefined) input.min = String(field.min / scale);
      if (field.max !== undefined) input.max = String(field.max / scale);
      input.required = true; input.value = String(field.displayScale ? Number((value / scale).toFixed(4)) : value);
      if (drafts.has(id)) { input.value = drafts.get(id).value; input.setCustomValidity(drafts.get(id).error); }
      input.dataset.fieldLabel = field.label;
      input.addEventListener("input", () => {
        const number = input.valueAsNumber;
        input.setCustomValidity("");
        if (!Number.isFinite(number)) input.setCustomValidity("数値を入力してください。");
        else if (field.integer && !Number.isSafeInteger(field.displayScale ? Math.round(number * scale) : number)) input.setCustomValidity("範囲内の整数を入力してください。");
        else if (field.positive && number <= 0) input.setCustomValidity("0より大きい数値を入力してください。");
        if (input.validity.valid) { drafts.delete(id); onChange(field.displayScale ? Math.round(number * scale) : number); }
        else { drafts.set(id, {value: input.value, error: input.validationMessage}); refresh(); }
      });
      visibleControls.push(input); return input;
    }
    function renderField(field, container) {
      const present = has(data, field.path);
      const value = present ? get(data, field.path) : field.value;
      const row = make("div", "field" + ((field.wide || field.kind === "mask") ? " wide" : "") + (present ? "" : " absent"));
      row.dataset.path = field.path;
      const id = "field-" + field.path.replaceAll(".", "-");
      const line = make("div", "label-line");
      const label = make("label", "field-label", field.label); label.htmlFor = id; line.append(label);
      const presence = make("label", "presence");
      const toggle = make("input"); toggle.type = "checkbox"; toggle.checked = present;
      toggle.setAttribute("aria-label", field.label + "をJSONに含める");
      presence.append(toggle, document.createTextNode("JSONに含める")); line.append(presence);
      toggle.addEventListener("change", () => {
        if (toggle.checked) { put(data, field.path, removedValues.has(field.path) ? removedValues.get(field.path) : field.value); applyModeDefaults(field.path); }
        else { removedValues.set(field.path, clone(get(data, field.path))); remove(data, field.path); }
        render(); refresh();
      });
      row.append(line, make("span", "json-key", field.path));
      const controls = make("div", "controls");
      if (field.kind === "number") {
        const shell = make("div", "input-shell" + (field.unit ? " has-unit" : ""));
        shell.append(numericInput(field, value, next => commit(field, next), id));
        if (field.unit) shell.append(make("span", "input-unit", field.unit));
        controls.append(shell);
      }
      else if (field.kind === "text") {
        const input = make("input"); input.type = "text"; input.id = id; input.value = value; input.spellcheck = false;
        const listId = tagListId(field.path); if ($(listId)) input.setAttribute("list", listId);
        input.addEventListener("input", () => commit(field, input.value)); controls.append(input);
      } else if (field.kind === "select") {
        const select = make("select"); select.id = id;
        if (!present) { const unset = make("option", "", "未設定（選択してください）"); unset.value = "__unset"; unset.disabled = true; select.append(unset); }
        for (const [key, title] of field.options) { const option = make("option", "", title.replace("（既存の綴り）", "")); option.value = key; select.append(option); }
        if (!field.options.some(([key]) => key === value)) { const option = make("option", "", "未対応の値 / " + value); option.value = value; select.append(option); }
        select.value = present ? value : "__unset"; select.addEventListener("change", () => commit(field, select.value, true));
        const choice = make("div", "choice-card");
        const mark = make("span", "choice-icon"); mark.append(icon(field.path.startsWith(move) ? "route" : field.path.startsWith(hit) ? "burst" : "spark"));
        choice.append(mark, select); controls.append(choice);
      } else if (field.kind === "boolean") {
        const wrap = make("label", "boolean"); const input = make("input"); input.type = "checkbox"; input.id = id; input.checked = value;
        const status = make("span", "", value ? "オン" : "オフ");
        input.addEventListener("change", () => { status.textContent = input.checked ? "オン" : "オフ"; commit(field, input.checked); });
        wrap.append(input, status); controls.append(wrap);
      } else if (field.kind === "vector") {
        if (field.axes[0] === "R") {
          const picker = make("input"); picker.type = "color"; picker.setAttribute("aria-label", field.label + "を選ぶ");
          const colorHex = values => {
            const divisor = field.max === 255 ? 255 : Math.max(1, ...values.slice(0,3));
            return "#" + values.slice(0,3).map(part => Math.min(255, Math.max(0, Math.round(part / divisor * 255))).toString(16).padStart(2,"0")).join("");
          };
          picker.value = colorHex(value);
          const colorRow = make("div", "color-control"); colorRow.append(picker, make("span", "", "クリックして色を選ぶ")); controls.append(colorRow);
          picker.addEventListener("input", () => {
            const current = get(data, field.path) ?? field.value;
            const brightness = field.max === 255 ? 255 : Math.max(1, ...current.slice(0,3));
            const rgb = [1,3,5].map(index => Number((parseInt(picker.value.slice(index,index+2),16) / 255 * brightness).toFixed(4)));
            commit(field, field.axes.length === 4 ? [...rgb, current[3]] : rgb);
            rgb.forEach((part,index) => { const control = $(id + "-" + index); control.value = String(part); control.setCustomValidity(""); drafts.delete(control.id); });
            refresh();
          });
        }
        const wrap = make("div", "vector");
        field.axes.forEach((axis, index) => {
          const component = make("label", "", axis);
          const input = numericInput(field, value[index], next => { const vectorValue = clone(get(data, field.path) ?? field.value); vectorValue[index] = next; commit(field, vectorValue); }, id + "-" + index);
          input.setAttribute("aria-label", field.label + " " + axis); if (!index) label.htmlFor = input.id;
          component.append(input); wrap.append(component);
        });
        controls.append(wrap);
      } else if (field.kind === "mask") {
        const wrap = make("div", "checkboxes");
        const options = [...field.options];
        for (const key of value) if (!options.some(([known]) => known === key)) options.push([key, "未対応の値"]);
        options.forEach(([key, title], index) => {
          const wrapLabel = make("label"); const input = make("input"); input.type = "checkbox"; input.id = id + "-" + index; input.checked = value.includes(key);
          if (!index) label.htmlFor = input.id;
          input.addEventListener("change", () => {
            const next = clone(get(data, field.path) ?? field.value);
            commit(field, input.checked ? [...next, key] : next.filter(part => part !== key));
          });
          wrapLabel.append(input, document.createTextNode(title)); wrap.append(wrapLabel);
        });
        controls.append(wrap);
      }
      for (const input of controls.querySelectorAll("input, select")) input.disabled = technicalMode && !present;
      row.append(controls);
      if (field.hint) row.append(make("span", "hint", field.hint));
      if (!present) row.append(make("span", "hint", "未指定の項目です。値を変更すると保存内容に追加されます。"));
      container.append(row);
    }
    function setPage(id, scroll = false) {
      activePage = id;
      document.querySelectorAll(".tab-panel").forEach(panel => { panel.hidden = panel.id !== "page-" + id; });
      document.querySelectorAll(".nav-button").forEach(button => {
        const selected = button.dataset.page === id; button.classList.toggle("active", selected);
        if (selected) button.setAttribute("aria-current", "page"); else button.removeAttribute("aria-current");
      });
      const index = pages.findIndex(page => page.id === id), page = pages[index];
      $("pageTitle").textContent = page.title; $("pageDescription").textContent = page.description;
      const next = pages[index + 1]; $("nextPageButton").hidden = !next;
      if (next) $("nextPageButton").textContent = next.title + "へ →";
      if (scroll) $("pageTitle").scrollIntoView({block: "start", behavior: "instant"});
    }
    function render() {
      visibleControls = []; $("editor").replaceChildren(); $("sectionNav").replaceChildren(); $("legacyNotice").hidden = !legacy;
      document.body.classList.toggle("show-technical", technicalMode);
      pages.forEach((page, index) => {
        const button = make("button", "nav-button"); button.type = "button"; button.dataset.page = page.id; button.id = "tab-" + page.id;
        button.disabled = legacy && page.id !== "basic";
        button.append(icon(page.icon), make("span", "", page.title), make("span", "nav-index", "0" + (index+1)));
        button.addEventListener("click", () => setPage(page.id, true)); $("sectionNav").append(button);
        const panel = make("div", "tab-panel"); panel.id = "page-" + page.id; panel.setAttribute("role", "region"); panel.setAttribute("aria-labelledby", button.id);
        if (page.id === "advanced") {
          const note = make("div", "advanced-note", "素材名や当たり判定など、細かい設定を調整できます。通常はベースの値のままで使えます。");
          const label = make("label"), toggle = make("input"); toggle.type = "checkbox"; toggle.id = "technicalToggle"; toggle.checked = technicalMode;
          label.append(toggle, document.createTextNode("JSONの項目名・出力設定を表示する")); note.append(label); panel.append(note);
          toggle.addEventListener("change", () => { technicalMode = toggle.checked; render(); refresh(); });
        }
        for (const group of page.groups) {
          if (group.when && !group.when(data)) continue;
          const fields = group.paths.map(path => fieldMap.get(path)).filter(field => field && (!legacy || !field.path.startsWith("bulletParam.")) && (!field.when || field.when(data)));
          if (!fields.length) continue;
          const card = make("section", "settings-panel");
          const title = make("div", "panel-heading"); title.append(icon(group.icon), make("h3", "", group.title)); card.append(title);
          const grid = make("div", "fields"); fields.forEach(field => renderField(field, grid)); card.append(grid); panel.append(card);
        }
        $("editor").append(panel);
      });
      setPage(activePage);
    }
    function filenameError() {
      const filename = $("fileName").value;
      if (!filename || filename.trim() !== filename || !/\.json$/i.test(filename) || /[<>:"/\\|?*\x00-\x1f]/.test(filename) || /[. ]\.json$/i.test(filename) || filename.length > 200 || /^(con|prn|aux|nul|com[1-9]|lpt[1-9])(?:\.|$)/i.test(filename) || filename.toLowerCase() === "surfacehitdata.json") return "保存名は有効な武器ファイル名（例: NewWeapon.json）にしてください。SurfaceHitData.jsonは指定できません。";
      return "";
    }
    function validationErrors() {
      const errors = [];
      if (legacy) errors.push("以前の形式の武器です。「今の形式に更新する」を押してください。");
      if (filenameError()) errors.push(filenameError());
      for (const field of allFields) {
        if (legacy && field.path.startsWith("bulletParam.")) continue;
        if (field.when && !field.when(data)) continue;
        if (!has(data, field.path)) { if (field.required) errors.push(field.label + "を選んでください。出力設定を変更した場合は、その項目を有効に戻してください。"); continue; }
        const value = get(data, field.path);
        if (field.kind === "number" || field.kind === "vector") {
          const values = field.kind === "vector" ? value : [value];
          if (values.some(part => !Number.isFinite(part) || (field.integer && !Number.isSafeInteger(part)) || (field.min !== undefined && part < field.min) || (field.max !== undefined && part > field.max) || (field.positive && part <= 0))) errors.push(field.label + "の数値が設定範囲外です。");
        }
        if (field.kind === "select" && !field.options.some(([key]) => key === value)) errors.push(field.label + "の値はゲームの読み込み処理に対応していません。");
        if (field.kind === "mask" && value.some(key => !categories.some(([known]) => known === key))) errors.push("衝突対象に未対応のカテゴリがあります。");
      }
      // The loader's fallback fireRate is zero and the gun divides by fireRate.
      if (!has(data, "fireRate")) errors.push("連射速度を設定してください。");
      for (const input of visibleControls) if (!input.disabled && !input.validity.valid) errors.push(input.dataset.fieldLabel + "の入力を確認してください。");
      return [...new Set(errors)];
    }
    function validationWarnings() {
      if (legacy) return [];
      const warnings = [];
      if (!get(data, "name")) warnings.push("武器名が空、または未指定です。");
      if (get(data, "bulletMaxNum") === 0) warnings.push("装弾数が0です。通常は1以上に設定してください。");
      if (get(data, common + "maxSpeed") < get(data, common + "speed")) warnings.push("最大速度が初速より小さく設定されています。");
      if (get(data, common + "aliveFrame") === 0) warnings.push("弾の寿命が0です。");
      if (get(data, visual + "type") === "EFFECT" && !get(data, visual + "bulletEffectTag")) warnings.push("EFFECT描画ですが弾のエフェクトタグが空です。");
      if (scaleLerp(data) && get(data, custom + "duration") === 0) warnings.push("サイズ変化にかける時間が0です。");
      if (homing(data) && !get(data, move + "lockOnRange")) warnings.push("誘導弾のロックオン距離が0、または未指定です。");
      if (homing(data) && !get(data, move + "targetingDuration")) warnings.push("誘導時間が0、または未指定のため、現在の実装では誘導しません。");
      if (get(data, visual + "enableFlightSmoke") && !get(data, visual + "smokeEffectTag")) warnings.push("飛行中の煙が有効ですが煙のタグが空です。");
      return warnings;
    }
    function refresh() {
      const errors = validationErrors(), warnings = validationWarnings();
      const changed = dirty(); $("dirtyBadge").textContent = changed ? "未保存の変更" : "変更なし";
      $("dirtyBadge").className = "badge" + (changed ? " dirty" : "");
      $("sourceLabel").textContent = sourceLabel;
      $("jsonPreview").textContent = serialize();
      const status = $("validation"); status.replaceChildren(); status.className = errors.length ? "invalid" : "valid";
      status.append(make("strong", "", errors.length ? "入力を確認してください" : "✓ 保存する準備ができました"));
      if (errors.length) { const list = make("ul"); errors.forEach(error => list.append(make("li", "", error))); status.append(list); }
      if (warnings.length) {
        const details = make("details", "warning"); details.append(make("summary", "", "気になる設定が" + warnings.length + "件あります"));
        const list = make("ul"); warnings.forEach(warning => list.append(make("li", "", warning))); details.append(list); status.append(details);
      }
      $("saveAsButton").disabled = busy || errors.length > 0;
      $("saveQuickButton").disabled = busy || errors.length > 0;
      $("overwriteButton").disabled = busy || errors.length > 0 || !fileHandle || $("fileName").value !== fileHandle.name;
      $("openButton").disabled = busy; $("templateButton").disabled = busy; $("resetButton").disabled = busy;
      $("libraryButton").disabled = busy;
      const damage = legacy ? get(data, "bulletParam.damage") : get(data, common + "damage");
      const rate = get(data, "fireRate"), simultaneous = get(data, "bulletSimultaneousNum") ?? 1;
      const format = number => Number.isFinite(number) ? new Intl.NumberFormat("ja-JP", {maximumFractionDigits: 3}).format(number) : "—";
      $("statDamage").textContent = format(damage); $("statRate").textContent = format(rate);
      $("statDps").textContent = format(damage * rate * simultaneous);
      $("statLife").textContent = format(legacy ? NaN : get(data, common + "aliveFrame") / 60);
      document.querySelectorAll(".tab-panel").forEach(panel => {
        const invalid = [...panel.querySelectorAll("input[type=number]")].some(input => !input.disabled && !input.validity.valid);
        $("tab-" + panel.id.replace("page-", "")).classList.toggle("has-issue", invalid);
      });
      $("fileName").setAttribute("aria-invalid", filenameError() ? "true" : "false");
      refreshSummary();
    }
    function weaponCategory(weapon, filename = "") {
      const type = get(weapon, common + "bulletType") ?? weapon.bulletType;
      if (type === "ACID" || /Acid/i.test(filename)) return "acid";
      if (type === "FLAME" || /Flame/i.test(filename) || /火炎/.test(weapon.name ?? "")) return "flame";
      if (type === "LASER" || /Laser/i.test(filename)) return "laser";
      if (get(weapon, hit + "type") === "EXPLOSION" || weapon.bulletType === "EXPLOSION" || /Launcher|Missile/i.test(filename)) return "launcher";
      if (/Sniper/i.test(filename) || (weapon.zoomLength >= 3 && weapon.fireRate <= 1)) return "sniper";
      if (/Shotgun/i.test(filename) || weapon.bulletSimultaneousNum >= 8) return "shotgun";
      return "rifle";
    }
    const categoryNames = {rifle: "ライフル", launcher: "ロケット", sniper: "スナイパー", shotgun: "ショットガン", laser: "レーザー", flame: "火炎放射器", acid: "酸の武器"};
    function weaponIllustration(kind) {
      const launcher = kind === "launcher";
      const long = kind === "sniper";
      const body = launcher ? '<path d="M60 58h156v33H60z" fill="#35505a"/><ellipse cx="216" cy="74.5" rx="8" ry="16.5" fill="#263d47"/><path d="M72 58v33M88 58v33M186 58v33M197 58v33M95 92l-8 27h18l8-27M146 92l9 25h15l-5-25"/><path d="M124 47h28v11M137 47v-9h35"/>' : '<path d="m48 79 40-7 6-14h90l12 11v18H90l-36 14-6-22Z" fill="#35505a"/><path d="M94 70h82M104 59v-7h54v7M117 88l-10 27h20l11-27M150 88l9 23h16l-7-23"/><path d="M183 69h' + (long ? '71' : '48') + 'v8h-' + (long ? '71' : '48') + 'M193 66v14M208 66v14"/><path d="M63 79l5 14M91 74v11M141 61v13M103 51h48"/>';
      return '<svg viewBox="0 0 320 160" fill="none" xmlns="http://www.w3.org/2000/svg" aria-hidden="true"><g transform="translate(9 11) rotate(-9 150 80)" stroke="currentColor" stroke-width="2" stroke-linejoin="round">' + body + '<path d="M104 68h42" stroke="#e1bc7c" stroke-width="3"/><path d="M' + (launcher ? '234' : long ? '267' : '244') + ' 73h17" stroke="#e1bc7c" stroke-dasharray="3 5"/></g></svg>';
    }
    function refreshSummary() {
      const name = get(data, "name") || "名前のない武器";
      $("currentWeaponName").textContent = name; $("summaryWeaponName").textContent = name;
      const kind = weaponCategory(data); $("weaponKind").textContent = categoryNames[kind];
      if ($("weaponArt").dataset.kind !== kind) { $("weaponArt").innerHTML = weaponIllustration(kind); $("weaponArt").dataset.kind = kind; }
      const ammo = get(data, "bulletMaxNum"), reload = get(data, "reloadTime");
      $("summaryDescription").textContent = "装弾数 " + (ammo === undefined ? "未指定" : ammo + "発") + " · リロード " + (reload === undefined ? "未指定" : reload + "秒");
      $("weaponPills").replaceChildren();
      for (const title of [homing(data) ? "誘導" : "直進", explosion(data) ? "爆発" : "直接命中", get(data, visual + "enableFlightSmoke") ? "煙あり" : ""]) if (title) $("weaponPills").append(make("span", "", title));
    }
    function renderLibrary() {
      const search = $("librarySearch").value.trim().toLowerCase(), filter = $("libraryFilter").value;
      const grid = $("libraryGrid"); grid.replaceChildren();
      for (const [filename, weapon] of Object.entries(templates)) {
        const kind = weaponCategory(weapon, filename);
        if (filter !== "all" && kind !== filter) continue;
        if (search && !(weapon.name + " " + filename).toLowerCase().includes(search)) continue;
        const card = make("button", "library-card" + ($("templateSelect").value === filename ? " selected" : "")); card.type = "button";
        card.append(icon("weapon"), make("strong", "", weapon.name), make("small", "", categoryNames[kind] + (isLegacy(weapon) ? " · 以前の形式" : "")));
        const damage = get(weapon, common + "damage") ?? get(weapon, "bulletParam.damage");
        card.append(make("span", "", "威力 " + damage + "　装弾数 " + weapon.bulletMaxNum));
        card.addEventListener("click", () => {
          if (busy || !confirmDiscard()) return;
          $("templateSelect").value = filename;
          acceptWeapon(weapon, "NewWeapon.json", "ベースの武器: " + filename);
          $("libraryDialog").close();
        });
        grid.append(card);
      }
      $("libraryEmpty").hidden = grid.childElementCount !== 0;
    }
    function convertLegacy() {
      const old = clone(data.bulletParam);
      const converted = clone(data);
      const defaults = clone(templates["AssultRifle01.json"].bulletParam);
      const mappedCommon = ["damage", "speed", "acceleration", "penetrationsCount", "collisionSize", "gravityScale", "knockbackForce", "collisionMask"];
      for (const key of mappedCommon) if (own(old, key)) { defaults.common[key] = clone(old[key]); delete old[key]; }
      defaults.common.maxSpeed = defaults.common.speed;
      const frames = Number.isFinite(old.range) && defaults.common.speed > 0 ? Math.max(1, Math.ceil(old.range / defaults.common.speed * 60)) : 60;
      if (!Number.isSafeInteger(frames) || frames > 2147483647) throw new Error("旧射程と速度から計算した寿命が大きすぎます。元のJSONの数値を確認してください。");
      defaults.common.aliveFrame = frames;
      defaults.common.bulletType = converted.bulletType ?? "NORMAL";
      for (const key of ["decalMaterialTag", "hitEffectTag"]) if (own(old, key)) { defaults.hit[key] = clone(old[key]); delete old[key]; }
      for (const key of ["bulletMaterialTag", "scale"]) if (own(old, key)) { defaults.visual[key] = clone(old[key]); delete old[key]; }
      converted.bulletParam = {...old, ...defaults};
      const errors = structuralErrors(converted); if (errors.length) throw new Error(errors.join("\n"));
      data = converted; legacy = false; render(); refresh();
      showMessage("現行形式に変換しました。弾の寿命・最大速度・判定用分類と見た目を確認してから保存してください。元ファイルは保存するまで変更されません。", "success");
    }
    function tagListId(path) { return "tags-" + path.split(".").at(-1); }
    function buildTagLists() {
      for (const field of allFields.filter(field => field.kind === "text" && /Tag$/.test(field.path))) {
        const id = tagListId(field.path); if ($(id)) continue;
        const values = new Set();
        for (const weapon of Object.values(templates)) { const value = get(weapon, field.path); if (typeof value === "string" && value) values.add(value); }
        const list = make("datalist"); list.id = id;
        [...values].sort().forEach(value => { const option = make("option"); option.value = value; list.append(option); });
        document.body.append(list);
      }
    }
    async function importFile(file, handle = null) {
      try {
        const raw = await file.text();
        const weapon = JSON.parse(raw.replace(/^\uFEFF/, ""));
        const errors = structuralErrors(weapon); if (errors.length) throw new Error(errors.join("\n"));
        if (!confirmDiscard()) return;
        acceptWeapon(weapon, file.name, "読み込み元: " + file.name, handle, raw);
        showMessage(file.name + "を読み込みました。" + (handle ? "上書き保存も使えます。" : "変更後はJSONを保存してください。"), "success");
      } catch (error) { showMessage("読み込めませんでした。\n" + error.message, "error"); }
    }
    async function openFile() {
      if (typeof window.showOpenFilePicker === "function" && window.isSecureContext) {
        try {
          const [handle] = await window.showOpenFilePicker({types: [{description: "武器JSON", accept: {"application/json": [".json"]}}], multiple: false});
          await importFile(await handle.getFile(), handle); return;
        } catch (error) {
          if (error.name === "AbortError") return;
          if (!["SecurityError", "NotAllowedError"].includes(error.name)) { showMessage("ファイルを開けませんでした。\n" + error.message, "error"); return; }
        }
      }
      $("fileInput").click();
    }
    function downloadJson(content, filename) {
      const url = URL.createObjectURL(new Blob([content], {type: "application/json;charset=utf-8"}));
      const link = make("a"); link.href = url; link.download = filename; document.body.append(link); link.click(); link.remove();
      setTimeout(() => URL.revokeObjectURL(url), 1000);
    }
    async function saveJson(overwrite = false) {
      if (busy) return;
      const errors = validationErrors(); if (errors.length) { showMessage(errors.join("\n"), "error"); return; }
      const content = serialize(), snapshot = clone(data), filename = $("fileName").value;
      busy = true; refresh();
      try {
        let target = overwrite ? fileHandle : null;
        if (overwrite) {
          if (!target || filename !== target.name) throw new Error("別名保存を使用してください。");
          const latest = await (await target.getFile()).text();
          if (originalFileText !== null && latest !== originalFileText) throw new Error("読み込み後に元ファイルが変更されています。既存JSONを開き直すか、別名で保存してください。");
        } else if (typeof window.showSaveFilePicker === "function" && window.isSecureContext) {
          try { target = await window.showSaveFilePicker({suggestedName: filename, types: [{description: "武器JSON", accept: {"application/json": [".json"]}}]}); }
          catch (error) { if (!["SecurityError", "NotAllowedError"].includes(error.name)) throw error; }
        }
        if (target) {
          if (target.name.toLowerCase() === "surfacehitdata.json" || !/\.json$/i.test(target.name)) throw new Error("武器用の.jsonファイルを選んでください。SurfaceHitData.jsonには保存できません。");
          // Keep the editor editable while the picker is open; the snapshot is what is written.
          const writer = await target.createWritable();
          try { await writer.write(content); await writer.close(); }
          catch (error) { try { await writer.abort(); } catch (_) {} throw error; }
          fileHandle = target; originalFileText = content;
          if ($("fileName").value === filename) $("fileName").value = target.name;
          baseline = snapshot; baselineFileName = target.name; sourceLabel = "保存先: " + target.name;
          showMessage(target.name + "を保存しました。", "success");
        } else {
          downloadJson(content, filename);
          showMessage(filename + "のダウンロードを開始しました。完了後、WeaponsDataフォルダに保存・移動してください。", "success");
          // A download request does not prove that the browser saved the file.
        }
      } catch (error) { if (error.name !== "AbortError") showMessage("保存できませんでした。\n" + error.message, "error"); }
      finally { busy = false; refresh(); }
    }
    $("templateButton").addEventListener("click", () => {
      if (!confirmDiscard()) return;
      const name = $("templateSelect").value;
      acceptWeapon(templates[name], "NewWeapon.json", "同梱テンプレート: " + name + "（HTML作成時のデータ）");
    });
    $("openButton").addEventListener("click", openFile);
    $("fileInput").addEventListener("change", async event => { const file = event.target.files[0]; if (file) await importFile(file); event.target.value = ""; });
    $("saveAsButton").addEventListener("click", () => saveJson());
    $("saveQuickButton").addEventListener("click", () => saveJson());
    $("nextPageButton").addEventListener("click", () => {
      const next = pages[pages.findIndex(page => page.id === activePage) + 1]; if (next) setPage(next.id, true);
    });
    $("libraryButton").addEventListener("click", () => { renderLibrary(); $("libraryDialog").showModal(); });
    $("librarySearch").addEventListener("input", renderLibrary);
    $("libraryFilter").addEventListener("change", renderLibrary);
    $("helpButton").addEventListener("click", () => $("helpDialog").showModal());
    document.querySelectorAll("[data-close]").forEach(button => button.addEventListener("click", () => $(button.dataset.close).close()));
    $("overwriteButton").addEventListener("click", () => saveJson(true));
    $("convertButton").addEventListener("click", () => { try { convertLegacy(); } catch (error) { showMessage(error.message, "error"); } });
    $("fileName").addEventListener("input", refresh);
    $("resetButton").addEventListener("click", () => {
      if (!confirmDiscard()) return;
      data = clone(baseline); $("fileName").value = baselineFileName; legacy = isLegacy(data); removedValues.clear(); drafts.clear(); activePage = "basic"; render(); refresh(); showMessage("読み込み時、または最後にファイル保存した時点の値に戻しました。");
    });
    $("editor").addEventListener("submit", event => event.preventDefault());
    window.addEventListener("beforeunload", event => { if (dirty()) { event.preventDefault(); event.returnValue = ""; } });
    document.addEventListener("dragover", event => { if (event.dataTransfer.types.includes("Files")) { event.preventDefault(); document.body.classList.add("dragging"); } });
    document.addEventListener("dragleave", event => { if (!event.relatedTarget) document.body.classList.remove("dragging"); });
    document.addEventListener("drop", event => {
      if (!event.dataTransfer.files.length) return;
      event.preventDefault(); document.body.classList.remove("dragging");
      if (busy) return;
      if (event.dataTransfer.files.length !== 1) { showMessage("武器JSONを1ファイルずつ読み込んでください。", "error"); return; }
      importFile(event.dataTransfer.files[0]);
    });
    for (const [filename, weapon] of Object.entries(templates)) {
      const option = make("option", "", weapon.name); option.value = filename; $("templateSelect").append(option);
    }
    document.querySelectorAll("[data-icon]").forEach(element => element.append(icon(element.dataset.icon)));
    buildTagLists();
    $("templateSelect").value = "AssultRifle01.json";
    acceptWeapon(templates["AssultRifle01.json"], "NewWeapon.json", "同梱テンプレート: AssultRifle01.json（HTML作成時のデータ）");
