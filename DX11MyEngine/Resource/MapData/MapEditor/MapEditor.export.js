/* Explicit save destination when supported; named download fallback otherwise. */
window.createMapEditorExport = api => {
  "use strict";
  const $ = id => document.getElementById(id), dialog = $("save-dialog");
  const text = () => JSON.stringify(api.getData(), null, 4) + "\n";
  function fallback(reason = "") {
    $("export-name").value = api.fileName(api.suggestedName());
    $("save-method-note").textContent = reason || "この環境では保存先を選択できません。保存フォルダーはブラウザーのダウンロード設定に従います。";
    dialog.showModal(); $("export-name").select();
  }
  async function save() {
    if (!api.canSave() || !api.validate()) return;
    if (typeof window.showSaveFilePicker !== "function" || !window.isSecureContext) { fallback(); return; }
    let writable = null, picked = false;
    api.setBusy(true);
    try {
      // Invoke from the click before serialization or any asynchronous work.
      const handle = await window.showSaveFilePicker({ id: "map-editor-json", suggestedName: api.fileName(api.suggestedName()), types: [{ description: "マップ配置JSON", accept: { "application/json": [".json"] } }], excludeAcceptAllOption: true });
      picked = true;
      writable = await handle.createWritable();
      await writable.write(text()); await writable.close(); writable = null;
      api.saved(handle.name); api.notice(`${handle.name} を保存しました。`);
    } catch (error) {
      if (writable) { try { await writable.abort(); } catch { /* Keep the original write error. */ } }
      if (error.name === "AbortError") api.notice("保存をキャンセルしました。");
      else if (!picked) fallback("保存先選択を利用できません。ファイル名を指定してダウンロードできます。保存フォルダーはブラウザーの設定に従います。");
      else api.notice(`保存に失敗しました：${error.message || String(error)}`, "error");
    } finally { api.setBusy(false); }
  }
  $("save-form").addEventListener("submit", event => {
    event.preventDefault();
    if (!api.canSave() || !api.validate()) return;
    if (!$("export-name").value.trim()) { $("export-name").setCustomValidity("ファイル名を入力してください。"); $("export-name").reportValidity(); return; }
    let url;
    api.setBusy(true);
    try {
      const name = api.fileName($("export-name").value);
      url = URL.createObjectURL(new Blob([text()], { type: "application/json" }));
      const link = document.createElement("a"); link.href = url; link.download = name;
      document.body.append(link); link.click(); link.remove();
      dialog.close(); api.saved(name); api.notice(`${name} のダウンロードを開始しました。`);
    } catch (error) { api.notice(`書き出しに失敗しました：${error.message || String(error)}`, "error"); }
    finally { if (url) setTimeout(() => URL.revokeObjectURL(url), 1000); api.setBusy(false); }
  });
  $("export-name").addEventListener("input", () => $("export-name").setCustomValidity(""));
  $("save-cancel").addEventListener("click", () => { dialog.close(); api.notice("保存をキャンセルしました。"); });
  dialog.addEventListener("cancel", () => api.notice("保存をキャンセルしました。"));
  return { save };
};
