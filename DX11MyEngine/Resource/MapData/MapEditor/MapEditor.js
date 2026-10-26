/* Local-only placement prototype. Original FBX files are never modified. */
(() => {
  "use strict";
  const $ = id => document.getElementById(id);
  const notice = (message, kind = "") => { $("notice").textContent = message; $("notice").className = kind; };
  if (!window.MapEditorThree) {
    notice("3Dライブラリを読み込めませんでした。vendor フォルダーをHTMLと一緒に配置してください。", "error");
    return;
  }
  const T = window.MapEditorThree;
  const C = window.MapEditorCore;
  const placementTools = window.MapEditorPlacement;
  const geometryTools = window.createMapEditorSceneTools(T);
  const appearance = window.createMapEditorAppearance();
  let renderer;
  try { renderer = new T.WebGLRenderer({ antialias: true }); }
  catch { notice("3D表示を開始できませんでした。WebGL 2に対応したブラウザーとGPU設定を確認してください。", "error"); return; }
  const viewport = $("viewport");
  renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
  viewport.prepend(renderer.domElement);
  const scene = new T.Scene();
  scene.background = new T.Color($("background-color").value);
  const perspectiveCamera = new T.PerspectiveCamera(50, 1, 0.01, 100000);
  const orthographicCamera = new T.OrthographicCamera(-100, 100, 100, -100, 0.01, 100000);
  let camera = perspectiveCamera, orthoHeight = 200, viewMode = "perspective";
  let perspectiveDirection = new T.Vector3(100, 80, 120).normalize();
  const viewDirections = { front: new T.Vector3(0, 0, 1), side: new T.Vector3(1, 0, 0), top: new T.Vector3(0, 1, 0) };
  camera.position.set(100, 80, 120);
  scene.add(new T.HemisphereLight(0xffffff, 0x777f89, 2));
  const sunlight = new T.DirectionalLight(0xffffff, 2.5);
  sunlight.position.set(100, 150, 80);
  scene.add(sunlight);
  let controls = new T.OrbitControls(camera, renderer.domElement);
  const transform = new T.TransformControls(camera, renderer.domElement);
  transform.setMode("translate"); transform.setSpace("world"); transform.setSize(0.9);
  scene.add(transform.getHelper());
  const pivot = new T.Group(); pivot.rotation.order = C.rotationOrder; scene.add(pivot);
  const outlines = new Map();
  let grid = null, selected = null, busy = false, sequence = 0, framePending = false;
  const models = [];
  const byId = new Map(), instances = new Map(), selection = new Set(), sources = new Map(), assetDefinitions = new Map(), sourceByAsset = new Map();
  let sourceSequence = 0, transaction = null;
  let scatter = null;
  let selectionBox = null;
  let centerPreviousMode = null;
  let spaceMode = "world", focusAnimation = null;
  const terrainIds = new Set(), terrainReferences = new Map();
  const hiddenIds = new Set(), lockedIds = new Set();
  let mapDocument = { mapName: "NewMap", groundAssetTag: "", objects: [] };
  let layoutFileName = "";
  const snapSettings = { position: false, positionStep: 1, rotation: false, rotationStep: 15 };
  const raycaster = new T.Raycaster();
  const pointer = new T.Vector2();
  const axes = ["x", "y", "z"];
  const minScale = 0.001;
  const fields = [...document.querySelectorAll(".transform-fields input")].map(input => {
    const [property, axis] = input.id.split("-");
    return { input, property, axis, sensitivity: property === "rotation" ? 0.5 : property === "scale" ? 0.01 : 0.1 };
  });
  let scrub = null, transformValidationMessage = null;
  const history = new C.History(100, () => { releaseUnusedInstances(); scatter?.invalidate(); updateButtons(); });

  function requestRender() {
    if (framePending) return;
    framePending = true;
    requestAnimationFrame(time => {
      framePending = false;
      advanceFocus(time);
      for (const [id, outline] of outlines) if (byId.has(id)) outline.box.setFromObject(byId.get(id).root);
      appearance.render(renderer, scene, camera, [...models.map(model => model.root), ...[...terrainReferences.values()].map(source => source.root), ...(scatter?.preview || []).map(model => model.root)]);
      if (focusAnimation) requestRender();
    });
  }
  function setGrid(step) {
    if (grid) { scene.remove(grid); grid.geometry.dispose(); grid.material.dispose(); }
    grid = new T.GridHelper(step * 20, 20, 0x43484d, 0x64696e);
    scene.add(grid);
    $("grid-status").textContent = `グリッド：${Number(step.toPrecision(4))} 単位`;
  }
  setGrid(10);
  function configureControls() {
    controls.enableDamping = false;
    controls.mouseButtons = { LEFT: null, MIDDLE: T.MOUSE.PAN, RIGHT: T.MOUSE.ROTATE };
    controls.addEventListener("start", cancelFocus);
    controls.addEventListener("change", () => { updateViewState(); requestRender(); });
  }
  configureControls();
  renderer.domElement.addEventListener("pointerdown", cancelFocus, { capture: true });
  renderer.domElement.addEventListener("wheel", cancelFocus, { capture: true, passive: true });
  transform.addEventListener("change", requestRender);
  transform.addEventListener("dragging-changed", e => { controls.enabled = !e.value && !busy; updateButtons(); });
  transform.addEventListener("mouseDown", () => scatter?.editingCenter ? scatter.beginCenterDrag() : beginEdit("Transform", "gizmo"));
  transform.addEventListener("mouseUp", () => scatter?.editingCenter ? scatter.endCenterDrag() : endEdit());
  transform.addEventListener("objectChange", () => {
    if (scatter?.editingCenter) { scatter.moveCenter(); requestRender(); return; }
    if (transaction) applyPivot();
    syncTransform(); requestRender();
  });
  renderer.domElement.addEventListener("contextmenu", e => e.preventDefault());
  renderer.domElement.addEventListener("webglcontextlost", e => {
    e.preventDefault(); cancelEdit(); notice("GPUとの接続が失われました。JSONを保存してからページを再読み込みしてください。", "error");
  });
  renderer.domElement.addEventListener("webglcontextrestored", () => { notice("3D表示を再開しました。"); requestRender(); });
  new ResizeObserver(() => {
    cancelFocus();
    const width = viewport.clientWidth, height = viewport.clientHeight;
    if (!width || !height) return;
    renderer.setSize(width, height, false);
    perspectiveCamera.aspect = width / height; perspectiveCamera.updateProjectionMatrix();
    resizeOrthographic(); requestRender();
  }).observe(viewport);

  function resizeOrthographic() {
    const aspect = viewport.clientWidth / Math.max(viewport.clientHeight, 1);
    orthographicCamera.left = -orthoHeight * aspect / 2;
    orthographicCamera.right = orthoHeight * aspect / 2;
    orthographicCamera.top = orthoHeight / 2;
    orthographicCamera.bottom = -orthoHeight / 2;
    orthographicCamera.updateProjectionMatrix();
  }
  function updateViewState() {
    if (viewDirections[viewMode]) {
      const direction = camera.position.clone().sub(controls.target).normalize();
      if (direction.dot(viewDirections[viewMode]) < 1 - 1e-8) viewMode = "orthographic";
    }
    const names = { perspective: "透視図", front: "前・平行投影", side: "横・平行投影", top: "上・平行投影", orthographic: "平行投影（自由）" };
    $("view-name").textContent = names[viewMode];
    $("view-axes").textContent = viewMode === "top" ? "+Yから ／ −Zが上" : viewMode === "front" ? "+Zから ／ Yが上" : viewMode === "side" ? "+Xから ／ Yが上" : "Y 上方向";
    document.querySelectorAll("[data-view]").forEach(button => button.setAttribute("aria-pressed", String(button.dataset.view === viewMode)));
  }
  function setView(view) {
    if (busy || transform.dragging || scrub || scatter?.interacting || selectionBox?.interacting) return;
    cancelFocus();
    const target = controls.target.clone();
    const distance = camera.position.distanceTo(target);
    const visibleHeight = camera.isOrthographicCamera ? orthoHeight / camera.zoom : 2 * distance * Math.tan(camera.fov * Math.PI / 360) / camera.zoom;
    if (camera.isPerspectiveCamera) perspectiveDirection.copy(camera.position).sub(target).normalize();
    controls.dispose();
    camera = view === "perspective" ? perspectiveCamera : orthographicCamera;
    camera.zoom = 1;
    camera.up.set(0, view === "top" ? 0 : 1, view === "top" ? -1 : 0);
    let nextDistance;
    if (camera.isOrthographicCamera) {
      orthoHeight = Math.max(visibleHeight, 0.0001); resizeOrthographic();
      nextDistance = Math.max(orthoHeight * 2, 1);
    } else {
      nextDistance = Math.max(visibleHeight / (2 * Math.tan(camera.fov * Math.PI / 360)), 0.001);
    }
    camera.position.copy(target).addScaledVector(viewDirections[view] || perspectiveDirection, nextDistance);
    camera.near = Math.min(camera.near, nextDistance / 1000);
    camera.far = Math.max(camera.far, nextDistance * 100);
    camera.updateProjectionMatrix(); camera.lookAt(target);
    controls = new T.OrbitControls(camera, renderer.domElement);
    configureControls(); controls.target.copy(target);
    controls.minDistance = camera.near * 10; controls.maxDistance = camera.far / 2;
    transform.camera = camera; viewMode = view;
    controls.update(); updateViewState(); requestRender();
  }
  document.querySelectorAll("[data-view]").forEach(button => button.addEventListener("click", () => setView(button.dataset.view)));
  function setMode(mode) {
    if (busy || transform.dragging || scrub || scatter?.interacting || selectionBox?.interacting) return;
    scatter?.stopCenter();
    transform.setMode(mode); transform.setSpace(spaceMode);
    document.querySelectorAll("[data-mode]").forEach(button => button.setAttribute("aria-pressed", String(button.dataset.mode === mode)));
    requestRender();
  }
  document.querySelectorAll("[data-mode]").forEach(button => button.addEventListener("click", () => setMode(button.dataset.mode)));
  function setSpace(space) {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
    scatter?.stopCenter();
    spaceMode = space;
    const editable = editableModels();
    if (editable.length > 1) {
      // Reorient the editing proxy only. Never reparent or rotate the objects.
      if (space === "local") pivot.quaternion.copy(editable[editable.length - 1].root.quaternion);
      else pivot.rotation.set(0, 0, 0);
      pivot.scale.set(1, 1, 1);
    }
    transform.setSpace(space);
    document.querySelectorAll("[data-space]").forEach(button => button.setAttribute("aria-pressed", String(button.dataset.space === space)));
    syncTransform(); requestRender();
  }
  document.querySelectorAll("[data-space]").forEach(button => button.addEventListener("click", () => setSpace(button.dataset.space)));
  $("background-color").addEventListener("input", event => { scene.background.set(event.target.value); requestRender(); });
  function syncAppearanceUI() {
    const items = selectedModels();
    $("solid-color").disabled = appearance.mode === "material";
    $("selected-color").disabled = busy || !items.length;
    $("selected-color").value = items.length ? appearance.colorOf((selected || items[items.length - 1]).root) : $("solid-color").value;
    $("selected-color").title = new Set(items.map(model => appearance.colorOf(model.root))).size > 1 ? "色が異なる対象を選択中。変更すると全対象へ同じ色を適用します。" : "編集用の表示色。変更すると単色表示へ切り替えます。";
    $("color-by-asset").disabled = busy || !items.length;
    $("reset-selected-color").disabled = busy || !(colorTargets().some(model => appearance.hasColor(model.root)) || ($("color-by-asset").checked && items.some(model => appearance.hasAssetColor(model.assetID))));
    $("reset-selected-color").title = $("color-by-asset").checked ? "同じアセットIDの色設定を解除して基本色に戻す" : "個別の色設定を解除し、種類別の色（未設定なら基本色）に戻す";
  }
  function colorTargets() {
    const items = selectedModels();
    if (!$("color-by-asset").checked) return items;
    const ids = new Set(items.map(model => model.assetID));
    // Include instances retained by history, so Undo cannot resurrect an old
    // personal color after an asset-wide change. Provisional objects inherit too.
    return [...instances.values(), ...(scatter?.preview || [])].filter(model => ids.has(model.assetID));
  }
  $("color-by-asset").addEventListener("change", syncAppearanceUI);
  $("display-mode").addEventListener("change", event => { appearance.setMode(event.target.value); syncAppearanceUI(); requestRender(); });
  $("solid-color").addEventListener("input", event => { appearance.setBaseColor(event.target.value); syncAppearanceUI(); requestRender(); });
  $("selected-color").addEventListener("input", event => {
    if (busy) return;
    if ($("color-by-asset").checked) {
      for (const id of new Set(selectedModels().map(model => model.assetID))) appearance.setAssetColor(id, event.target.value);
      for (const model of colorTargets()) appearance.clearColor(model.root);
    } else for (const model of selectedModels()) appearance.setColor(model.root, event.target.value);
    if (appearance.mode === "material") { appearance.setMode("solid"); $("display-mode").value = "solid"; }
    syncAppearanceUI(); requestRender();
  });
  $("reset-selected-color").addEventListener("click", () => {
    if (busy) return;
    if ($("color-by-asset").checked) for (const id of new Set(selectedModels().map(model => model.assetID))) appearance.clearAssetColor(id);
    for (const model of colorTargets()) appearance.clearColor(model.root);
    syncAppearanceUI(); requestRender();
  });
  for (const [id, key] of [["map-name", "mapName"], ["ground-asset-tag", "groundAssetTag"]]) $(id).addEventListener("input", event => {
    if (!busy) mapDocument[key] = event.target.value;
  });
  function syncMapInfo() { $("map-name").value = mapDocument.mapName; $("ground-asset-tag").value = mapDocument.groundAssetTag; }

  function snapshotRoot(root) {
    return { position: root.position.toArray(), rotation: root.rotation.toArray(), scale: root.scale.toArray() };
  }
  function restoreRoot(root, state) {
    root.position.fromArray(state.position); root.rotation.fromArray(state.rotation); root.scale.fromArray(state.scale);
    root.updateMatrixWorld(true);
  }
  const selectedModels = () => [...selection].map(id => byId.get(id)).filter(Boolean);
  const canEdit = model => !!model && !hiddenIds.has(model.id) && !lockedIds.has(model.id);
  const editableModels = () => selectedModels().filter(canEdit);
  const selectionState = () => ({ ids: [...selection], editableIds: editableModels().map(model => model.id), pivot: snapshotRoot(pivot) });
  const snapshots = (items = editableModels()) => items.map(model => ({ id: model.id, assetID: model.assetID, ...snapshotRoot(model.root) }));
  function restoreStates(states, pivotState, ids = states.map(state => state.id)) {
    for (const state of states) {
      const model = byId.get(state.id);
      if (model) { model.assetID = state.assetID; appearance.setAsset(model.root, model.assetID); restoreRoot(model.root, state); }
    }
    setSelection({ ids, editableIds: states.map(state => state.id), pivot: pivotState });
  }
  function beginEdit(label, source = "field") {
    if (transaction || !editableModels().length) return;
    scatter?.stopCenter();
    cancelFocus();
    transaction = {
      label, source, space: spaceMode, before: snapshots(), selection: selectionState(), pivot: snapshotRoot(pivot),
      position: pivot.position.clone(), quaternion: pivot.quaternion.clone(), scale: pivot.scale.clone(),
      objects: editableModels().map(model => ({ model, position: model.root.position.clone(), quaternion: model.root.quaternion.clone(), scale: model.root.scale.clone() })),
      snap: { ...snapSettings },
    };
    // Three.js snaps the rotation delta, preserving an existing off-step angle.
    transform.setRotationSnap(transaction.snap.rotation ? transaction.snap.rotationStep * Math.PI / 180 : null);
    updateButtons();
  }
  function applyPivot() {
    if (!transaction) return;
    const start = transaction;
    if (start.source === "gizmo" && transform.mode === "translate" && start.snap.position) {
      if (start.space === "local") {
        const delta = pivot.position.clone().sub(start.position).applyQuaternion(start.quaternion.clone().invert());
        axes.forEach(axis => { delta[axis] = C.snap(delta[axis], start.snap.positionStep); });
        pivot.position.copy(start.position).add(delta.applyQuaternion(start.quaternion));
      } else axes.forEach(axis => { pivot.position[axis] = C.snapDrag(pivot.position[axis], start.position[axis], start.snap.positionStep); });
    }
    if (C.equal(snapshotRoot(pivot), start.pivot)) {
      start.before.forEach(state => restoreRoot(byId.get(state.id).root, state));
      return;
    }
    axes.forEach(axis => { if (pivot.scale[axis] !== start.scale[axis]) pivot.scale[axis] = Math.max(minScale, pivot.scale[axis]); });
    if (start.objects.length === 1) {
      const root = start.objects[0].model.root;
      root.position.copy(pivot.position); root.rotation.copy(pivot.rotation); root.scale.copy(pivot.scale); root.updateMatrixWorld(true);
      return;
    }
    const rotationChanged = !C.equal(pivot.rotation.toArray(), start.pivot.rotation);
    const scaleChanged = !C.equal(pivot.scale.toArray(), start.pivot.scale);
    const rotation = pivot.quaternion.clone().multiply(start.quaternion.clone().invert());
    const ratio = pivot.scale.clone().divide(start.scale);
    const inverse = start.quaternion.clone().invert();
    for (const [index, item] of start.objects.entries()) {
      const offset = item.position.clone().sub(start.position);
      if (scaleChanged) offset.applyQuaternion(inverse).multiply(ratio).applyQuaternion(start.quaternion);
      if (rotationChanged) offset.applyQuaternion(rotation);
      item.model.root.position.copy(pivot.position).add(offset);
      if (rotationChanged) item.model.root.quaternion.copy(rotation).multiply(item.quaternion);
      else item.model.root.rotation.fromArray(start.before[index].rotation);
      item.model.root.scale.copy(item.scale); if (scaleChanged) item.model.root.scale.multiply(ratio);
      item.model.root.updateMatrixWorld(true);
    }
  }
  function endEdit(cancel = false) {
    if (!transaction) return;
    const edit = transaction;
    const after = snapshots(edit.objects.map(item => item.model)), afterPivot = snapshotRoot(pivot);
    transaction = null;
    if (cancel) restoreStates(edit.before, edit.pivot, edit.selection.ids);
    else if (!C.equal(edit.before, after)) {
      const ids = edit.selection.ids;
      history.commit(new C.TransformCommand(edit.label, edit.before, after, edit.pivot, afterPivot, (states, statePivot) => restoreStates(states, statePivot, ids)), true);
    }
    syncTransform(); updateButtons(); requestRender();
  }
  function cancelEdit() {
    scatter?.cancelInteraction();
    if (transform.dragging) { transform.reset(); transform.dragging = false; transform.axis = null; }
    endEdit(true); controls.enabled = !busy;
  }
  function displayValue(field) {
    return pivot[field.property][field.axis] * (field.property === "rotation" ? 180 / Math.PI : 1);
  }
  function applyValue(field, value, dragging = false) {
    if (!transaction) return;
    const { property, axis } = field;
    const settings = transaction?.snap || snapSettings;
    const step = property === "position" && settings.position ? settings.positionStep : property === "rotation" && settings.rotation ? settings.rotationStep : null;
    if (step) {
      const start = transaction.pivot[property][axes.indexOf(axis)] * (property === "rotation" ? 180 / Math.PI : 1);
      value = dragging ? C.snapDrag(value, start, step) : C.snap(value, step);
    }
    pivot[property][axis] = value * (property === "rotation" ? Math.PI / 180 : 1);
    applyPivot();
    if (transformValidationMessage && $("notice").textContent === transformValidationMessage) notice("編集値を更新しました。");
    transformValidationMessage = null;
    syncTransform(); requestRender();
  }
  function syncTransform() {
    if (!selection.size) return;
    fields.forEach(field => { field.input.value = Number(displayValue(field).toFixed(6)); });
  }
  function refreshList() {
    const matches = C.filterObjects(models, $("object-search").value);
    const existing = new Map([...$("object-list").querySelectorAll("[data-object-id]")].map(button => [Number(button.dataset.objectId), button]));
    const rows = [];
    for (const model of matches) {
      let button = existing.get(model.id);
      if (!button) {
        button = document.createElement("button"); button.className = "object-item";
        const row = document.createElement("div"); row.className = "object-row"; row.append(button);
        const visible = document.createElement("button"); visible.className = "row-toggle"; visible.dataset.visibilityId = model.id;
        visible.addEventListener("click", event => { event.stopPropagation(); setHidden([model.id], !hiddenIds.has(model.id)); });
        row.append(visible);
        const lock = document.createElement("button"); lock.className = "row-toggle"; lock.dataset.lockId = model.id;
        lock.addEventListener("click", event => { event.stopPropagation(); setLocked([model.id], !lockedIds.has(model.id)); });
        row.append(lock);
        button.append(document.createElement("strong"), document.createElement("small"));
        button.addEventListener("click", event => select(model, event));
        button.addEventListener("dblclick", event => { event.preventDefault(); if (!selection.has(model.id)) select(model); frame([model], true); });
        button.addEventListener("contextmenu", event => { event.preventDefault(); if (!selection.has(model.id)) select(model); showContextMenu(event.clientX, event.clientY); });
      }
      button.setAttribute("aria-pressed", String(selection.has(model.id)));
      button.dataset.objectId = model.id;
      button.dataset.active = String(model === selected);
      const row = button.parentElement;
      row.classList.toggle("is-hidden", hiddenIds.has(model.id));
      row.classList.toggle("is-locked", lockedIds.has(model.id));
      const lock = row.querySelector("[data-lock-id]");
      lock.textContent = lockedIds.has(model.id) ? "🔒" : "🔓";
      lock.title = lockedIds.has(model.id) ? "ロック解除" : "編集をロック";
      lock.setAttribute("aria-label", `${lock.title} #${model.id}`); lock.setAttribute("aria-pressed", String(lockedIds.has(model.id)));
      const visible = row.querySelector("[data-visibility-id]");
      visible.textContent = hiddenIds.has(model.id) ? "○" : "◉";
      visible.title = hiddenIds.has(model.id) ? "表示する" : "非表示にする";
      visible.setAttribute("aria-label", `${visible.title} #${model.id}`);
      visible.setAttribute("aria-pressed", String(!hiddenIds.has(model.id)));
      button.querySelector("strong").textContent = model.name;
      button.querySelector("small").textContent = `#${model.id} · ${model.assetID}${terrainIds.has(model.id) ? " · 地形" : ""}${model === selected && selection.size > 1 ? " · Active" : ""}`;
      rows.push(row);
    }
    const current = [...$("object-list").children];
    if (current.length !== rows.length || rows.some((row, index) => row !== current[index])) $("object-list").replaceChildren(...rows);
    if (!matches.length) {
      const p = document.createElement("p"); p.className = "empty-list"; p.textContent = models.length ? "一致するアセットIDはありません。" : "FBXを開くと、ここにモデルが表示されます。"; $("object-list").replaceChildren(p);
    }
    $("model-count").textContent = `${matches.length} / ${models.length} 件`;
    $("clear-search").disabled = !$("object-search").value;
    $("frame-all").disabled = !models.length || busy;
    $("empty-view").hidden = !!models.length;
    $("scene-status").textContent = `${models.length} モデル ／ 選択 ${selection.size} 件`;
    $("selection-count").textContent = `選択：${selection.size} 件 ／ 編集 ${editableModels().length} 件`;
    updateButtons();
  }
  $("object-search").addEventListener("input", refreshList);
  $("clear-search").addEventListener("click", () => { $("object-search").value = ""; refreshList(); $("object-search").focus(); });
  function setHidden(ids, hidden) {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
    for (const id of ids) {
      const model = byId.get(id); if (!model) continue;
      if (hidden) hiddenIds.add(id); else hiddenIds.delete(id);
      model.root.visible = !hidden;
    }
    scatter?.invalidate(); setSelection({ ids: [...selection] });
  }
  $("hide-selected").addEventListener("click", () => setHidden([...selection], true));
  $("show-all").addEventListener("click", () => setHidden(models.map(model => model.id), false));
  function setLocked(ids, locked) {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
    for (const id of ids) if (byId.has(id)) { if (locked) lockedIds.add(id); else lockedIds.delete(id); }
    scatter?.invalidate(); setSelection({ ids: [...selection] });
  }
  $("lock-selected").addEventListener("click", () => setLocked([...selection], true));
  $("unlock-all").addEventListener("click", () => setLocked(models.map(model => model.id), false));
  function setSelection(state) {
    selectionBox?.cancel();
    cancelFocus();
    scatter?.stopCenter(false);
    transform.detach(); selection.clear();
    for (const id of state.ids) if (byId.has(id) && !hiddenIds.has(id)) selection.add(id);
    for (const outline of outlines.values()) { scene.remove(outline); outline.geometry.dispose(); outline.material.dispose(); }
    outlines.clear();
    const items = selectedModels();
    const editable = editableModels(), targets = editable.length ? editable : items;
    selected = editable[editable.length - 1] || items[items.length - 1] || null;
    $("selection").hidden = !items.length; $("no-selection").hidden = !!items.length;
    if (items.length) {
      for (const model of items) { const outline = new T.Box3Helper(new T.Box3(), canEdit(model) ? 0xf5bd4f : 0x969da5); scene.add(outline); outlines.set(model.id, outline); }
      if (state.pivot && C.equal(state.editableIds || editable.map(model => model.id), editable.map(model => model.id))) restoreRoot(pivot, state.pivot);
      else if (targets.length === 1) restoreRoot(pivot, snapshotRoot(targets[0].root));
      else {
        const bounds = new T.Box3(); targets.forEach(item => bounds.union(new T.Box3().setFromObject(item.root)));
        bounds.getCenter(pivot.position); pivot.rotation.set(0, 0, 0); pivot.scale.set(1, 1, 1);
      }
      if (targets.length > 1 && spaceMode === "local") pivot.quaternion.copy(targets[targets.length - 1].root.quaternion);
      transform.setSpace(spaceMode);
      if (editable.length) transform.attach(pivot);
      $("selected-name").textContent = items.length === 1 ? selected.name : `${items.length} オブジェクトを選択`;
      $("model-info").textContent = `編集 ${editable.length} / 選択 ${items.length} ／ 頂点 ${items.reduce((sum, item) => sum + item.vertices, 0).toLocaleString()}${items.length > 1 ? ` ／ Active #${selected.id}` : ""}`;
      $("asset-id").disabled = items.length !== 1 || !editable.length;
      $("asset-id").value = items.length === 1 ? selected.assetID : "複数選択";
      $("group-hint").hidden = targets.length < 2;
      $("position-title").textContent = targets.length > 1 ? "位置（編集中心・ワールド）" : "位置（ワールド）";
      $("rotation-title").textContent = targets.length > 1 ? "回転（°・編集グループ）" : "回転（°・ローカル）";
      $("scale-title").textContent = targets.length > 1 ? "スケール（編集グループ倍率）" : "スケール（ローカル）";
      syncTransform();
    }
    refreshList(); requestRender();
  }
  function select(model, event = {}) {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || (model && hiddenIds.has(model.id))) return;
    const ids = new Set(selection);
    if (!model) ids.clear();
    else if (event.ctrlKey || event.metaKey) { if (ids.has(model.id)) ids.delete(model.id); else ids.add(model.id); }
    else if (event.shiftKey) { ids.delete(model.id); ids.add(model.id); }
    else { ids.clear(); ids.add(model.id); }
    setSelection({ ids: [...ids] });
  }
  function updateButtons() {
    const locked = busy || !!transaction || transform.dragging || !!scrub || scatter?.interacting || selectionBox?.interacting;
    $("undo").disabled = locked || !history.canUndo; $("redo").disabled = locked || !history.canRedo;
    $("undo").title = history.canUndo ? `取り消し：${history.commands[history.cursor - 1].label} (Ctrl+Z)` : "取り消し (Ctrl+Z)";
    $("redo").title = history.canRedo ? `やり直し：${history.commands[history.cursor].label} (Ctrl+Y)` : "やり直し (Ctrl+Y)";
    const editable = editableModels();
    for (const id of ["duplicate", "remove-selected", "context-duplicate", "context-delete", "snap-ground", "is-terrain"]) $(id).disabled = locked || !editable.length;
    fields.forEach(field => { field.input.disabled = busy || scatter?.interacting || selectionBox?.interacting || !editable.length; });
    $("hide-selected").disabled = locked || !selection.size;
    $("show-all").disabled = locked || !models.some(model => hiddenIds.has(model.id));
    $("lock-selected").disabled = locked || !editable.length;
    $("unlock-all").disabled = locked || !models.some(model => lockedIds.has(model.id));
    document.querySelectorAll("[data-visibility-id], [data-lock-id]").forEach(button => { button.disabled = locked; });
    $("save-map").disabled = locked;
    $("open-files").disabled = locked;
    for (const id of ["position-snap", "position-step", "rotation-snap", "rotation-step"]) $(id).disabled = locked;
    document.querySelectorAll("[data-space]").forEach(button => { button.disabled = locked; });
    $("open-terrain").disabled = locked;
    $("clear-terrain").disabled = locked || !terrainReferences.size;
    $("align-objects").disabled = locked || editable.length < 2;
    $("space-objects").disabled = locked || editable.length < ($("spacing-kind").value === "ends" ? 3 : 2);
    $("layout-gap").disabled = locked || $("spacing-kind").value === "ends";
    $("map-name").disabled = locked; $("ground-asset-tag").disabled = locked;
    syncAppearanceUI();
    updateTerrainUI();
    scatter?.updateUI();
  }
  $("spacing-kind").addEventListener("change", updateButtons);
  function runPositionPlan(label, plan) {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
    const targets = plan.map(item => ({ ...item, model: byId.get(item.id) })).filter(item => selection.has(item.id) && canEdit(item.model));
    if (!targets.length || targets.some(item => !item.position.every(Number.isFinite))) return;
    beginEdit(label);
    for (const item of targets) { item.model.root.position.fromArray(item.position); item.model.root.updateMatrixWorld(true); }
    setSelection({ ids: [...selection] }); endEdit();
    notice(`${targets.length} 件に「${label}」を適用しました。`);
  }
  function layoutItems() { return editableModels().map(model => ({ id: model.id, position: model.root.position.toArray() })); }
  $("align-objects").addEventListener("click", () => {
    try { runPositionPlan("整列", placementTools.alignmentPlan(layoutItems(), $("layout-axis").value, $("align-kind").value)); }
    catch (error) { notice(error.message, "error"); }
  });
  $("space-objects").addEventListener("click", () => {
    try { runPositionPlan("等間隔配置", placementTools.spacingPlan(layoutItems(), $("layout-axis").value, $("spacing-kind").value, $("spacing-kind").value === "ends" ? 0 : $("layout-gap").valueAsNumber)); }
    catch (error) { notice(error.message, "error"); }
  });
  function setPresent(entries, present) {
    if (present) {
      for (const { model, index } of [...entries].sort((a, b) => a.index - b.index)) {
        if (byId.has(model.id)) continue;
        models.splice(Math.min(index, models.length), 0, model); byId.set(model.id, model); model.root.visible = !hiddenIds.has(model.id); scene.add(model.root);
      }
    } else {
      for (const { model } of entries) {
        const active = byId.get(model.id); if (!active) continue;
        scene.remove(active.root); models.splice(models.indexOf(active), 1); byId.delete(active.id);
      }
    }
  }
  function objectCommand(label, items, adding, afterIds) {
    const beforeSelection = selectionState();
    const entries = items.map((model, offset) => ({ model, index: adding ? models.length + offset : models.indexOf(model) }));
    history.commit(new C.ObjectsCommand(label, entries, adding, beforeSelection, { ids: afterIds }, setPresent, setSelection));
  }
  function deleteSelection() {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || !editableModels().length) return;
    objectCommand("削除", editableModels(), false, selectedModels().filter(model => !canEdit(model)).map(model => model.id)); hideContextMenu();
  }
  function cloneRoot(original) {
    const root = original.clone(true), oldNodes = [], newNodes = [];
    original.traverse(node => oldNodes.push(node)); root.traverse(node => newNodes.push(node));
    const copies = new Map(oldNodes.map((node, index) => [node, newNodes[index]]));
    oldNodes.forEach(node => {
      if (!node.isSkinnedMesh) return;
      const copy = copies.get(node); copy.skeleton = node.skeleton.clone();
      copy.skeleton.bones = node.skeleton.bones.map(bone => copies.get(bone));
    });
    return root;
  }
  function makeModel(source, assetID, state = null, register = true) {
    const root = cloneRoot(source.root), id = register ? ++sequence : null;
    if (register) root.userData.mapEditorID = id; else delete root.userData.mapEditorID;
    if (state) restoreRoot(root, state);
    const model = { id, assetID, source, name: source.name, root, meshCount: source.meshCount, vertices: source.vertices };
    appearance.setAsset(root, assetID);
    if (register) instances.set(id, model); return model;
  }
  function discardInstance(model) {
    appearance.release(model.root);
    model.root.removeFromParent();
    model.root.traverse(node => { if (node.isSkinnedMesh) node.skeleton.dispose(); });
  }
  function releaseUnusedInstances() {
    const retained = new Set(byId.keys());
    for (const command of history.commands) for (const entry of command.entries || []) retained.add(entry.model.id);
    for (const [id, model] of instances) {
      if (retained.has(id)) continue;
      // Geometry/materials belong to cached FBX sources. Only the independently
      // cloned skeleton belongs to an instance; releasing it cannot break copies.
      discardInstance(model);
      instances.delete(id); terrainIds.delete(id); hiddenIds.delete(id); lockedIds.delete(id);
    }
  }
  function duplicateSelection() {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || !editableModels().length) return;
    const copies = editableModels().map(model => {
      const copy = makeModel(model.source, model.assetID, snapshotRoot(model.root));
      appearance.copyColor(model.root, copy.root);
      if (terrainIds.has(model.id)) terrainIds.add(copy.id);
      return copy;
    });
    objectCommand("複製", copies, true, copies.map(model => model.id));
    setMode("translate"); hideContextMenu(); viewport.focus({ preventScroll: true });
  }
  $("undo").addEventListener("click", () => history.undo());
  $("redo").addEventListener("click", () => history.redo());
  $("duplicate").addEventListener("click", duplicateSelection);
  $("context-duplicate").addEventListener("click", duplicateSelection);
  $("context-delete").addEventListener("click", deleteSelection);
  $("remove-selected").addEventListener("click", deleteSelection);
  function terrainRoots() {
    return [...terrainReferences.values()].map(source => source.root).concat(models.filter(model => terrainIds.has(model.id)).map(model => model.root));
  }
  function updateTerrainUI() {
    const items = editableModels(), count = items.filter(item => terrainIds.has(item.id)).length;
    $("is-terrain").checked = !!items.length && count === items.length;
    $("is-terrain").indeterminate = count > 0 && count < items.length;
    $("terrain-count").textContent = `${terrainReferences.size + models.filter(model => terrainIds.has(model.id)).length} 件`;
  }
  $("is-terrain").addEventListener("change", event => {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
    for (const { id } of editableModels()) { if (event.target.checked) terrainIds.add(id); else terrainIds.delete(id); }
    scatter?.invalidate();
    refreshList();
  });
  $("snap-ground").addEventListener("click", () => {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging || !selection.size) return;
    const roots = terrainRoots();
    if (!roots.length) { notice("参照地形FBXを読み込むか、配置済みモデルを吸着先の地形に指定してください。", "warning"); return; }
    const targets = editableModels().filter(model => !terrainIds.has(model.id));
    if (!targets.length) { notice("地形以外の吸着対象を選択してください。地形自身は移動しません。", "warning"); return; }
    const positions = targets.map(model => ({ model, y: geometryTools.groundY(model.root, roots) }));
    const found = positions.filter(item => item.y !== null);
    if (!found.length) { notice("選択対象の下に地面が見つかりませんでした。配置は変更していません。", "warning"); return; }
    beginEdit("地面への吸着");
    for (const item of found) { item.model.root.position.y = item.y; item.model.root.updateMatrixWorld(true); }
    setSelection({ ids: [...selection] }); endEdit();
    const missing = targets.length - found.length;
    notice(`${found.length} 件を地面へ吸着しました。${missing ? `地面が見つからない ${missing} 件は変更していません。` : ""}`, missing ? "warning" : "");
  });
  $("open-terrain").addEventListener("click", () => $("terrain-input").click());
  $("terrain-input").addEventListener("change", event => {
    const files = Array.from(event.target.files); event.target.value = ""; importTerrain(files);
  });
  $("clear-terrain").addEventListener("click", () => {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
    for (const source of terrainReferences.values()) { scene.remove(source.root); dispose(source.root); }
    terrainReferences.clear(); updateButtons(); requestRender();
    scatter?.invalidate();
    notice("参照地形をクリアしました。配置済みモデルの地形指定は維持しています。");
  });
  $("asset-id").addEventListener("change", event => {
    if (!selected || selection.size !== 1 || busy || !canEdit(selected)) return;
    const id = event.target.value.trim();
    if (!id) { event.target.value = selected.assetID; notice("アセットIDを入力してください。", "error"); return; }
    beginEdit("アセットID"); selected.assetID = id; appearance.setAsset(selected.root, id); sourceByAsset.set(id, selected.source); endEdit(); refreshList();
  });
  for (const property of ["position", "rotation"]) {
    $(property + "-snap").addEventListener("change", event => { snapSettings[property] = event.target.checked; });
    $(property + "-step").addEventListener("change", event => {
      const value = event.target.valueAsNumber;
      if (!Number.isFinite(value) || value < 0.000001 || (property === "rotation" && value > 360)) {
        event.target.value = snapSettings[property + "Step"]; notice("スナップ間隔は正の数値、回転角度は360°以下にしてください。", "error"); return;
      }
      snapSettings[property + "Step"] = value;
    });
  }
  function hideContextMenu() { $("context-menu").hidden = true; }
  function showContextMenu(x, y) {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting) return;
    const menu = $("context-menu"); updateButtons(); menu.hidden = false;
    menu.style.left = `${Math.max(0, Math.min(x, window.innerWidth - menu.offsetWidth))}px`;
    menu.style.top = `${Math.max(0, Math.min(y, window.innerHeight - menu.offsetHeight))}px`;
  }
  document.addEventListener("pointerdown", event => { if (!$("context-menu").contains(event.target)) hideContextMenu(); });
  window.addEventListener("resize", hideContextMenu);
  function cancelFocus() { focusAnimation = null; }
  function finishFocus(frame) {
    camera.near = frame.near; camera.far = frame.far; camera.updateProjectionMatrix();
    controls.minDistance = frame.radius / 1000; controls.maxDistance = frame.far / 2;
  }
  function advanceFocus(time) {
    if (!focusAnimation) return;
    const animation = focusAnimation;
    if (animation.camera !== camera) { cancelFocus(); return; }
    const progress = Math.min(1, Math.max(0, (time - animation.start) / 240));
    const t = progress * progress * (3 - 2 * progress);
    camera.position.lerpVectors(animation.fromPosition, animation.toPosition, t);
    controls.target.lerpVectors(animation.fromTarget, animation.frame.center, t);
    if (camera.isOrthographicCamera) { orthoHeight = animation.fromHeight + (animation.frame.height - animation.fromHeight) * t; resizeOrthographic(); }
    controls.update();
    if (progress === 1) { finishFocus(animation.frame); focusAnimation = null; }
  }
  function frame(items, animate = false) {
    items = items.filter(model => !hiddenIds.has(model.id));
    if (!items.length) return;
    if (animate && (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging)) return;
    cancelFocus();
    const bounds = geometryTools.boundsOf(items.map(item => item.root));
    const targetFrame = geometryTools.focusFrame(bounds, camera, viewport.clientWidth / Math.max(viewport.clientHeight, 1));
    if (!targetFrame) return;
    const direction = camera.position.clone().sub(controls.target).normalize();
    if (!direction.lengthSq()) direction.set(1, 0.8, 1).normalize();
    const toPosition = targetFrame.center.clone().addScaledVector(direction, targetFrame.distance);
    const fromHeight = orthoHeight / camera.zoom;
    if (camera.isOrthographicCamera) { camera.zoom = 1; orthoHeight = fromHeight; resizeOrthographic(); }
    if (animate) {
      focusAnimation = { camera, frame: targetFrame, start: performance.now(), fromPosition: camera.position.clone(), fromTarget: controls.target.clone(), toPosition, fromHeight };
      camera.near = Math.min(camera.near, targetFrame.near); camera.far = Math.max(camera.far, targetFrame.far);
      camera.updateProjectionMatrix();
      controls.minDistance = Math.min(camera.position.distanceTo(controls.target), targetFrame.radius / 1000);
      controls.maxDistance = Math.max(camera.position.distanceTo(controls.target) * 2, targetFrame.far / 2);
    } else {
      camera.position.copy(toPosition); controls.target.copy(targetFrame.center);
      if (camera.isOrthographicCamera) { orthoHeight = targetFrame.height; resizeOrthographic(); }
      finishFocus(targetFrame); controls.update();
    }
    setGrid(Math.pow(10, Math.floor(Math.log10(targetFrame.radius)))); requestRender();
  }
  let rightPress = null;
  renderer.domElement.addEventListener("pointerdown", e => {
    if (e.button === 2 && !busy && !selectionBox?.interacting) rightPress = { x: e.clientX, y: e.clientY, id: e.pointerId };
  });
  renderer.domElement.addEventListener("pointercancel", () => { rightPress = null; selectionBox?.cancel(); cancelEdit(); });
  function pickModel(event) {
    const rect = renderer.domElement.getBoundingClientRect();
    pointer.set((event.clientX - rect.left) / rect.width * 2 - 1, -(event.clientY - rect.top) / rect.height * 2 + 1);
    scene.updateMatrixWorld(true); camera.updateMatrixWorld(true); raycaster.setFromCamera(pointer, camera);
    const hits = raycaster.intersectObjects(models.filter(canEdit).map(model => model.root), true);
    let target = hits.find(hit => hit.object.isMesh)?.object;
    while (target && !target.userData.mapEditorID) target = target.parent;
    return byId.get(target?.userData.mapEditorID) || null;
  }
  renderer.domElement.addEventListener("pointerup", e => {
    if (e.button === 2) {
      const right = rightPress; rightPress = null;
      if (right && right.id === e.pointerId && Math.hypot(e.clientX - right.x, e.clientY - right.y) < 5) {
        const model = pickModel(e); if (model && !selection.has(model.id)) select(model);
        showContextMenu(e.clientX, e.clientY);
      }
      return;
    }
  });
  selectionBox = window.createMapEditorSelection({
    canvas: renderer.domElement, viewport, overlay: $("selection-rectangle"),
    canStart: () => !busy && !transaction && !scrub && !scatter?.interacting && !transform.dragging,
    isGizmo: () => transform.dragging || transform.axis !== null,
    selectedIDs: () => [...selection], click: event => select(pickModel(event), event),
    interactionChanged: active => {
      cancelFocus(); if (!active) transform.axis = null;
      controls.enabled = !active && !busy && !transform.dragging; transform.enabled = !active && !busy;
      updateButtons();
    },
    selectRectangle: (rect, modifiers, previous) => {
      if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
      const size = renderer.domElement.getBoundingClientRect();
      const candidates = models.filter(canEdit);
      const roots = new Set(geometryTools.rectangleSelection(candidates.map(model => model.root), camera, {
        left: rect.left / size.width * 2 - 1, right: rect.right / size.width * 2 - 1,
        top: 1 - rect.top / size.height * 2, bottom: 1 - rect.bottom / size.height * 2,
      }));
      const ids = new Set(modifiers.ctrlKey || modifiers.shiftKey ? previous : []);
      for (const model of candidates) if (roots.has(model.root)) {
        if (modifiers.ctrlKey && ids.has(model.id)) ids.delete(model.id); else ids.add(model.id);
      }
      setSelection({ ids: [...ids] });
    },
  });
  fields.forEach(field => {
    const { input, property } = field;
    input.title = "左右ドラッグ：値を変更 ／ クリック：直接入力 ／ Shift：微調整";
    input.addEventListener("change", () => {
      if (!editableModels().length || busy || scatter?.interacting || selectionBox?.interacting) { syncTransform(); return; }
      const value = input.valueAsNumber;
      if (input.value === "" || !Number.isFinite(value) || (property === "scale" && value < minScale)) {
        transformValidationMessage = property === "scale" ? "スケールには0.001以上の有限の数値を入力してください。" : "有限の数値を入力してください。";
        notice(transformValidationMessage, "error");
        syncTransform(); return;
      }
      beginEdit("数値入力"); applyValue(field, value); endEdit();
    });
    const label = input.previousElementSibling;
    label.title = input.title;
    for (const handle of [input, label]) handle.addEventListener("pointerdown", event => {
      if (event.button !== 0 || !editableModels().length || busy || transform.dragging || scrub || scatter?.interacting || selectionBox?.interacting) return;
      // A focused input keeps native caret/selection behavior for direct typing.
      if (handle === input && document.activeElement === input) return;
      event.preventDefault();
      if (document.activeElement instanceof HTMLElement) document.activeElement.blur();
      scrub = { field, handle, id: event.pointerId, startX: event.clientX, lastX: event.clientX, value: displayValue(field), dragging: false };
      handle.setPointerCapture(event.pointerId);
    });
  });
  function finishScrub(cancel = false) {
    if (!scrub) return;
    const state = scrub; scrub = null;
    document.body.classList.remove("scrubbing");
    if (state.dragging) endEdit(cancel);
    if (state.handle.hasPointerCapture(state.id)) state.handle.releasePointerCapture(state.id);
    if (!cancel && !state.dragging) { state.field.input.focus(); state.field.input.select(); }
    updateButtons();
  }
  document.addEventListener("pointermove", event => {
    if (!scrub || event.pointerId !== scrub.id) return;
    if (!scrub.dragging && Math.abs(event.clientX - scrub.startX) < 4) return;
    if (!scrub.dragging) beginEdit("数値ドラッグ", "scrub");
    scrub.dragging = true; event.preventDefault(); document.body.classList.add("scrubbing");
    const delta = (event.clientX - scrub.lastX) * scrub.field.sensitivity * (event.shiftKey ? 0.1 : 1);
    scrub.lastX = event.clientX;
    const value = scrub.value + delta;
    if (!Number.isFinite(value)) return;
    scrub.value = scrub.field.property === "scale" ? Math.max(minScale, value) : value;
    applyValue(scrub.field, scrub.value, true);
  }, { passive: false });
  document.addEventListener("pointerup", event => { if (scrub?.id === event.pointerId) finishScrub(); });
  document.addEventListener("pointercancel", event => { if (scrub?.id === event.pointerId) finishScrub(true); });
  document.addEventListener("lostpointercapture", event => { if (scrub?.id === event.pointerId) finishScrub(true); });
  window.addEventListener("blur", () => { finishScrub(true); cancelEdit(); hideContextMenu(); cancelFocus(); });
  $("frame-all").addEventListener("click", () => frame(models, true));
  $("frame-selected").addEventListener("click", () => frame(selectedModels(), true));
  function dispose(root) {
    appearance.release(root);
    const resources = new Set();
    root.traverse(child => {
      if (child.geometry) resources.add(child.geometry);
      if (child.skeleton) resources.add(child.skeleton);
      for (const material of child.material ? Array.isArray(child.material) ? child.material : [child.material] : []) {
        resources.add(material);
        for (const value of Object.values(material)) if (value?.isTexture) resources.add(value);
      }
    });
    resources.forEach(resource => resource.dispose());
  }
  document.addEventListener("keydown", e => {
    if ($("save-dialog").open) return;
    if (e.key === "Escape") { if (scatter?.cancelInteraction()) { e.preventDefault(); transform.dragging = false; transform.axis = null; controls.enabled = !busy; updateButtons(); } else if (scrub) { e.preventDefault(); finishScrub(true); } else cancelEdit(); hideContextMenu(); document.querySelector(".help").open = false; return; }
    if (busy || scrub || scatter?.interacting || selectionBox?.interacting || transaction || transform.dragging || document.activeElement.isContentEditable || /^(INPUT|TEXTAREA|SELECT|SUMMARY)$/.test(document.activeElement.tagName)) return;
    if (e.ctrlKey || e.metaKey) {
      const key = e.key.toLowerCase();
      if (key === "z") { e.preventDefault(); e.shiftKey ? history.redo() : history.undo(); }
      if (key === "y") { e.preventDefault(); history.redo(); }
      if (key === "d") { e.preventDefault(); duplicateSelection(); }
      return;
    }
    if (e.altKey) return;
    if (e.key === "Delete") { e.preventDefault(); deleteSelection(); return; }
    const view = { "1": "front", "2": "side", "3": "top", "4": "perspective" }[e.key];
    const mode = { w: "translate", e: "rotate", r: "scale" }[e.key.toLowerCase()];
    if (view) { e.preventDefault(); setView(view); }
    if (mode) { e.preventDefault(); setMode(mode); }
    if (e.key.toLowerCase() === "f") { e.preventDefault(); if (scatter?.editingCenter) scatter.focusCenter(); else frame(selectedModels(), true); }
  });

  const basename = name => name.replace(/\\/g, "/").split("/").pop().toLowerCase();
  function textureFiles(files) {
    const textures = new Map();
    for (const file of files.filter(file => !/\.(fbx|json)$/i.test(file.name))) {
      const name = basename(file.name); if (!textures.has(name)) textures.set(name, []); textures.get(name).push(file);
    }
    return textures;
  }
  function setBusy(value) {
    if (value) selectionBox?.cancel();
    busy = value; cancelFocus();
    $("object-list").inert = value; $("selection").inert = value;
    controls.enabled = !value; transform.enabled = !value;
    $("frame-all").disabled = value || !models.length;
    updateButtons();
  }
  const blankTexture = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVQIHWP4////fwAJ+wP9KobjigAAAABJRU5ErkJggg==";
  async function loadFBX(file, textures) {
    const bytes = await file.arrayBuffer();
    const manager = new T.LoadingManager(), urls = new Set(), missing = new Set();
    let finish;
    const loaded = new Promise(resolve => { finish = resolve; });
    manager.onLoad = finish;
    manager.onError = url => missing.add(url.startsWith("blob:") || url.startsWith("data:") ? "読み込めない画像" : basename(url));
    manager.setURLModifier(url => {
      if (url.startsWith("blob:")) { urls.add(url); return url; }
      if (url.startsWith("data:")) return url;
      let name;
      try { name = basename(decodeURIComponent(url)); } catch { name = basename(url); }
      const matches = textures.get(name);
      if (matches?.length === 1) {
        const objectURL = URL.createObjectURL(matches[0]); urls.add(objectURL); return objectURL;
      }
      missing.add(matches?.length > 1 ? `${name}（同名画像が複数）` : name);
      return blankTexture;
    });
    // The sentinel prevents a texture-free FBX from leaving the load promise pending.
    manager.itemStart("model-parse");
    let imported, parseError;
    try { imported = new T.FBXLoader(manager).parse(bytes, ""); }
    catch (error) { parseError = error; }
    finally { manager.itemEnd("model-parse"); }
    await loaded;
    urls.forEach(url => URL.revokeObjectURL(url));
    if (parseError) throw parseError;
    let meshCount = 0, vertices = 0;
    imported.traverse(child => {
      if (child.isCamera || child.isLight) child.visible = false;
      if (!child.isMesh) return;
      meshCount++; vertices += child.geometry.getAttribute("position")?.count || 0;
      for (const material of Array.isArray(child.material) ? child.material : [child.material]) {
        for (const [key, value] of Object.entries(material)) {
          if (value?.isTexture && (!value.image || value.image.src === blankTexture || value.image.naturalWidth === 0)) {
            value.dispose(); material[key] = null; material.needsUpdate = true;
            if (!missing.size) missing.add("画像未設定");
          }
        }
      }
    });
    const bounds = new T.Box3().setFromObject(imported);
    if (!meshCount || bounds.isEmpty() || ![...bounds.min.toArray(), ...bounds.max.toArray()].every(Number.isFinite)) {
      dispose(imported); throw new Error("表示できるメッシュがないか、頂点座標が不正です。");
    }
    const root = new T.Group(); root.rotation.order = C.rotationOrder; root.add(imported);
    return { key: ++sourceSequence, name: file.name, root, meshCount, vertices, missing: [...missing] };
  }
  function updateAssetOptions() {
    const ids = new Set([...assetDefinitions.keys(), ...sourceByAsset.keys()]);
    $("asset-options").replaceChildren(...[...ids].sort().map(id => { const option = document.createElement("option"); option.value = id; return option; }));
    scatter?.invalidate(); scatter?.refreshAssets();
  }
  async function importTerrain(files) {
    if (busy || transaction || scrub || scatter?.interacting || selectionBox?.interacting || transform.dragging) return;
    const fbxs = files.filter(file => /\.fbx$/i.test(file.name));
    if (!fbxs.length) { notice("参照地形のFBXを選択してください。画像はFBXと一緒に指定できます。", "warning"); return; }
    const staged = [], textures = textureFiles(files);
    setBusy(true);
    try {
      for (const file of fbxs) {
        notice(`${file.name} を参照地形として読み込み中…`);
        await new Promise(resolve => setTimeout(resolve, 0));
        staged.push(await loadFBX(file, textures));
      }
      for (const source of staged) { terrainReferences.set(source.key, source); scene.add(source.root); }
      scatter?.invalidate();
      const missing = staged.some(source => source.missing.length);
      notice(`${staged.length} 件の参照地形を追加しました。JSONの配置データには含めません。${missing ? "一部の画像は省略しました。" : ""}`, missing ? "warning" : "");
    } catch (error) {
      for (const source of staged) if (!terrainReferences.has(source.key)) dispose(source.root);
      notice(`参照地形の読み込みを中止しました：${error.message || String(error)}`, "error");
    } finally { setBusy(false); requestRender(); }
  }
  function defaultAssetID(source, definitions) {
    const candidates = [...definitions].filter(([, entry]) => basename(entry.modelPath) === basename(source.name)).map(([id]) => id);
    const stem = source.name.replace(/\.fbx$/i, "");
    return candidates.length === 1 ? candidates[0] : stem;
  }
  function resolveSource(assetID, available, definitions) {
    const definition = definitions.get(assetID);
    if (definition) {
      const matches = available.filter(source => basename(source.name) === basename(definition.modelPath));
      if (matches.length) return matches[matches.length - 1];
    }
    return sourceByAsset.get(assetID) || available.find(source => defaultAssetID(source, definitions) === assetID);
  }
  async function importFiles(files) {
    if (busy || transaction || transform.dragging) return;
    finishScrub();
    const all = Array.from(files), fbxs = all.filter(file => /\.fbx$/i.test(file.name));
    const jsonFiles = all.filter(file => /\.json$/i.test(file.name));
    if (!fbxs.length && !jsonFiles.length) { notice("FBXまたはJSONを選択してください。外部テクスチャはFBXと一緒に選択できます。", "warning"); return; }
    const textures = textureFiles(all);
    setBusy(true);
    const loadedSources = [], warnings = [];
    try {
      let layout = null, importedName = "";
      const definitions = new Map(assetDefinitions);
      for (const file of jsonFiles) {
        const json = JSON.parse((await file.text()).replace(/^\uFEFF/, ""));
        if (json && Array.isArray(json.objects) && !Object.hasOwn(json, "mapName") && json.objects.every(item => item && typeof item.assetID === "string" && item.assetID.trim() && typeof item.modelPath === "string")) {
          for (const entry of json.objects) definitions.set(entry.assetID, { modelPath: entry.modelPath });
        } else {
          if (layout) throw new Error("配置JSONは一度に1ファイルだけ選択してください。");
          layout = C.validateMap(json);
          importedName = file.name;
        }
      }
      for (const [index, file] of fbxs.entries()) {
        notice(`${file.name} を読み込み中… (${index + 1}/${fbxs.length})`);
        // Yield so the progress message paints before synchronous FBX parsing.
        await new Promise(resolve => setTimeout(resolve, 0));
        const source = await loadFBX(file, textures); loadedSources.push(source);
        if (source.missing.length) warnings.push(`${file.name}：省略した画像 ${source.missing.join("、")}`);
      }
      const available = [...sources.values(), ...loadedSources];
      const missing = layout ? [...new Set(layout.objects.filter(object => !resolveSource(object.assetID, available, definitions)).map(object => object.assetID))] : [];
      if (missing.length) throw new Error(`未読み込みのモデル：${missing.join("、")}。対応するFBXとMAP_OBJECT_ASSETS.jsonも一緒に選択するか、FBXのアセットIDを設定してください。`);
      // Commit only after the complete import has validated. Failed reloads keep
      // the current scene, selection and undo history intact.
      for (const [id, entry] of definitions) assetDefinitions.set(id, entry);
      for (const source of loadedSources) { sources.set(source.key, source); sourceByAsset.set(defaultAssetID(source, definitions), source); }
      let result;
      if (layout) {
        const replacements = layout.objects.map(object => makeModel(resolveSource(object.assetID, available, definitions), object.assetID, C.fromGameTransform(object)));
        setSelection({ ids: [] }); setPresent(models.map((model, index) => ({ model, index })), false);
        mapDocument = structuredClone(layout);
        layoutFileName = importedName; syncMapInfo();
        terrainIds.clear();
        hiddenIds.clear();
        lockedIds.clear();
        setPresent(replacements.map((model, index) => ({ model, index })), true);
        history.clear(); setSelection({ ids: [] }); frame(models);
        result = `${layout.mapName}：${replacements.length} 件の配置を復元しました。再読み込み前の履歴はクリアしました。`;
      } else {
        const added = loadedSources.map(source => makeModel(source, defaultAssetID(source, definitions)));
        if (added.length) { objectCommand("FBX配置", added, true, added.map(model => model.id)); frame(models); }
        result = `${added.length} 件のFBXを追加しました。`;
      }
      updateAssetOptions();
      notice([result, ...warnings].join("\n"), warnings.length ? "warning" : "");
    } catch (error) {
      for (const source of loadedSources) if (!sources.has(source.key)) dispose(source.root);
      notice(`読み込みを中止しました：${error.message || String(error)}`, "error");
    } finally {
      setBusy(false); refreshList(); requestRender();
    }
  }
  const exporter = window.createMapEditorExport({
    canSave: () => !busy && !transaction && !scrub && !scatter?.interacting && !selectionBox?.interacting && !transform.dragging,
    validate: () => {
      if (mapDocument.mapName.trim()) return true;
      $("map-info").open = true; $("map-name").focus(); notice("マップ名を入力してください。", "error"); return false;
    },
    getData: () => C.serializeMap(mapDocument, models),
    suggestedName: () => layoutFileName || mapDocument.mapName,
    fileName: C.jsonFileName, saved: name => { layoutFileName = name; }, setBusy, notice,
  });
  $("save-map").addEventListener("click", exporter.save);
  $("open-files").addEventListener("click", () => $("file-input").click());
  $("file-input").addEventListener("change", e => { const files = Array.from(e.target.files); e.target.value = ""; importFiles(files); });
  let dragDepth = 0;
  document.addEventListener("dragenter", e => { if (Array.from(e.dataTransfer.types).includes("Files")) { e.preventDefault(); dragDepth++; document.body.classList.add("dragging"); } });
  document.addEventListener("dragover", e => { if (Array.from(e.dataTransfer.types).includes("Files")) e.preventDefault(); });
  document.addEventListener("dragleave", () => { if (--dragDepth <= 0) { dragDepth = 0; document.body.classList.remove("dragging"); } });
  document.addEventListener("drop", e => { e.preventDefault(); dragDepth = 0; document.body.classList.remove("dragging"); if (e.dataTransfer.files.length) importFiles(e.dataTransfer.files); });
  window.addEventListener("beforeunload", e => { if (models.length || busy) { e.preventDefault(); e.returnValue = ""; } });
  scatter = window.createMapEditorScatter({
    T, placement: placementTools, geometryTools, scene,
    centerTarget: (target, restoreSelection = true) => {
      cancelFocus(); transform.detach();
      if (target) { centerPreviousMode = transform.mode; transform.setMode("translate"); transform.setSpace("world"); transform.attach(target); }
      else {
        transform.setMode(centerPreviousMode || "translate"); centerPreviousMode = null; transform.setSpace(spaceMode);
        if (restoreSelection) setSelection(selectionState());
      }
      document.querySelectorAll("[data-mode]").forEach(button => button.setAttribute("aria-pressed", String(button.dataset.mode === transform.mode)));
      document.querySelectorAll("[data-space]").forEach(button => button.setAttribute("aria-pressed", String(button.dataset.space === transform.space)));
      requestRender();
    },
    snapCenter: (position, start) => { if (start && snapSettings.position) axes.forEach(axis => { position[axis] = C.snapDrag(position[axis], start[axis], snapSettings.positionStep); }); },
    interactionChanged: () => { cancelFocus(); transform.enabled = !scatter?.interacting && !selectionBox?.interacting && !busy; updateButtons(); },
    assets: () => {
      const ids = new Set([...assetDefinitions.keys(), ...sourceByAsset.keys()]);
      return [...ids].sort().map(assetID => ({ assetID, source: resolveSource(assetID, [...sources.values()], assetDefinitions) })).filter(asset => asset.source);
    },
    existing: () => models, grounds: terrainRoots,
    canStart: () => !busy && !transaction && !scrub && !selectionBox?.interacting && !transform.dragging,
    create: (source, assetID, state) => makeModel(source, assetID, state, false),
    restore: restoreRoot, discard: discardInstance,
    render: requestRender, notice, focus: items => frame(items, true),
    setGenerating: value => { setBusy(value); if (value) controls.enabled = true; },
    commit: copies => {
      for (const model of copies) { model.id = ++sequence; model.root.userData.mapEditorID = model.id; instances.set(model.id, model); }
      objectCommand("ランダム配置", copies, true, copies.map(model => model.id)); setMode("translate");
      notice(`${copies.length} 件のランダム配置を確定しました。`);
    },
  });
  for (const [button, panel] of [["open-layout", "layout-tools"], ["open-random", "random-tools"]]) $(button).addEventListener("click", () => {
    $(panel).open = true; $(panel).scrollIntoView({ block: "nearest" }); document.querySelector(".tool-menu").open = false;
  });
  updateButtons();
  notice("FBXを開いてください。外部テクスチャがある場合は、画像も一緒に選択できます。");
  requestRender();
})();
