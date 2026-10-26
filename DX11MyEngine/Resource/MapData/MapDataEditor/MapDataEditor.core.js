/* Data-only operations are shared by the browser UI and the Node regression checks. */
(function (root) {
  "use strict";
  const definitions = {
    physics: { file: "MAP_OBJECT_PHYSICS_PRESET.json", key: "physics", id: "presetID", label: "物理プリセット", english: "PHYSICS", icon: "◈" },
    assets: { file: "MAP_OBJECT_ASSETS.json", key: "objects", id: "assetID", label: "オブジェクト定義", english: "OBJECT ASSETS", icon: "▧" },
    fragments: { file: "BUILDING_FRAGMENT_DATA.json", key: "fragmentData", id: "assetID", label: "建物の破片", english: "BUILDING FRAGMENTS", icon: "◇" }
  };
  const categories = ["PLAYER", "PLAYER_BULLET", "ENEMY", "ENEMY_BULLET", "DESTRUCTION_BUILDING", "BUILDING", "EVERY", "ITEM", "PROP"];
  const surfaces = ["DEFAULT", "CONCRETE", "METAL", "SOIL", "ANT"];
  const colliderTypes = ["NONE", "BOX", "SPHERE", "CAPSULE", "CYLINDER", "CONVEX_HULL", "CONCAVE_TRIANGLE"];
  const objectTypes = ["PROP", "BUILDING", "EXPLOSION_BUILDING"];
  const isObject = value => value !== null && typeof value === "object" && !Array.isArray(value);
  const clone = value => JSON.parse(JSON.stringify(value));
  const rows = (kind, data) => data[definitions[kind].key];

  // Refuse malformed containers before they reach the form. Scalar errors remain editable.
  function parse(text) {
    const data = JSON.parse(text.replace(/^\uFEFF/, ""));
    if (!isObject(data)) throw new Error("JSONの一番外側はオブジェクトである必要があります。");
    const matches = Object.keys(definitions).filter(k => Object.hasOwn(data, definitions[k].key));
    if (matches.length !== 1) throw new Error("physics / objects / fragmentData のいずれか1種類を持つJSONを選択してください。");
    const kind = matches[0];
    const array = rows(kind, data);
    if (!Array.isArray(array)) throw new Error(`${definitions[kind].key} は配列である必要があります。`);
    const objectArray = (value, path) => {
      if (!Array.isArray(value) || value.some(item => !isObject(item))) throw new Error(`${path} はオブジェクトの配列である必要があります。`);
    };
    objectArray(array, definitions[kind].key);
    array.forEach((entry, i) => {
      if (kind === "physics") {
        if (!isObject(entry.rigidBody)) throw new Error(`physics[${i}].rigidBody がありません、または形式が不正です。`);
        objectArray(entry.colliders, `physics[${i}].colliders`);
        if (entry.collisionMask !== undefined && !Array.isArray(entry.collisionMask)) throw new Error(`physics[${i}].collisionMask は配列にしてください。`);
      }
      if (kind === "assets") objectArray(entry.materials, `objects[${i}].materials`);
      if (kind === "fragments") objectArray(entry.fragments, `fragmentData[${i}].fragments`);
    });
    // JSON.parse accepts 1e400 as Infinity; never let serialization silently turn it into null.
    const finite = value => {
      if (typeof value === "number" && !Number.isFinite(value)) throw new Error("数値が大きすぎます。有限の数値にしてください。");
      if (value && typeof value === "object") Object.values(value).forEach(finite);
    };
    finite(data);
    return { kind, data };
  }

  function collider() { return { type: "BOX", height: 0, radius: 0, center: [0, 0, 0], size: [1, 1, 1], surfaceType: "DEFAULT" }; }
  function fragment() { return { modelPath: "", physicsPresetID: "", position: [0, 0, 0], rotation: [0, 0, 0], scale: [1, 1, 1] }; }
  function newEntry(kind, id) {
    if (kind === "physics") return { presetID: id, rigidBody: { mass: 1, restitution: 0, friction: 0.5, gravityScale: 1, linearDamping: 0, angularDamping: 0, linearFactor: [1, 1, 1], angularFactor: [1, 1, 1] }, collisionCategory: "PROP", collisionMask: ["PLAYER", "ENEMY", "BUILDING"], colliders: [collider()] };
    if (kind === "assets") return { assetID: id, physicsPresetID: "", modelPath: "", mapObjectType: "PROP", materials: [{ slot: 0, tag: "" }], isGrabbable: false, isDestruction: true, health: 10 };
    return { assetID: id, fragments: [fragment()] };
  }
  function uniqueID(kind, data, base) {
    const used = new Set(rows(kind, data).map(e => e[definitions[kind].id]));
    let id = base, n = 2;
    while (used.has(id)) id = `${base}_${n++}`;
    return id;
  }
  function references(documents, kind, id) {
    const found = [];
    if (kind === "physics") {
      if (documents.assets) rows("assets", documents.assets).forEach((e, index) => { if (e.physicsPresetID === id) found.push({ kind: "assets", index, object: e, key: "physicsPresetID" }); });
      if (documents.fragments) rows("fragments", documents.fragments).forEach((e, index) => e.fragments.forEach(f => { if (f.physicsPresetID === id) found.push({ kind: "fragments", index, object: f, key: "physicsPresetID" }); }));
    }
    if (kind === "assets" && documents.fragments) rows("fragments", documents.fragments).forEach((e, index) => { if (e.assetID === id) found.push({ kind: "fragments", index, object: e, key: "assetID" }); });
    return found;
  }
  function rename(documents, kind, index, nextID) {
    const data = documents[kind], key = definitions[kind].id, entry = rows(kind, data)[index], oldID = entry[key];
    if (rows(kind, data).some((e, i) => i !== index && e[key] === nextID)) throw new Error("同じIDが既に存在します。別のIDを指定してください。");
    const uniqueOldID = rows(kind, data).filter(e => e[key] === oldID).length === 1;
    const refs = oldID && uniqueOldID ? references(documents, kind, oldID) : [];
    entry[key] = nextID;
    refs.forEach(r => { r.object[r.key] = nextID; });
    return refs.length;
  }

  function validate(documents) {
    const issues = [];
    Object.entries(documents).forEach(([kind, data]) => {
      if (!data) return;
      const seen = new Set();
      rows(kind, data).forEach((entry, index) => {
        const add = (path, message, level = "error") => issues.push({ kind, index, path, message, level });
        const string = (value, path) => { if (typeof value !== "string" || !value.trim()) add(path, "文字列を入力してください。"); };
        const number = (value, path, min = -Infinity, max = Infinity, optional = false) => {
          if (optional && value === undefined) return;
          if (typeof value !== "number" || !Number.isFinite(value) || value < min || value > max) add(path, `数値${min !== -Infinity ? ` (${min}以上` : ""}${max !== Infinity ? `・${max}以下` : ""}${min !== -Infinity ? ")" : ""}を入力してください。`);
        };
        const vector = (value, path, min = -Infinity, max = Infinity) => {
          if (!Array.isArray(value) || value.length !== 3 || value.some(n => typeof n !== "number" || !Number.isFinite(n) || n < min || n > max)) add(path, `XYZの3つの有効な数値を指定してください${min === 0 ? "（0以上）" : ""}。`);
        };
        const choice = (value, path, options) => { if (!options.includes(value)) add(path, "ゲーム側で定義されている値を選択してください。"); };
        const reference = (value, path, targetKind) => {
          string(value, path);
          if (!value) return;
          if (!documents[targetKind]) add(path, `${definitions[targetKind].label}が未読み込みのため、参照先を確認できません。`, "warning");
          else if (!rows(targetKind, documents[targetKind]).some(e => e[definitions[targetKind].id] === value)) add(path, `参照先「${value}」が存在しません。`);
        };
        const pathString = (value, path) => {
          string(value, path);
          if (typeof value === "string" && value && (!value.startsWith("Resource/") || value.includes("\\") || value.split("/").includes(".."))) add(path, "Resource/ から始まる相対パスを推奨します。ファイルの存在はゲーム側で確認してください。", "warning");
        };
        const key = definitions[kind].id;
        string(entry[key], key);
        if (seen.has(entry[key])) add(key, `ID「${entry[key]}」が重複しています。`);
        seen.add(entry[key]);
        if (kind === "physics") {
          const rb = entry.rigidBody;
          ["mass", "friction", "restitution"].forEach(k => number(rb[k], `rigidBody.${k}`, 0, Infinity, true));
          number(rb.gravityScale, "rigidBody.gravityScale", -Infinity, Infinity, true);
          ["linearDamping", "angularDamping"].forEach(k => number(rb[k], `rigidBody.${k}`, 0, 1, true));
          ["linearFactor", "angularFactor"].forEach(k => { if (rb[k] !== undefined) vector(rb[k], `rigidBody.${k}`, 0, 1); });
          choice(entry.collisionCategory, "collisionCategory", categories);
          (entry.collisionMask || []).forEach((c, i) => choice(c, `collisionMask[${i}]`, categories));
          if (!(entry.collisionMask || []).length) add("collisionMask", "衝突対象が選択されていません。", "warning");
          if (entry.colliders.length > 1) add("colliders", "現在のゲーム側は1プリセットにつきCollider 1個までにしてください。");
          entry.colliders.forEach((c, i) => {
            const p = `colliders[${i}]`;
            choice(c.type, `${p}.type`, colliderTypes);
            if (c.type === "NONE") add(`${p}.type`, "NONEの生成経路は未初期化RigidBodyを作るため、現在は保存できません。判定なしの場合はColliderを削除してください。");
            if (c.center !== undefined) vector(c.center, `${p}.center`);
            if (c.size !== undefined) vector(c.size, `${p}.size`, 0);
            if (c.surfaceType !== undefined) choice(c.surfaceType, `${p}.surfaceType`, surfaces);
            if (c.type === "BOX") {
              vector(c.size, `${p}.size`, 0);
              if (Array.isArray(c.size) && c.size.some(n => n <= 0)) add(`${p}.size`, "箱の各サイズは0より大きくしてください。");
            }
            if (["SPHERE", "CAPSULE", "CYLINDER"].includes(c.type)) {
              number(c.radius, `${p}.radius`, 0);
              if (c.radius === 0) add(`${p}.radius`, "半径は0より大きくしてください。");
            }
            if (["CAPSULE", "CYLINDER"].includes(c.type)) {
              number(c.height, `${p}.height`, 0);
              if (c.height === 0) add(`${p}.height`, "高さは0より大きくしてください。");
            }
          });
        } else if (kind === "assets") {
          reference(entry.physicsPresetID, "physicsPresetID", "physics");
          pathString(entry.modelPath, "modelPath");
          choice(entry.mapObjectType, "mapObjectType", objectTypes);
          ["isGrabbable", "isDestruction"].forEach(k => { if (typeof entry[k] !== "boolean") add(k, "ON / OFFを指定してください。"); });
          number(entry.health, "health", 0);
          const slots = new Set();
          entry.materials.forEach((m, i) => {
            if (!Number.isInteger(m.slot) || m.slot < 0) add(`materials[${i}].slot`, "スロットは0以上の整数にしてください。");
            if (slots.has(m.slot)) add(`materials[${i}].slot`, "マテリアルスロットが重複しています。");
            slots.add(m.slot);
            string(m.tag, `materials[${i}].tag`);
          });
          if (entry.isDestruction && entry.mapObjectType === "BUILDING") {
            if (!documents.fragments) add("assetID", "破片データが未読み込みのため、破壊時の参照先を確認できません。", "warning");
            else if (!rows("fragments", documents.fragments).some(f => f.assetID === entry.assetID)) add("assetID", "破壊可能な建物に対応する破片データがありません。");
          }
        } else {
          reference(entry.assetID, "assetID", "assets");
          if (!entry.fragments.length) add("fragments", "破片が0個です。破壊時に置き換えるモデルがありません。", "warning");
          entry.fragments.forEach((f, i) => {
            const p = `fragments[${i}]`;
            pathString(f.modelPath, `${p}.modelPath`);
            reference(f.physicsPresetID, `${p}.physicsPresetID`, "physics");
            vector(f.position, `${p}.position`);
            vector(f.rotation, `${p}.rotation`);
            vector(f.scale, `${p}.scale`, 0);
            if (Array.isArray(f.scale) && f.scale.some(n => n <= 0)) add(`${p}.scale`, "拡縮は各軸0より大きくしてください。");
          });
        }
      });
    });
    return issues;
  }
  const api = { definitions, categories, surfaces, colliderTypes, objectTypes, clone, rows, parse, collider, fragment, newEntry, uniqueID, references, rename, validate };
  if (typeof module !== "undefined" && module.exports) module.exports = api;
  else root.MapDataCore = api;
})(typeof globalThis !== "undefined" ? globalThis : this);
