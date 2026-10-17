/* Buildless, offline UI: no network libraries and no automatic disk writes. */
(() => {
  "use strict";
  const C = window.MapDataCore, D = C.definitions;
  const $ = id => document.getElementById(id);
  const state = { docs: {}, kind: "physics", selected: { physics: 0, assets: 0, fragments: 0 }, past: [], future: [], busy: false };
  const json = value => JSON.stringify(value);
  const documents = () => Object.fromEntries(Object.entries(state.docs).map(([k, doc]) => [k, doc.data]));
  const currentDoc = () => state.docs[state.kind];
  const entries = () => currentDoc() ? C.rows(state.kind, currentDoc().data) : [];
  const currentEntry = () => entries()[state.selected[state.kind]];
  const dirty = doc => json(doc.data) !== doc.baseline;
  const snapshot = () => ({ data: C.clone(documents()), selected: { ...state.selected }, kind: state.kind });
  function restore(s) {
    Object.entries(s.data).forEach(([k, data]) => { state.docs[k].data = C.clone(data); });
    state.selected = { ...s.selected }; state.kind = s.kind;
    render();
  }
  function mutate(action, redraw = true) {
    const before = snapshot();
    try { action(); } catch (error) { notify(error.message, true); render(); return; }
    if (json(before.data) !== json(documents())) {
      state.past.push(before);
      if (state.past.length > 60) state.past.shift();
      state.future = [];
    }
    if (redraw) render(); else refresh();
  }
  function undo() { if (state.busy || !state.past.length) return; state.future.push(snapshot()); restore(state.past.pop()); }
  function redo() { if (state.busy || !state.future.length) return; state.past.push(snapshot()); restore(state.future.pop()); }
  function notify(message, error = false) { $("notice").hidden = false; $("notice").textContent = message; $("notice").className = error ? "error" : ""; }
  function node(tag, className, text) { const e = document.createElement(tag); if (className) e.className = className; if (text !== undefined) e.textContent = text; return e; }
  function button(text, action, className = "") { const b = node("button", className, text); b.type = "button"; b.addEventListener("click", action); return b; }
  async function operation(action) {
    if (state.busy) return;
    state.busy = true;
    document.querySelector(".workspace").inert = true;
    $("open-files").disabled = true; $("open-folder").disabled = true;
    try { await action(); } catch (error) { if (error.name !== "AbortError") notify(error.message || String(error), true); }
    finally {
      state.busy = false; document.querySelector(".workspace").inert = false;
      $("open-files").disabled = false; $("open-folder").disabled = !window.showDirectoryPicker;
      refresh();
    }
  }
  const idLabel = (kind, entry) => typeof entry[D[kind].id] === "string" && entry[D[kind].id] ? entry[D[kind].id] : "（ID未入力）";

  function renderList() {
    $("tabs").replaceChildren();
    for (const [kind, def] of Object.entries(D)) {
      const b = button("", () => { state.kind = kind; $("search").value = ""; render(); }, `tab${state.kind === kind ? " active" : ""}`);
      b.setAttribute("aria-current", state.kind === kind ? "page" : "false");
      b.append(node("span", "tab-icon", def.icon));
      const text = node("span", "tab-text", def.label);
      text.append(node("small", "", state.docs[kind] ? `${C.rows(kind, state.docs[kind].data).length} 件 · 読み込み済み` : "未読み込み"));
      b.append(text);
      if (state.docs[kind] && dirty(state.docs[kind])) { const dot = node("span", "tab-dot", "●"); dot.title = "元ファイルに未保存"; b.append(dot); }
      $("tabs").append(b);
    }
    $("list-title").textContent = D[state.kind].label;
    $("item-count").textContent = entries().length;
    $("entry-list").replaceChildren();
    const q = $("search").value.toLowerCase();
    entries().forEach((entry, index) => {
      const id = idLabel(state.kind, entry);
      if (!id.toLowerCase().includes(q)) return;
      const b = button("", () => { state.selected[state.kind] = index; render(); }, `entry${index === state.selected[state.kind] ? " active" : ""}`);
      b.append(node("strong", "", id));
      b.append(node("small", "", state.kind === "physics" ? `${entry.colliders.length} collider · ${entry.collisionCategory || "未設定"}` : state.kind === "assets" ? `${entry.mapObjectType || "未設定"} · ${entry.physicsPresetID || "プリセット未設定"}` : `${entry.fragments.length} fragments`));
      $("entry-list").append(b);
    });
    if (!$("entry-list").childElementCount) $("entry-list").append(node("p", "list-empty", !currentDoc() ? "JSONファイルを開くと、ここに一覧が表示されます。" : q ? "一致するデータがありません。" : "データがありません。新規作成から追加できます。"));
    $("add-entry").disabled = !currentDoc();
  }

  function refresh() {
    renderList();
    const doc = currentDoc();
    $("file-name").textContent = doc ? doc.name : D[state.kind].file;
    $("document-status").textContent = !doc ? "未読み込み" : !dirty(doc) ? "変更なし" : doc.exported === json(doc.data) ? "書き出し済み / 元ファイル未保存" : "未保存の変更";
    $("document-status").className = `status-chip${doc && dirty(doc) ? " dirty" : ""}`;
    if (currentEntry()) $("entry-title").textContent = idLabel(state.kind, currentEntry());
    $("undo").disabled = !state.past.length; $("redo").disabled = !state.future.length;
    const issues = C.validate(documents()), errors = issues.filter(i => i.level === "error");
    const warnings = issues.length - errors.length;
    const loaded = Object.keys(state.docs).length;
    $("validation-summary").textContent = !loaded ? "読み込み待ち" : errors.length ? `${errors.length} 件のエラー · ${warnings} 件の注意` : warnings ? `エラーなし · ${warnings} 件の注意` : "✓ データの整合性に問題はありません";
    $("validation-summary").className = `validation-summary${errors.length ? " has-errors" : ""}`;
    $("validation-list").replaceChildren();
    issues.forEach(issue => {
      const b = button("", () => {
        state.kind = issue.kind; state.selected[issue.kind] = issue.index; $("search").value = ""; render();
        const input = Array.from($("editor-form").querySelectorAll("[data-path]")).find(e => e.dataset.path === issue.path);
        if (input) { input.focus(); input.scrollIntoView({ block: "center", behavior: "smooth" }); }
      }, `issue ${issue.level}`);
      b.append(node("small", "", `${D[issue.kind].label} / ${idLabel(issue.kind, C.rows(issue.kind, state.docs[issue.kind].data)[issue.index])}`));
      b.append(node("strong", "", issue.path), node("div", "", issue.message));
      $("validation-list").append(b);
    });
    $("save").disabled = !doc || !!errors.length;
    $("download").disabled = !doc || !!errors.length;
    $("save").textContent = doc?.handle ? "元ファイルへ保存" : "保存先を選んで保存";
    if (!window.showSaveFilePicker && !doc?.handle) { $("save").disabled = true; $("save").title = "このブラウザではJSONを書き出すをご利用ください。"; }
    $("save-summary").textContent = !doc ? "ファイルを開いて編集を始めましょう" : errors.length ? "エラーを修正してから保存してください" : dirty(doc) ? "編集した内容を保存できます" : "読み込み済みのデータを編集できます";
    $("save-hint").textContent = errors.length ? "読み込み済みの全タブを検証しています。右側の項目から移動できます。" : "ダウンロード書き出しでは元ファイルは更新されません。";
  }

  function section(title, number, parent = $("editor-form"), addAction) {
    const box = node("section", "form-section");
    const head = node("div", "section-head"), h = node("h3");
    h.append(node("span", "section-number", number), document.createTextNode(title));
    head.append(h); if (addAction) head.append(button("＋ 追加", addAction));
    const body = node("div", "section-body"); box.append(head, body); parent.append(box); return body;
  }
  function subCard(parent, title, remove, duplicate) {
    const box = node("div", "sub-card"), head = node("div", "section-head"); head.append(node("h3", "", title));
    const actions = node("div", "entry-actions");
    if (duplicate) actions.append(button("複製", duplicate));
    actions.append(button("削除", remove, "danger-quiet")); head.append(actions);
    const body = node("div", "section-body"); box.append(head, body); parent.append(box); return body;
  }
  function field(parent, object, key, label, options = {}) {
    const wrap = node("label", `field${options.wide ? " wide" : ""}`);
    const caption = node("span", "field-label", label); caption.append(node("span", "field-key", key)); wrap.append(caption);
    let input;
    if (options.choices) {
      input = node("select");
      input.append(new Option("選択してください", ""));
      const choices = options.choices;
      choices.forEach(v => { const value = Array.isArray(v) ? v[0] : v; input.append(new Option(Array.isArray(v) ? v[1] : v, value)); });
      const current = object[key];
      if (current && !choices.some(v => (Array.isArray(v) ? v[0] : v) === current)) input.append(new Option(`${current}（参照先未確認）`, current));
      input.value = current ?? "";
    } else {
      input = node("input"); input.type = options.number ? "number" : "text";
      input.value = object[key] ?? "";
      if (options.number) { input.step = options.step ?? "any"; if (options.min !== undefined) input.min = options.min; if (options.max !== undefined) input.max = options.max; }
      if (options.placeholder) input.placeholder = options.placeholder;
      if (options.suggestions) {
        const id = `suggest-${field.next++}`, list = node("datalist"); list.id = id;
        options.suggestions.forEach(v => list.append(new Option(v, v)));
        input.setAttribute("list", id); wrap.append(list);
      }
    }
    input.dataset.path = options.path || key;
    input.setAttribute("aria-label", options.aria || label);
    input.addEventListener("change", () => {
      const value = options.number ? input.value === "" || !Number.isFinite(input.valueAsNumber) ? null : input.valueAsNumber : input.value;
      mutate(() => { if (options.update) options.update(value); else object[key] = value; }, !!options.redraw);
    });
    wrap.append(input); if (options.help) wrap.append(node("small", "", options.help)); parent.append(wrap); return input;
  }
  field.next = 0;
  function vector(parent, object, key, label, path = key, help = "", min, max) {
    const wrap = node("div", "field wide"), caption = node("span", "field-label", label);
    caption.append(node("span", "field-key", key)); wrap.append(caption);
    const group = node("div", "vector");
    ["X", "Y", "Z"].forEach((axis, index) => {
      const lab = node("label", "axis-field"), input = node("input"); input.type = "number"; input.step = "any";
      input.value = Array.isArray(object[key]) ? object[key][index] ?? "" : "";
      input.setAttribute("aria-label", `${label} ${axis}`); input.dataset.path = path;
      if (min !== undefined) input.min = min; if (max !== undefined) input.max = max;
      input.addEventListener("change", () => mutate(() => {
        if (!Array.isArray(object[key]) || object[key].length !== 3) object[key] = [null, null, null];
        object[key][index] = input.value === "" || !Number.isFinite(input.valueAsNumber) ? null : input.valueAsNumber;
      }, false));
      lab.append(node("span", "", axis), input); group.append(lab);
    });
    wrap.append(group); if (help) wrap.append(node("small", "", help)); parent.append(wrap);
  }
  function check(parent, object, key, label) {
    const lab = node("label", "check-field"), input = node("input"); input.type = "checkbox"; input.checked = object[key] === true;
    input.dataset.path = key; input.addEventListener("change", () => mutate(() => { object[key] = input.checked; }, false));
    lab.append(input, document.createTextNode(label)); parent.append(lab);
  }
  function referenceChoices(kind) { return state.docs[kind] ? C.rows(kind, state.docs[kind].data).map(e => e[D[kind].id]).filter(v => typeof v === "string" && v) : []; }
  function referenceField(parent, obj, key, label, kind, options = {}) {
    const choices = referenceChoices(kind);
    return field(parent, obj, key, label, { ...options, ...(state.docs[kind] ? { choices: [...new Set(choices)] } : {}), help: state.docs[kind] ? options.help : `${D[kind].label}を読み込むと一覧から選択できます。` });
  }
  function idField(parent, entry) {
    const kind = state.kind, key = D[kind].id;
    const update = value => {
      if (!value.trim()) throw new Error("IDは空にできません。");
      const count = C.rename(documents(), kind, state.selected[kind], value);
      notify(`IDを変更しました。読み込み済みデータの参照 ${count} 件を更新しました。${kind === "assets" ? " 配置JSON（MAP_MOUNTAINS_Aなど）のassetIDは別途更新してください。" : " 未読み込みファイルの参照は変更されません。"}`);
    };
    if (kind === "fragments") referenceField(parent, entry, key, "対象の建物アセット", "assets", { wide: true, update, redraw: true });
    else field(parent, entry, key, kind === "physics" ? "プリセットID" : "アセットID", { wide: true, update, redraw: true, help: "読み込み済みの関連データも同時に更新します。変更された各タブを保存してください。" });
  }

  function renderPhysics(entry) {
    let body = section("基本情報", "01"); idField(body, entry);
    field(body, entry, "collisionCategory", "自身の衝突カテゴリ", { wide: true, choices: C.categories });
    body = section("動き・物理特性", "02");
    const values = [["mass", "質量", "配置直後はゲーム側で0に固定され、破壊時などに利用されます。"], ["friction", "摩擦係数", ""], ["restitution", "反発係数", "0で跳ね返りなし。1を超える値も保持します。"], ["gravityScale", "重力倍率", "1が基準、0で重力なし。"], ["linearDamping", "移動の減衰", "0〜1。大きいほど移動が減衰します。"], ["angularDamping", "回転の減衰", "0〜1。大きいほど回転が減衰します。"]];
    values.forEach(([key, label, help]) => field(body, entry.rigidBody, key, label, { number: true, min: key === "gravityScale" ? undefined : 0, max: key.endsWith("Damping") ? 1 : undefined, help, path: `rigidBody.${key}` }));
    vector(body, entry.rigidBody, "linearFactor", "軸ごとの移動倍率", "rigidBody.linearFactor", "0で固定、1で制限なし。", 0, 1);
    vector(body, entry.rigidBody, "angularFactor", "軸ごとの回転倍率", "rigidBody.angularFactor", "0で固定、1で制限なし。", 0, 1);
    body = section("衝突対象", "03");
    const grid = node("div", "mask-grid wide");
    [...new Set([...C.categories, ...(entry.collisionMask || [])])].forEach(category => {
      const lab = node("label"), input = node("input"); input.type = "checkbox"; input.checked = (entry.collisionMask || []).includes(category);
      input.addEventListener("change", () => mutate(() => {
        const mask = entry.collisionMask || [];
        entry.collisionMask = input.checked ? [...mask, category] : mask.filter(v => v !== category);
      }, false)); lab.append(input, document.createTextNode(String(category))); grid.append(lab);
    });
    body.append(grid, node("p", "field-note wide", "衝突対象の反映方法はゲーム側の生成処理に依存します。実際の衝突はゲームで確認してください。"));
    body = section("当たり判定の形状", "04", $("editor-form"), () => mutate(() => entry.colliders.push(C.collider())));
    if (!entry.colliders.length) body.append(node("p", "empty-inline", "Colliderはありません。判定を付ける場合は追加してください。"));
    entry.colliders.forEach((c, i) => {
      const p = `colliders[${i}]`, b = subCard(body, `COLLIDER ${String(i + 1).padStart(2, "0")}`, () => mutate(() => entry.colliders.splice(i, 1)));
      field(b, c, "type", "形状", { path: `${p}.type`, choices: C.colliderTypes, redraw: true });
      field(b, c, "surfaceType", "表面の材質", { path: `${p}.surfaceType`, choices: C.surfaces });
      vector(b, c, "center", "中心位置", `${p}.center`);
      if (c.type === "BOX") vector(b, c, "size", "箱の半分のサイズ", `${p}.size`, "現行エンジンはsizeを各軸の半幅として扱います。全幅2の箱は1を入力します。", 0);
      if (["SPHERE", "CAPSULE", "CYLINDER"].includes(c.type)) field(b, c, "radius", "半径", { number: true, min: 0, path: `${p}.radius` });
      if (["CAPSULE", "CYLINDER"].includes(c.type)) field(b, c, "height", c.type === "CYLINDER" ? "高さの半分" : "円柱部分の高さ", { number: true, min: 0, path: `${p}.height`, help: c.type === "CYLINDER" ? "現行エンジンはheightを半分の高さとして扱います。" : "上下の半球部分を除いた高さです。" });
      if (["CONVEX_HULL", "CONCAVE_TRIANGLE"].includes(c.type)) b.append(node("p", "field-note wide", "形状はオブジェクトのモデルから生成します。非表示のsize・radius・heightも保存時に保持します。"));
    });
  }
  function renderAssets(entry) {
    let body = section("基本情報", "01"); idField(body, entry);
    field(body, entry, "mapObjectType", "オブジェクトの種類", { choices: [["PROP", "小物・木など (PROP)"], ["BUILDING", "建物 (BUILDING)"], ["EXPLOSION_BUILDING", "爆発する建物 (EXPLOSION_BUILDING)"]] });
    referenceField(body, entry, "physicsPresetID", "物理プリセット", "physics");
    field(body, entry, "modelPath", "モデルのパス", { wide: true, placeholder: "Resource/Model/…/model.fbx", help: "ゲームの実行フォルダーを基準とした相対パス。ファイルの存在は検証しません。" });
    body = section("ゲーム内の性質", "02");
    field(body, entry, "health", "体力", { number: true, min: 0, wide: true });
    check(body, entry, "isDestruction", "破壊できる"); check(body, entry, "isGrabbable", "掴むことができる");
    body.append(node("p", "field-note wide", "掴む・爆発などの挙動は、ゲーム側の対応状況に応じて反映されます。"));
    body = section("マテリアル", "03", $("editor-form"), () => mutate(() => {
      let slot = 0; while (entry.materials.some(m => m.slot === slot)) slot++;
      entry.materials.push({ slot, tag: "" });
    }));
    const tags = [...new Set(C.rows("assets", state.docs.assets.data).flatMap(e => e.materials.map(m => m.tag)).filter(v => typeof v === "string"))];
    if (!entry.materials.length) body.append(node("p", "empty-inline", "マテリアルの指定はありません。"));
    entry.materials.forEach((m, i) => {
      const b = subCard(body, `MATERIAL ${String(i + 1).padStart(2, "0")}`, () => mutate(() => entry.materials.splice(i, 1)));
      field(b, m, "slot", "スロット番号", { number: true, min: 0, step: 1, path: `materials[${i}].slot` });
      field(b, m, "tag", "マテリアルタグ", { suggestions: tags, path: `materials[${i}].tag`, help: "候補は読み込んだ定義から取得。ゲーム側の登録名と一致させてください。" });
    });
  }
  function renderFragments(entry) {
    let body = section("対象の建物", "01"); idField(body, entry);
    body = section("破片モデル", "02", $("editor-form"), () => mutate(() => entry.fragments.push(C.fragment())));
    body.append(node("p", "field-note wide", "回転はJSONと同じラジアンで入力します。位置・回転・拡縮がゲームで使われるかは、破片生成処理の実装に依存します。"));
    if (!entry.fragments.length) body.append(node("p", "empty-inline", "破片はありません。追加ボタンで作成できます。"));
    entry.fragments.forEach((f, i) => {
      const p = `fragments[${i}]`, b = subCard(body, `FRAGMENT ${String(i + 1).padStart(2, "0")}`, () => mutate(() => entry.fragments.splice(i, 1)), () => mutate(() => entry.fragments.splice(i + 1, 0, C.clone(f))));
      field(b, f, "modelPath", "破片モデルのパス", { wide: true, path: `${p}.modelPath`, placeholder: "Resource/Model/…/fragment.fbx" });
      referenceField(b, f, "physicsPresetID", "物理プリセット", "physics", { wide: true, path: `${p}.physicsPresetID` });
      vector(b, f, "position", "ローカル位置", `${p}.position`);
      vector(b, f, "rotation", "回転（ラジアン）", `${p}.rotation`, "約1.5708 rad = 90°、約3.1416 rad = 180°。値の自動変換はしません。");
      vector(b, f, "scale", "拡縮", `${p}.scale`, "1が元の大きさです。", 0);
    });
  }
  function render() {
    const e = currentEntry();
    $("welcome").hidden = !!currentDoc(); $("editor-content").hidden = !currentDoc();
    $("editor-form").replaceChildren();
    $("duplicate-entry").disabled = !e; $("delete-entry").disabled = !e;
    $("entry-type").textContent = D[state.kind].english;
    $("entry-title").textContent = e ? idLabel(state.kind, e) : "新しい定義を追加";
    if (e) ({ physics: renderPhysics, assets: renderAssets, fragments: renderFragments })[state.kind](e);
    else if (currentDoc()) $("editor-form").append(node("p", "empty-inline", "左の「新規作成」から定義を追加できます。"));
    refresh();
  }

  // A batch is parsed completely before any live document is replaced.
  async function importRecords(records) {
    const parsed = records.map(record => ({ ...record, ...C.parse(record.text) }));
    if (new Set(parsed.map(p => p.kind)).size !== parsed.length) throw new Error("同じ種類のJSONが複数選ばれています。各種類1ファイルずつ選択してください。");
    if (parsed.some(p => state.docs[p.kind] && dirty(state.docs[p.kind])) && !window.confirm("読み込み直すファイルに未保存の変更があります。変更を破棄して読み込みますか？")) return;
    parsed.forEach(p => {
      state.docs[p.kind] = { data: p.data, name: p.name, handle: p.handle || null, diskText: p.text, baseline: json(p.data), exported: null };
      state.selected[p.kind] = 0;
    });
    state.past = []; state.future = []; $("search").value = "";
    if (!currentDoc()) state.kind = parsed[0].kind;
    render(); notify(`${parsed.length} ファイルを読み込みました。編集内容は保存するまで元ファイルに反映されません。`);
  }
  async function readFiles(files) { await importRecords(await Promise.all(Array.from(files).map(async file => ({ name: file.name, text: await file.text() })))); }
  function openFiles() {
    if (!window.showOpenFilePicker) { $("file-input").click(); return; }
    operation(async () => {
      let handles;
      try { handles = await window.showOpenFilePicker({ multiple: true, types: [{ description: "Map JSON", accept: { "application/json": [".json"] } }] }); }
      catch (error) { if (error.name === "SecurityError") { $("file-input").click(); return; } throw error; }
      const records = await Promise.all(handles.map(async handle => { const f = await handle.getFile(); return { name: f.name, text: await f.text(), handle }; }));
      await importRecords(records);
    });
  }
  $("open-files").addEventListener("click", openFiles); $("welcome-open").addEventListener("click", openFiles);
  $("file-input").addEventListener("change", e => { const files = Array.from(e.target.files); e.target.value = ""; if (files.length) operation(() => readFiles(files)); });
  $("open-folder").disabled = !window.showDirectoryPicker;
  $("open-folder").title = window.showDirectoryPicker ? "MapDataフォルダーを選択して3ファイルを開きます" : "このブラウザではJSONを開くを使ってください";
  $("open-folder").addEventListener("click", () => operation(async () => {
    const dir = await window.showDirectoryPicker({ mode: "read" });
    const records = [];
    for (const def of Object.values(D)) {
      let handle;
      try { handle = await dir.getFileHandle(def.file); }
      catch { throw new Error(`${def.file} が見つかりません。3ファイルがあるMapDataフォルダーを選択してください。`); }
      const file = await handle.getFile(); records.push({ name: file.name, text: await file.text(), handle });
    }
    await importRecords(records);
  }));
  $("load-nearby").addEventListener("click", () => {
    if (location.protocol === "file:") { notify("HTMLを直接開いた場合は「JSONを開く」または「フォルダーを開く」でファイルを選択してください。"); return; }
    operation(async () => {
      const records = await Promise.all(Object.values(D).map(async def => {
        const response = await fetch(def.file, { cache: "no-store" });
        if (!response.ok) throw new Error(`${def.file} を読み込めませんでした (${response.status})。`);
        return { name: def.file, text: await response.text() };
      }));
      await importRecords(records);
    });
  });
  $("search").addEventListener("input", renderList);
  $("undo").addEventListener("click", undo); $("redo").addEventListener("click", redo);
  $("editor-form").addEventListener("submit", e => e.preventDefault());
  $("add-entry").addEventListener("click", () => mutate(() => {
    const kind = state.kind, data = currentDoc().data;
    const id = kind === "fragments" ? referenceChoices("assets").find(v => !entries().some(e => e.assetID === v)) || "" : C.uniqueID(kind, data, kind === "physics" ? "New_Physics" : "New_Asset");
    entries().push(C.newEntry(kind, id)); state.selected[kind] = entries().length - 1; $("search").value = "";
  }));
  $("duplicate-entry").addEventListener("click", () => mutate(() => {
    const e = C.clone(currentEntry()), kind = state.kind;
    e[D[kind].id] = kind === "fragments" ? referenceChoices("assets").find(v => !entries().some(row => row.assetID === v)) || "" : C.uniqueID(kind, currentDoc().data, `${e[D[kind].id]}_copy`);
    entries().push(e); state.selected[kind] = entries().length - 1; $("search").value = "";
  }));
  $("delete-entry").addEventListener("click", () => {
    const e = currentEntry(), id = e[D[state.kind].id], refs = C.references(documents(), state.kind, id);
    if (refs.length) { notify(`「${id}」は読み込み済みデータの ${refs.length} 箇所から参照されています。先に参照先を変更してください。`, true); return; }
    if (!window.confirm(`「${id || "ID未入力"}」を削除しますか？${state.kind === "assets" ? " 配置JSONの参照はこのツールでは確認できません。" : ""}`)) return;
    mutate(() => { entries().splice(state.selected[state.kind], 1); state.selected[state.kind] = Math.max(0, state.selected[state.kind] - 1); });
  });
  function checkedText() {
    if (!$("editor-form").reportValidity()) throw new Error("入力欄の値を確認してください。");
    if (C.validate(documents()).some(i => i.level === "error")) throw new Error("保存前に、読み込み済みデータのエラーを修正してください。");
    return JSON.stringify(currentDoc().data, null, 4) + "\n";
  }
  $("download").addEventListener("click", () => {
    try {
      const text = checkedText(), doc = currentDoc();
      const url = URL.createObjectURL(new Blob([text], { type: "application/json;charset=utf-8" }));
      const a = node("a"); a.href = url; a.download = doc.name; document.body.append(a); a.click(); a.remove();
      setTimeout(() => URL.revokeObjectURL(url), 30000);
      doc.exported = json(doc.data); refresh();
      notify(`${doc.name} を書き出しました。元ファイルは更新されていません。ダウンロードしたJSONを必要な場所へ置き換えてください。`);
    } catch (error) { notify(error.message, true); }
  });
  $("save").addEventListener("click", () => operation(async () => {
    const text = checkedText(), doc = currentDoc();
    let handle = doc.handle;
    if (!handle) {
      if (!window.showSaveFilePicker) throw new Error("このブラウザでは「JSONを書き出す」を使ってください。");
      handle = await window.showSaveFilePicker({ suggestedName: doc.name, types: [{ description: "Map JSON", accept: { "application/json": [".json"] } }] });
      const dest = await handle.getFile();
      const existing = await dest.text();
      if (existing && existing !== doc.diskText && !window.confirm("保存先には読み込み時と異なる内容があります。編集した内容で上書きしますか？")) return;
    } else {
      // Do not overwrite edits made by the game, a text editor, or another browser tab.
      const latest = await handle.getFile();
      if (await latest.text() !== doc.diskText) throw new Error("元ファイルが別の場所で更新されています。書き出しで編集内容を退避し、元ファイルを読み込み直してください。");
    }
    const stream = await handle.createWritable();
    try { await stream.write(text); await stream.close(); }
    catch (error) { try { await stream.abort(); } catch { /* The stream may already be closed. */ } throw error; }
    doc.handle = handle; doc.name = handle.name; doc.diskText = text; doc.baseline = json(doc.data); doc.exported = null;
    notify(`${doc.name} を保存しました。ほかのタブに変更マークがある場合は、そのファイルも保存してください。`);
  }));
  window.addEventListener("beforeunload", e => { if (Object.values(state.docs).some(dirty)) { e.preventDefault(); e.returnValue = ""; } });
  document.addEventListener("keydown", e => {
    const editing = /^(INPUT|TEXTAREA|SELECT)$/.test(document.activeElement.tagName);
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "z" && !editing) { e.preventDefault(); e.shiftKey ? redo() : undo(); }
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "s") { e.preventDefault(); document.activeElement.blur(); if (!$("save").disabled) $("save").click(); }
  });
  let dragDepth = 0;
  document.addEventListener("dragenter", e => { if (Array.from(e.dataTransfer.types).includes("Files")) { e.preventDefault(); dragDepth++; document.body.classList.add("dragging"); } });
  document.addEventListener("dragover", e => { if (Array.from(e.dataTransfer.types).includes("Files")) e.preventDefault(); });
  document.addEventListener("dragleave", () => { if (--dragDepth <= 0) { dragDepth = 0; document.body.classList.remove("dragging"); } });
  document.addEventListener("drop", e => { e.preventDefault(); dragDepth = 0; document.body.classList.remove("dragging"); if (e.dataTransfer.files.length && !state.busy) operation(() => readFiles(e.dataTransfer.files)); });
  render();
})();
