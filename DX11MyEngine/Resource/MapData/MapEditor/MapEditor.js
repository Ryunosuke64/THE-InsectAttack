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
  let renderer;
  try { renderer = new T.WebGLRenderer({ antialias: true }); }
  catch { notice("3D表示を開始できませんでした。WebGL 2に対応したブラウザーとGPU設定を確認してください。", "error"); return; }
  const viewport = $("viewport");
  renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
  viewport.prepend(renderer.domElement);
  const scene = new T.Scene();
  scene.background = new T.Color(0xe4e7ea);
  const camera = new T.PerspectiveCamera(50, 1, 0.01, 100000);
  camera.position.set(100, 80, 120);
  scene.add(new T.HemisphereLight(0xffffff, 0x777f89, 2));
  const sunlight = new T.DirectionalLight(0xffffff, 2.5);
  sunlight.position.set(100, 150, 80);
  scene.add(sunlight);
  const controls = new T.OrbitControls(camera, renderer.domElement);
  controls.enableDamping = false;
  controls.mouseButtons = { LEFT: null, MIDDLE: T.MOUSE.PAN, RIGHT: T.MOUSE.ROTATE };
  const transform = new T.TransformControls(camera, renderer.domElement);
  transform.setMode("translate"); transform.setSpace("world"); transform.setSize(0.9);
  scene.add(transform.getHelper());
  const outline = new T.Box3Helper(new T.Box3(), 0x28649b);
  outline.visible = false; scene.add(outline);
  let grid = null, selected = null, busy = false, sequence = 0, framePending = false;
  const models = [];
  const raycaster = new T.Raycaster();
  const pointer = new T.Vector2();
  const positionInputs = [$("position-x"), $("position-y"), $("position-z")];
  const axes = ["x", "y", "z"];

  function requestRender() {
    if (framePending) return;
    framePending = true;
    requestAnimationFrame(() => {
      framePending = false;
      if (selected) outline.box.setFromObject(selected.root);
      renderer.render(scene, camera);
    });
  }
  function setGrid(step) {
    if (grid) { scene.remove(grid); grid.geometry.dispose(); grid.material.dispose(); }
    grid = new T.GridHelper(step * 20, 20, 0x8c959f, 0xbac0c6);
    scene.add(grid);
    $("grid-status").textContent = `グリッド：${Number(step.toPrecision(4))} 単位`;
  }
  setGrid(10);
  controls.addEventListener("change", requestRender);
  transform.addEventListener("change", requestRender);
  transform.addEventListener("dragging-changed", e => { controls.enabled = !e.value && !busy; });
  transform.addEventListener("objectChange", () => { syncPosition(); requestRender(); });
  renderer.domElement.addEventListener("contextmenu", e => e.preventDefault());
  renderer.domElement.addEventListener("webglcontextlost", e => {
    e.preventDefault(); notice("GPUとの接続が失われました。配置は保存できないため、状況を確認してからページを再読み込みしてください。", "error");
  });
  renderer.domElement.addEventListener("webglcontextrestored", () => { notice("3D表示を再開しました。"); requestRender(); });
  new ResizeObserver(() => {
    const width = viewport.clientWidth, height = viewport.clientHeight;
    if (!width || !height) return;
    renderer.setSize(width, height, false); camera.aspect = width / height; camera.updateProjectionMatrix(); requestRender();
  }).observe(viewport);

  function syncPosition() {
    if (!selected) return;
    axes.forEach((axis, index) => { positionInputs[index].value = Number(selected.root.position[axis].toFixed(6)); });
  }
  function refreshList() {
    $("object-list").replaceChildren();
    for (const model of models) {
      const button = document.createElement("button");
      button.className = "object-item";
      button.setAttribute("aria-pressed", String(selected === model));
      const title = document.createElement("strong"), detail = document.createElement("small");
      title.textContent = model.name; detail.textContent = `#${model.id} · メッシュ ${model.meshCount}`;
      button.append(title, detail); button.addEventListener("click", () => select(model));
      $("object-list").append(button);
    }
    if (!models.length) {
      const p = document.createElement("p"); p.className = "empty-list"; p.textContent = "FBXを開くと、ここにモデルが表示されます。"; $("object-list").append(p);
    }
    $("model-count").textContent = `${models.length} 件`;
    $("frame-all").disabled = !models.length || busy;
    $("empty-view").hidden = !!models.length;
    $("scene-status").textContent = `${models.length} モデル${selected ? ` ／ 選択：${selected.name}` : " ／ 選択なし"}`;
  }
  function select(model) {
    selected = model;
    transform.detach(); outline.visible = !!model;
    $("selection").hidden = !model; $("no-selection").hidden = !!model;
    if (model) {
      transform.attach(model.root);
      $("selected-name").textContent = model.name;
      $("model-info").textContent = `メッシュ ${model.meshCount} ／ 頂点 ${model.vertices.toLocaleString()}`;
      syncPosition();
    }
    refreshList(); requestRender();
  }
  function frame(items) {
    if (!items.length) return;
    const bounds = new T.Box3();
    for (const item of items) {
      bounds.union(new T.Box3().setFromObject(item.root));
      // Keep the original FBX origin visible so its move handles remain reachable.
      bounds.expandByPoint(item.root.position);
    }
    if (bounds.isEmpty()) return;
    const center = bounds.getCenter(new T.Vector3());
    const radius = Math.max(bounds.getSize(new T.Vector3()).length() / 2, 0.001);
    const vertical = camera.fov * Math.PI / 180;
    const horizontal = 2 * Math.atan(Math.tan(vertical / 2) * camera.aspect);
    const distance = radius / Math.sin(Math.min(vertical, horizontal) / 2) * 1.25;
    const direction = camera.position.clone().sub(controls.target).normalize();
    if (!direction.lengthSq()) direction.set(1, 0.8, 1).normalize();
    controls.target.copy(center); camera.position.copy(center).addScaledVector(direction, distance);
    camera.near = Math.max(radius / 10000, 0.00001); camera.far = Math.max(distance * 100, 1000);
    camera.updateProjectionMatrix(); controls.minDistance = radius / 1000; controls.maxDistance = camera.far / 2;
    setGrid(Math.pow(10, Math.floor(Math.log10(radius)))); controls.update(); requestRender();
  }
  let press = null;
  renderer.domElement.addEventListener("pointerdown", e => {
    if (e.button !== 0 || busy) return;
    press = { x: e.clientX, y: e.clientY, id: e.pointerId, gizmo: transform.dragging || transform.axis !== null };
    viewport.focus({ preventScroll: true });
  });
  renderer.domElement.addEventListener("pointercancel", () => { press = null; });
  renderer.domElement.addEventListener("pointerup", e => {
    const start = press; press = null;
    if (!start || e.button !== 0 || e.pointerId !== start.id || busy || start.gizmo || Math.hypot(e.clientX - start.x, e.clientY - start.y) > 5) return;
    const rect = renderer.domElement.getBoundingClientRect();
    pointer.set((e.clientX - rect.left) / rect.width * 2 - 1, -(e.clientY - rect.top) / rect.height * 2 + 1);
    scene.updateMatrixWorld(true); raycaster.setFromCamera(pointer, camera);
    const hits = raycaster.intersectObjects(models.map(model => model.root), true);
    let target = hits.find(hit => hit.object.isMesh)?.object;
    while (target && !target.userData.mapEditorID) target = target.parent;
    select(models.find(model => model.id === target?.userData.mapEditorID) || null);
  });
  positionInputs.forEach((input, index) => input.addEventListener("change", () => {
    if (!selected || busy) return;
    if (input.value === "" || !Number.isFinite(input.valueAsNumber)) {
      notice("位置には有限の数値を入力してください。", "error"); syncPosition(); return;
    }
    selected.root.position[axes[index]] = input.valueAsNumber;
    syncPosition(); requestRender();
  }));
  $("frame-all").addEventListener("click", () => frame(models));
  $("frame-selected").addEventListener("click", () => { if (selected) frame([selected]); });
  function dispose(root) {
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
  $("remove-selected").addEventListener("click", () => {
    if (!selected || busy) return;
    const model = selected; select(null); scene.remove(model.root); dispose(model.root);
    models.splice(models.indexOf(model), 1); refreshList(); requestRender();
    notice(`${model.name} を画面から削除しました。`);
  });
  document.addEventListener("keydown", e => {
    if (e.key === "Escape") { document.querySelector(".help").open = false; return; }
    if (busy || e.ctrlKey || e.altKey || e.metaKey || /^(INPUT|TEXTAREA|SELECT|BUTTON|SUMMARY)$/.test(document.activeElement.tagName)) return;
    if (e.key.toLowerCase() === "f" && selected) { e.preventDefault(); frame([selected]); }
  });

  const basename = name => name.replace(/\\/g, "/").split("/").pop().toLowerCase();
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
    const root = new T.Group(); root.add(imported); root.userData.mapEditorID = ++sequence;
    return { id: sequence, name: file.name, root, meshCount, vertices, missing: [...missing] };
  }
  async function importFiles(files) {
    if (busy) return;
    const all = Array.from(files), fbxs = all.filter(file => /\.fbx$/i.test(file.name));
    if (!fbxs.length) { notice("FBXファイルを選択してください。外部テクスチャはFBXと一緒に選択できます。", "warning"); return; }
    const textures = new Map();
    for (const file of all.filter(file => !/\.fbx$/i.test(file.name))) {
      const name = basename(file.name); if (!textures.has(name)) textures.set(name, []); textures.get(name).push(file);
    }
    busy = true; $("open-files").disabled = true; $("frame-all").disabled = true;
    $("object-list").inert = true; $("selection").inert = true; controls.enabled = false; transform.enabled = false;
    const added = [], errors = [], warnings = [];
    try {
      for (const [index, file] of fbxs.entries()) {
        notice(`${file.name} を読み込み中… (${index + 1}/${fbxs.length})`);
        // Yield so the progress message paints before synchronous FBX parsing.
        await new Promise(resolve => setTimeout(resolve, 0));
        try {
          const model = await loadFBX(file, textures); models.push(model); added.push(model); scene.add(model.root);
          if (model.missing.length) warnings.push(`${file.name}：省略した画像 ${model.missing.join("、")}`);
        } catch (error) { errors.push(`${file.name}：${error.message || String(error)}`); }
      }
      if (added.length) { select(added[added.length - 1]); frame(models); }
      const result = `${added.length} 件のFBXを追加しました。`;
      notice([result, ...errors, ...warnings].join("\n"), errors.length ? "error" : warnings.length ? "warning" : "");
    } finally {
      busy = false; $("open-files").disabled = false; $("object-list").inert = false; $("selection").inert = false;
      controls.enabled = true; transform.enabled = true; refreshList(); requestRender();
    }
  }
  $("open-files").addEventListener("click", () => $("file-input").click());
  $("file-input").addEventListener("change", e => { const files = Array.from(e.target.files); e.target.value = ""; importFiles(files); });
  let dragDepth = 0;
  document.addEventListener("dragenter", e => { if (Array.from(e.dataTransfer.types).includes("Files")) { e.preventDefault(); dragDepth++; document.body.classList.add("dragging"); } });
  document.addEventListener("dragover", e => { if (Array.from(e.dataTransfer.types).includes("Files")) e.preventDefault(); });
  document.addEventListener("dragleave", () => { if (--dragDepth <= 0) { dragDepth = 0; document.body.classList.remove("dragging"); } });
  document.addEventListener("drop", e => { e.preventDefault(); dragDepth = 0; document.body.classList.remove("dragging"); if (e.dataTransfer.files.length) importFiles(e.dataTransfer.files); });
  window.addEventListener("beforeunload", e => { if (models.length || busy) { e.preventDefault(); e.returnValue = ""; } });
  $("open-files").disabled = false;
  notice("FBXを開いてください。外部テクスチャがある場合は、画像も一緒に選択できます。");
  requestRender();
})();
