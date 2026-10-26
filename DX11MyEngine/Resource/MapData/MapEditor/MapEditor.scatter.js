/* Random placement UI and provisional instances; game objects are committed once. */
window.createMapEditorScatter = api => {
  "use strict";
  const { T, placement, geometryTools } = api;
  const $ = id => document.getElementById(id);
  const panel = $("random-tools"), assetSelect = $("random-asset");
  const group = new T.Group(); api.scene.add(group);
  const center = new T.Group(); api.scene.add(center);
  let preview = [], guide = null, generating = false, generation = 0;
  let editingCenter = false, centerStart = null, scrub = null;
  const controls = [...panel.querySelectorAll("input, select")];
  const centerInputs = [$("random-x"), $("random-y"), $("random-z")];
  function readCenter() {
    const values = centerInputs.map(input => input.valueAsNumber);
    if (!values.every(Number.isFinite)) return false;
    center.position.fromArray(values); center.updateMatrixWorld(true); return true;
  }
  function settingsChanged() {
    invalidate("設定が変わりました。プレビューを更新してください。");
    readCenter(); updateUI(); updateGuide();
  }
  function stopCenter(restoreSelection = true) {
    if (!editingCenter) return;
    editingCenter = false; centerStart = null;
    api.centerTarget(null, restoreSelection); updateUI();
  }
  function beginCenterDrag() { centerStart = center.position.clone(); }
  function moveCenter() {
    if (!editingCenter) return;
    api.snapCenter(center.position, centerStart);
    centerInputs.forEach((input, index) => { input.value = String(center.position.getComponent(index)); });
    invalidate("中心が変わりました。プレビューを更新してください。");
    if (guide) guide.position.copy(center.position);
    api.render();
  }
  function endCenterDrag(cancel = false) {
    if (cancel && centerStart) { center.position.copy(centerStart); moveCenter(); }
    centerStart = null;
  }
  function focusCenter() {
    if (generating || scrub || !api.canStart()) return;
    updateGuide();
    if (guide) api.focus([{ root: guide }]);
  }
  function config() {
    const number = id => $(id).valueAsNumber;
    const shape = $("random-shape").value, randomYaw = $("random-yaw").checked, randomScale = $("random-scale").checked;
    return placement.validateScatter({
      count: number("random-count"), shape, centerX: number("random-x"), centerY: number("random-y"), centerZ: number("random-z"),
      radius: shape === "circle" ? number("random-radius") : 1,
      width: shape === "rectangle" ? number("random-width") : 1, depth: shape === "rectangle" ? number("random-depth") : 1,
      minDistance: number("random-distance"), ground: $("random-ground").checked,
      randomYaw, yawMin: randomYaw ? number("random-yaw-min") : 0, yawMax: randomYaw ? number("random-yaw-max") : 0,
      randomScale, scaleMin: randomScale ? number("random-scale-min") : 1, scaleMax: randomScale ? number("random-scale-max") : 1,
      seed: $("random-seed").value,
    });
  }
  function clearGuide() {
    if (!guide) return;
    api.scene.remove(guide); guide.geometry.dispose(); guide.material.dispose(); guide = null;
  }
  function updateGuide() {
    clearGuide();
    if (!panel.open) { api.render(); return; }
    try {
      const settings = config(), points = [];
      if (settings.shape === "circle") {
        for (let i = 0; i < 64; i++) for (const angle of [i / 64 * Math.PI * 2, (i + 1) / 64 * Math.PI * 2]) points.push(Math.cos(angle) * settings.radius, 0, Math.sin(angle) * settings.radius);
      } else {
        const x = settings.width / 2, z = settings.depth / 2;
        points.push(-x, 0, -z, x, 0, -z, x, 0, -z, x, 0, z, x, 0, z, -x, 0, z, -x, 0, z, -x, 0, -z);
      }
      // GridHelper is already bundled and provides a LineSegments geometry.
      guide = new T.GridHelper(1, 1);
      const Attribute = guide.geometry.getAttribute("position").constructor;
      const colors = new Float32Array(points.length); for (let i = 0; i < colors.length; i += 3) { colors[i] = 0.1; colors[i + 1] = 0.7; colors[i + 2] = 0.85; }
      guide.geometry.setAttribute("position", new Attribute(new Float32Array(points), 3));
      guide.geometry.setAttribute("color", new Attribute(colors, 3)); guide.geometry.computeBoundingSphere();
      guide.material.depthTest = false; guide.renderOrder = 10;
      guide.position.set(settings.centerX, settings.centerY, settings.centerZ); api.scene.add(guide);
    } catch { /* Incomplete numeric input temporarily hides the guide. */ }
    api.render();
  }
  function clearPreview() {
    for (const model of preview) { group.remove(model.root); api.discard(model); }
    preview = []; api.render();
  }
  function updateUI() {
    const disabled = generating || !api.canStart();
    const interacting = !!scrub || !!centerStart;
    for (const input of controls) input.disabled = disabled;
    $("random-yaw-min").disabled = disabled || !$("random-yaw").checked;
    $("random-yaw-max").disabled = disabled || !$("random-yaw").checked;
    $("random-scale-min").disabled = disabled || !$("random-scale").checked;
    $("random-scale-max").disabled = disabled || !$("random-scale").checked;
    $("random-radius-label").hidden = $("random-shape").value !== "circle";
    $("random-width-label").hidden = $("random-shape").value !== "rectangle";
    $("random-depth-label").hidden = $("random-shape").value !== "rectangle";
    $("random-preview").disabled = disabled || interacting || !assetSelect.value;
    $("random-confirm").disabled = disabled || interacting || !preview.length;
    $("random-frame").disabled = disabled || interacting || !preview.length;
    $("random-move").disabled = disabled || interacting;
    $("random-move").setAttribute("aria-pressed", String(editingCenter));
    $("random-center-frame").disabled = disabled || interacting;
  }
  function refreshAssets() {
    const previous = assetSelect.value;
    assetSelect.replaceChildren(...api.assets().map(asset => { const option = document.createElement("option"); option.value = asset.assetID; option.textContent = asset.assetID; return option; }));
    if ([...assetSelect.options].some(option => option.value === previous)) assetSelect.value = previous;
    updateUI();
  }
  function invalidate(message = "配置が変わりました。プレビューを更新してください。") {
    if (!preview.length || generating) return;
    clearPreview(); $("random-status").textContent = message; updateUI();
  }
  function cancel() {
    finishScrub(true); endCenterDrag(true); stopCenter();
    generation++;
    clearPreview(); clearGuide();
    if (generating) { generating = false; api.setGenerating(false); }
    $("random-status").textContent = "キャンセルしました。"; updateUI();
  }
  for (const input of controls) input.addEventListener("input", settingsChanged);
  // Keep focused inputs native for typing. Labels always allow scrubbing.
  for (const input of controls.filter(input => input.type === "number")) {
    const label = document.createElement("span"); label.className = "scrub-label";
    label.textContent = input.parentElement.firstChild.textContent;
    input.parentElement.firstChild.replaceWith(label);
    input.title = label.title = "左右ドラッグ：値を変更 ／ クリック：直接入力 ／ Shift：微調整 ／ Esc：取消";
    const sensitivity = input.id === "random-count" ? 0.2 : input.id.includes("yaw-") ? 0.5 : input.id.includes("scale-") ? 0.01 : 0.1;
    for (const handle of [input, label]) handle.addEventListener("pointerdown", event => {
      if (event.button !== 0 || input.disabled || scrub || generating || !api.canStart()) return;
      if (handle === input && document.activeElement === input) return;
      if (!Number.isFinite(input.valueAsNumber)) return;
      event.preventDefault(); document.activeElement?.blur();
      scrub = { input, handle, id: event.pointerId, startX: event.clientX, lastX: event.clientX, original: input.value, value: input.valueAsNumber, sensitivity, dragging: false };
      handle.setPointerCapture(event.pointerId); api.interactionChanged();
    });
  }
  function finishScrub(cancel = false) {
    if (!scrub) return;
    const state = scrub; scrub = null; document.body.classList.remove("scrubbing");
    if (cancel && state.dragging) { state.input.value = state.original; settingsChanged(); }
    if (state.handle.hasPointerCapture(state.id)) state.handle.releasePointerCapture(state.id);
    if (!cancel && !state.dragging) { state.input.focus(); state.input.select(); }
    api.interactionChanged(); updateUI();
  }
  document.addEventListener("pointermove", event => {
    if (!scrub || event.pointerId !== scrub.id) return;
    if (!scrub.dragging && Math.abs(event.clientX - scrub.startX) < 4) return;
    event.preventDefault(); scrub.dragging = true; document.body.classList.add("scrubbing");
    const { input } = scrub;
    const value = scrub.value + (event.clientX - scrub.lastX) * scrub.sensitivity * (event.shiftKey ? 0.1 : 1);
    scrub.lastX = event.clientX;
    if (!Number.isFinite(value)) return;
    scrub.value = Math.min(input.max === "" ? Infinity : Number(input.max), Math.max(input.min === "" ? -Infinity : Number(input.min), value));
    input.value = String(input.step === "1" ? Math.round(scrub.value) : Number(scrub.value.toPrecision(12)));
    settingsChanged();
  }, { passive: false });
  document.addEventListener("pointerup", event => { if (scrub?.id === event.pointerId) finishScrub(); });
  for (const type of ["pointercancel", "lostpointercapture"]) document.addEventListener(type, event => { if (scrub?.id === event.pointerId) finishScrub(true); });
  window.addEventListener("blur", () => finishScrub(true));
  $("random-move").addEventListener("click", () => {
    if (generating || scrub || !api.canStart()) return;
    if (editingCenter) stopCenter();
    else if (readCenter()) { editingCenter = true; updateGuide(); api.centerTarget(center); updateUI(); }
    else api.notice("中心には有限の数値を入力してください。", "error");
  });
  $("random-center-frame").addEventListener("click", focusCenter);
  panel.addEventListener("toggle", () => { if (panel.open) { refreshAssets(); updateGuide(); } else cancel(); });
  $("random-cancel").addEventListener("click", cancel);
  $("random-frame").addEventListener("click", () => { if (!generating && api.canStart()) api.focus(preview); });
  $("random-preview").addEventListener("click", async () => {
    if (generating || !api.canStart()) return;
    let settings, asset;
    try {
      settings = config(); asset = api.assets().find(item => item.assetID === assetSelect.value);
      if (!asset) throw new Error("読み込み済みのアセットを選択してください。");
      if (settings.ground && !api.grounds().length) throw new Error("地面吸着には参照地形または明示指定した地形が必要です。");
    } catch (error) { api.notice(error.message, "error"); return; }
    clearPreview(); updateGuide();
    const run = ++generation;
    const sampler = placement.createScatter(settings, api.existing().map(model => model.root.position.toArray()));
    const grounds = api.grounds();
    let probe = null, groundMisses = 0, sliceStart = performance.now();
    generating = true; api.setGenerating(true); updateUI();
    $("random-status").textContent = `生成中：0 / ${settings.count} 件`;
    try {
      // Paint the cancel button before work starts; yield again every ~8 ms.
      await new Promise(resolve => setTimeout(resolve, 0));
      while (run === generation) {
        const step = sampler.next(); if (step.done) break;
        if (step.candidate) {
          const candidate = step.candidate;
          if (!probe) probe = api.create(asset.source, asset.assetID, candidate);
          else api.restore(probe.root, candidate);
          const y = settings.ground ? geometryTools.groundY(probe.root, grounds) : probe.root.position.y;
          if (y === null) groundMisses++;
          else {
            probe.root.position.y = y; probe.root.updateMatrixWorld(true);
            sampler.accept(candidate); preview.push(probe); group.add(probe.root); probe = null;
          }
        }
        if (performance.now() - sliceStart > 8) {
          $("random-status").textContent = `生成中：${preview.length} / ${settings.count} 件（試行 ${sampler.attempts}）`;
          api.render(); await new Promise(resolve => setTimeout(resolve, 0)); sliceStart = performance.now();
        }
      }
      if (run === generation) {
        $("random-status").textContent = `未確定 ${preview.length} / ${settings.count} 件 ／ 試行 ${sampler.attempts}${groundMisses ? ` ／ 地面なし ${groundMisses}` : ""}${preview.length < settings.count ? "（試行上限）" : ""}`;
        api.render();
      }
    } catch (error) {
      if (run === generation) { clearPreview(); api.notice(`生成を中止しました：${error.message}`, "error"); $("random-status").textContent = "生成失敗"; }
    } finally {
      if (probe) api.discard(probe);
      if (run === generation) { generating = false; api.setGenerating(false); updateUI(); }
    }
  });
  $("random-confirm").addEventListener("click", () => {
    if (generating || !preview.length || !api.canStart()) return;
    const committed = preview; preview = [];
    for (const model of committed) group.remove(model.root);
    api.commit(committed);
    $("random-status").textContent = `${committed.length} 件を確定しました。`;
    updateUI(); api.render();
  });
  refreshAssets();
  return { refreshAssets, invalidate, cancel, updateUI, stopCenter, beginCenterDrag, moveCenter, endCenterDrag, focusCenter,
    cancelInteraction() { if (scrub) { finishScrub(true); return true; } if (centerStart) { endCenterDrag(true); return true; } return false; },
    get editingCenter() { return editingCenter; }, get interacting() { return !!scrub; },
    get generating() { return generating; }, get preview() { return preview; }, get guide() { return guide; } };
};
