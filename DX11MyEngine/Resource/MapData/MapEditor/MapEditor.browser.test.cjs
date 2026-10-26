/* Optional integration checks: requires Playwright and an installed Edge/Chrome. */
"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const http = require("node:http");
const os = require("node:os");
const { chromium } = require("playwright");
const root = __dirname;
const artifactDir = process.env.MAP_EDITOR_TEST_OUTPUT || os.tmpdir();
const server = http.createServer((req, res) => {
  const file = path.resolve(root, "." + (req.url === "/" ? "/MapEditor.html" : req.url.split("?")[0]));
  if (!file.startsWith(root + path.sep)) { res.writeHead(403); res.end(); return; }
  try {
    let source = fs.readFileSync(file);
    if (file.endsWith("MapEditor.js")) source = source.toString().replace(/\r\n/g, "\n").replace("  requestRender();\n})();", "  window.__check = { scene, renderer, appearance, transform, models, selection, history, pivot, byId, outlines, hiddenIds, lockedIds, terrainIds, terrainReferences, geometryTools, get scatter() { return scatter; }, get focusAnimation() { return focusAnimation; }, get spaceMode() { return spaceMode; }, get selected() { return selected; }, get camera() { return camera; }, get controls() { return controls; } };\n  requestRender();\n})();");
    res.setHeader("Content-Type", file.endsWith(".html") ? "text/html; charset=utf-8" : file.endsWith(".css") ? "text/css" : "text/javascript");
    res.end(source);
  } catch { res.writeHead(404); res.end(); }
});
const close = (a, b, label = "number") => assert.ok(Math.abs(a - b) < 1e-6, `${label}: ${a} != ${b}`);

(async () => {
  await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
  const browser = await chromium.launch({ executablePath: process.env.MAP_EDITOR_BROWSER || "C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe", headless: true });
  try {
    const page = await browser.newPage({ viewport: { width: 1440, height: 1000 }, acceptDownloads: true });
    await page.addInitScript(() => { window.showSaveFilePicker = undefined; });
    const errors = []; page.on("pageerror", error => errors.push(error.message));
    await page.goto(`http://127.0.0.1:${server.address().port}/`);
    await page.waitForFunction(() => !!window.__check);
    const state = () => page.evaluate(() => __check.models.map(model => ({ id: model.id, assetID: model.assetID, p: model.root.position.toArray(), r: model.root.rotation.toArray(), s: model.root.scale.toArray() })));
    const selected = () => page.evaluate(() => [...__check.selection]);
    const cursor = () => page.evaluate(() => __check.history.cursor);
    const ids = () => page.evaluate(() => __check.models.map(model => model.id));
    async function key(key) { await page.locator("#viewport").focus(); await page.keyboard.press(key); if (key.toLowerCase() === "f") await page.waitForFunction(() => !__check.focusAnimation); }
    async function enter(id, value) { await page.locator(id).fill(String(value)); await page.locator(id).press("Tab"); }
    async function downloadMap(name = null) {
      await page.locator("#save-map").click(); await page.locator("#save-dialog").waitFor({ state: "visible" });
      if (name !== null) await page.locator("#export-name").fill(name);
      const event = page.waitForEvent("download"); await page.locator("#save-download").click(); return event;
    }
    async function choose(id, modifiers = []) {
      try { await page.locator(`[data-object-id="${id}"]`).click({ modifiers, timeout: 5000 }); }
      catch (error) { console.error(await page.evaluate(() => ({ query: document.querySelector("#object-search").value, ids: __check.models.map(model => model.id), rows: [...document.querySelectorAll("[data-object-id]")].map(row => row.dataset.objectId), active: document.activeElement.id, notice: document.querySelector("#notice").textContent }))); throw error; }
    }
    async function drag(selector, dx, shift = false, cancel = false) {
      await page.locator("#viewport").focus(); await page.locator(selector).scrollIntoViewIfNeeded();
      const rect = await page.locator(selector).boundingBox(), x = rect.x + rect.width / 2, y = rect.y + rect.height / 2;
      await page.mouse.move(x, y); await page.mouse.down(); if (shift) await page.keyboard.down("Shift");
      await page.mouse.move(x + dx, y, { steps: 20 }); if (cancel) await page.keyboard.press("Escape");
      await page.mouse.up(); if (shift) await page.keyboard.up("Shift");
    }
    let mapNumber = 0;
    const object = (x, y = 0, z = 0) => ({ assetID: "Rock_01", position: [x, y, z], rotation: [0, 0, 0], scale: [1, 1, 1] });
    async function loadMap(objects, extra = {}) {
      const map = { mapName: "BrowserTest" + ++mapNumber, groundAssetTag: "MountainsGroundA", objects, ...extra };
      await page.locator("#file-input").setInputFiles({ name: "Map.json", mimeType: "application/json", buffer: Buffer.from(JSON.stringify(map)) });
      await page.waitForFunction(name => document.querySelector("#notice").textContent.includes(name) && !document.querySelector("#open-files").disabled, map.mapName);
      assert.equal(await cursor(), 0); assert.equal(await page.locator("#redo").isDisabled(), true);
      assert.equal(await page.locator("#map-name").inputValue(), map.mapName); assert.equal(await page.locator("#ground-asset-tag").inputValue(), map.groundAssetTag);
      return map;
    }
    async function chooseAll() { const list = await ids(); for (let i = 0; i < list.length; i++) await choose(list[i], i ? ["Shift"] : []); }
    async function undoRedo(before, after, redoKey = "Control+y") {
      await key("Control+z"); assert.deepEqual(await state(), before);
      await key(redoKey); assert.deepEqual(await state(), after);
    }
    await page.locator("#file-input").setInputFiles(path.resolve(root, "../../Model/MapObject/Rock/Rock_01.fbx"));
    await page.waitForFunction(() => __check.models.length === 1 && !document.querySelector("#open-files").disabled);
    const placement = await state();
    await key("Control+z"); assert.deepEqual(await state(), []);
    await key("Control+y"); assert.deepEqual(await state(), placement);
    for (const [field, value] of [["#position-x", 13.25], ["#rotation-y", 93.5], ["#scale-z", 2.5]]) {
      const before = await state(), oldCursor = await cursor();
      await enter(field, value); const after = await state();
      assert.equal(await cursor(), oldCursor + 1); await undoRedo(before, after, "Control+Shift+z");
    }
    let before = await state(), oldCursor = await cursor();
    await drag("#position-x", 80); let after = await state();
    assert.equal(await cursor(), oldCursor + 1, "20 pointer moves produce one command"); await undoRedo(before, after);
    before = await state(); oldCursor = await cursor();
    await drag("#position-x", 80, false, true); assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await key("Control+z"); await enter("#position-y", 7); assert.equal(await page.locator("#redo").isDisabled(), true);
    console.log("PASS single transforms, placement, drag coalescing, cancellation, redo branching");

    // Observe materials used by a real WebGL render; imported materials must be
    // restored immediately afterwards for picking, cloning and terrain queries.
    await page.evaluate(() => {
      const renderer = __check.renderer, render = renderer.render;
      __check.originalMaterials = [];
      __check.models[0].root.traverse(mesh => { if (mesh.isMesh) __check.originalMaterials.push([mesh, mesh.material]); });
      renderer.render = function(scene, camera) {
        __check.materialFrame = __check.models.map(model => {
          const materials = []; model.root.traverse(mesh => { if (mesh.isMesh) for (const material of Array.isArray(mesh.material) ? mesh.material : [mesh.material]) materials.push({ color: material.color?.getHexString(), wireframe: !!material.wireframe, texture: !!material.map }); });
          return { id: model.id, materials };
        });
        return render.call(this, scene, camera);
      };
    });
    async function colorInput(selector, value) { await page.locator(selector).evaluate((input, color) => { input.value = color; input.dispatchEvent(new Event("input", { bubbles: true })); }, value); }
    async function paint() { await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve)))); }
    before = await state(); oldCursor = await cursor();
    await page.locator("#display-mode").selectOption("solid"); await colorInput("#solid-color", "#88aa44"); await paint();
    assert.ok(await page.evaluate(() => __check.materialFrame[0].materials.every(material => material.color === "88aa44" && !material.wireframe && !material.texture)));
    await page.locator("#display-mode").selectOption("wireframe"); await paint();
    assert.ok(await page.evaluate(() => __check.materialFrame[0].materials.every(material => material.wireframe)));
    assert.ok(await page.evaluate(() => __check.originalMaterials.every(([mesh, material]) => mesh.material === material)));
    await page.locator("#display-mode").selectOption("material"); await colorInput("#selected-color", "#e07030"); await paint();
    assert.equal(await page.locator("#display-mode").inputValue(), "solid");
    assert.ok(await page.evaluate(() => __check.materialFrame[0].materials.every(material => material.color === "e07030")));
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await key("Control+d"); await colorInput("#selected-color", "#40a0f0"); await paint();
    assert.ok(await page.evaluate(() => __check.materialFrame[0].materials.every(material => material.color === "e07030") && __check.materialFrame[1].materials.every(material => material.color === "40a0f0")));
    await key("Control+z"); await key("Control+y"); await paint();
    assert.ok(await page.evaluate(() => __check.appearance.colorOf(__check.selected.root) === "#40a0f0"));
    await key("Control+z"); await page.locator("#reset-selected-color").click(); await paint();
    assert.ok(await page.evaluate(() => __check.materialFrame[0].materials.every(material => material.color === "88aa44")));
    await page.locator("#display-mode").selectOption("material"); await paint();
    assert.ok(await page.evaluate(() => __check.originalMaterials.every(([mesh, material]) => mesh.material === material)));
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    console.log("PASS real WebGL material/solid/wire modes, global/per-object colors, shared originals restored, duplication/history and no transform/history edits from colors");

    let downloadCount = 0; const countDownload = () => downloadCount++; page.on("download", countDownload);
    await enter("#map-name", "山岳テスト"); await enter("#ground-asset-tag", "MountainsGroundA");
    await page.locator("#save-map").click(); assert.equal(await page.locator("#export-name").inputValue(), "山岳テスト.json");
    await page.locator("#save-cancel").click(); assert.equal(downloadCount, 0);
    await page.locator("#save-map").click(); await page.keyboard.press("Escape"); assert.equal(await page.locator("#save-dialog").isVisible(), false); assert.equal(downloadCount, 0);
    await enter("#map-name", ""); await page.locator("#save-map").click(); assert.equal(await page.locator("#save-dialog").isVisible(), false);
    assert.ok((await page.locator("#notice").textContent()).includes("マップ名を入力")); await enter("#map-name", "山岳テスト");
    const namedDownload = await downloadMap("MAP_CUSTOM_A"); assert.equal(namedDownload.suggestedFilename(), "MAP_CUSTOM_A.json");
    const namedJSON = JSON.parse(fs.readFileSync(await namedDownload.path(), "utf8"));
    assert.equal(namedJSON.mapName, "山岳テスト"); assert.equal(namedJSON.groundAssetTag, "MountainsGroundA");
    assert.deepEqual(Object.keys(namedJSON).sort(), ["groundAssetTag", "mapName", "objects"]);
    namedJSON.objects.forEach(object => assert.deepEqual(Object.keys(object), ["assetID", "position", "rotation", "scale"]));
    await page.evaluate(() => {
      window.showSaveFilePicker = async options => {
        __check.nativeSave = { options, busyAtPicker: document.querySelector("#open-files").disabled, metadataDisabled: document.querySelector("#map-name").disabled };
        return { name: "chosen-map.json", async createWritable() { return { async write(value) { __check.nativeSave.text = value; }, async close() { __check.nativeSave.closed = true; }, async abort() { __check.nativeSave.aborted = true; } }; } };
      };
    });
    await page.locator("#save-map").click(); await page.waitForFunction(() => __check.nativeSave?.closed && !document.querySelector("#save-map").disabled);
    const nativeSave = await page.evaluate(() => __check.nativeSave);
    assert.equal(nativeSave.options.suggestedName, "MAP_CUSTOM_A.json"); assert.ok(nativeSave.busyAtPicker && nativeSave.metadataDisabled);
    assert.deepEqual(JSON.parse(nativeSave.text), namedJSON); assert.equal(downloadCount, 1);
    await page.evaluate(() => { window.showSaveFilePicker = async () => { throw new DOMException("cancel", "AbortError"); }; });
    await page.locator("#save-map").click(); await page.waitForFunction(() => !document.querySelector("#save-map").disabled);
    assert.equal(await page.locator("#save-dialog").isVisible(), false); assert.equal(downloadCount, 1);
    await page.evaluate(() => { window.showSaveFilePicker = async () => ({ name: "fail.json", async createWritable() { return { async write() { throw new Error("disk full"); }, async close() { __check.badClose = true; }, async abort() { __check.failedWriteAborted = true; } }; } }); });
    await page.locator("#save-map").click(); await page.waitForFunction(() => __check.failedWriteAborted && !document.querySelector("#save-map").disabled);
    assert.ok((await page.locator("#notice").textContent()).includes("disk full")); assert.equal(await page.evaluate(() => !!__check.badClose), false); assert.equal(downloadCount, 1);
    await page.evaluate(() => { window.showSaveFilePicker = async () => { throw new DOMException("blocked", "SecurityError"); }; });
    await page.locator("#save-map").click(); await page.locator("#save-dialog").waitFor({ state: "visible" });
    assert.equal(await page.locator("#export-name").inputValue(), "chosen-map.json"); await page.locator("#save-cancel").click();
    await page.evaluate(() => { window.showSaveFilePicker = undefined; }); page.off("download", countDownload);
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await colorInput("#selected-color", "#d01040");
    await page.locator("#file-input").setInputFiles({ name: "RenamedMap.json", mimeType: "application/json", buffer: Buffer.from(JSON.stringify(namedJSON)) });
    await page.waitForFunction(() => __check.history.cursor === 0 && !document.querySelector("#open-files").disabled);
    assert.equal(await page.locator("#map-name").inputValue(), namedJSON.mapName); assert.equal(await page.locator("#ground-asset-tag").inputValue(), namedJSON.groundAssetTag);
    assert.ok(await page.evaluate(() => __check.models.every(model => !__check.appearance.hasColor(model.root))));
    await page.locator("#display-mode").selectOption("material");
    console.log("PASS editable map metadata, named download dialog/cancel/validation, native picker-write contract/cancel/errors, fallback without surprise downloads, schema and metadata reload; native OS picker mocked");

    await loadMap([object(-25), object(0), object(25), object(0, 0, -25)]);
    let marqueeIDs = await ids(); await key("3");
    async function marquee(targets, modifiers = [], cancelled = false, reverse = false) {
      const points = await page.evaluate(([targets, reverse]) => {
        const { scene, camera, transform } = __check; scene.updateMatrixWorld(true); camera.updateMatrixWorld(true);
        const rect = document.querySelector("canvas").getBoundingClientRect(), projected = [];
        for (const id of targets) {
          const box = new MapEditorThree.Box3().setFromObject(__check.byId.get(id).root);
          for (const x of [box.min.x, box.max.x]) for (const y of [box.min.y, box.max.y]) for (const z of [box.min.z, box.max.z]) {
            const p = new MapEditorThree.Vector3(x, y, z).project(camera); projected.push({ x: rect.x + (p.x + 1) * rect.width / 2, y: rect.y + (1 - p.y) * rect.height / 2 });
          }
        }
        const left = Math.min(...projected.map(p => p.x)) - 12, right = Math.max(...projected.map(p => p.x)) + 12;
        const top = Math.min(...projected.map(p => p.y)) - 12, bottom = Math.max(...projected.map(p => p.y)) + 12;
        const corners = [{ x: left, y: top, ex: right, ey: bottom }, { x: right, y: bottom, ex: left, ey: top }, { x: right, y: top, ex: left, ey: bottom }, { x: left, y: bottom, ex: right, ey: top }];
        if (reverse) corners.reverse();
        const ray = new MapEditorThree.Raycaster();
        return corners.find(p => {
          if (!transform.object) return true;
          ray.setFromCamera(new MapEditorThree.Vector2((p.x - rect.x) / rect.width * 2 - 1, -(p.y - rect.y) / rect.height * 2 + 1), camera);
          return !ray.intersectObject(transform._gizmo.picker[transform.mode], true).some(hit => hit.object.visible);
        });
      }, [targets, reverse]);
      assert.ok(points, "marquee starts away from gizmo handles");
      for (const modifier of modifiers) await page.keyboard.down(modifier);
      await page.mouse.move(points.x, points.y); await page.mouse.down(); await page.mouse.move(points.ex, points.ey, { steps: 8 });
      assert.equal(await page.locator("#selection-rectangle").isVisible(), true);
      assert.equal(await page.evaluate(() => __check.controls.enabled), false);
      if (cancelled) await page.keyboard.press("Escape");
      await page.mouse.up(); for (const modifier of modifiers) await page.keyboard.up(modifier);
      assert.equal(await page.locator("#selection-rectangle").isVisible(), false); assert.equal(await page.evaluate(() => __check.controls.enabled), true);
    }
    before = await state(); oldCursor = await cursor();
    await marquee(marqueeIDs.slice(0, 2)); assert.deepEqual(await selected(), marqueeIDs.slice(0, 2));
    await marquee([marqueeIDs[2]], ["Shift"], false, true); assert.deepEqual(await selected(), marqueeIDs.slice(0, 3));
    await marquee([marqueeIDs[1]], ["Control"]); assert.deepEqual(await selected(), [marqueeIDs[0], marqueeIDs[2]]);
    const beforeCancel = await selected(); await marquee([marqueeIDs[1], marqueeIDs[3]], [], true); assert.deepEqual(await selected(), beforeCancel);
    for (const view of ["1", "2", "3", "4"]) {
      await key(view); await page.locator("#frame-all").click(); await page.waitForFunction(() => !__check.focusAnimation);
      // From the side the X-separated objects overlap, so use a full-view box.
      if (view === "2") continue;
      await marquee([marqueeIDs[0]], [], false, true); assert.deepEqual(await selected(), [marqueeIDs[0]]);
    }
    await page.locator(`[data-visibility-id="${marqueeIDs[2]}"]`).click(); await page.locator(`[data-lock-id="${marqueeIDs[3]}"]`).click();
    const marqueeViewport = await page.locator("canvas").boundingBox();
    await page.mouse.move(marqueeViewport.x + 4, marqueeViewport.y + 45); await page.mouse.down();
    await page.mouse.move(marqueeViewport.x + marqueeViewport.width + 25, marqueeViewport.y + marqueeViewport.height + 25, { steps: 10 }); await page.mouse.up();
    assert.deepEqual(await selected(), marqueeIDs.slice(0, 2), "viewport-external release keeps capture and excludes hidden/locked objects");
    assert.equal(await page.locator("#selection-rectangle").isVisible(), false);
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await page.locator("#show-all").click(); await page.locator("#unlock-all").click();
    console.log("PASS real left-drag marquee, shift-add/ctrl-toggle, reverse drags, Escape, perspective/orthographic views, hidden/locked exclusion, outside release and no transform/history edits");

    await loadMap([object(-40), object(-20), object(0), object(20), object(40)]);
    let colorIDs = await ids(); await choose(colorIDs[3]); await enter("#asset-id", "Tree_01"); await choose(colorIDs[4]); await enter("#asset-id", "ROCK_01");
    await choose(colorIDs[1]); await colorInput("#selected-color", "#ff8800");
    await page.locator(`[data-lock-id="${colorIDs[1]}"]`).click(); await page.locator(`[data-visibility-id="${colorIDs[2]}"]`).click(); await choose(colorIDs[0]);
    before = await state(); oldCursor = await cursor();
    await page.locator("#color-by-asset").check(); await colorInput("#selected-color", "#dd2020"); await paint();
    let objectColors = await page.evaluate(() => __check.models.map(model => __check.appearance.colorOf(model.root)));
    assert.deepEqual(objectColors.slice(0, 3), ["#dd2020", "#dd2020", "#dd2020"]);
    assert.notEqual(objectColors[3], "#dd2020"); assert.notEqual(objectColors[4], "#dd2020");
    assert.ok(await page.evaluate(ids => __check.materialFrame.filter(frame => ids.includes(frame.id)).every(frame => frame.materials.every(material => material.color === "dd2020")), colorIDs.slice(0, 2)));
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await page.locator("#color-by-asset").uncheck(); await colorInput("#selected-color", "#f09020"); await key("Delete");
    await choose(colorIDs[1]); await page.locator("#color-by-asset").check(); await colorInput("#selected-color", "#2060dd"); await key("Control+z");
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    assert.equal(await page.evaluate(id => __check.appearance.colorOf(__check.byId.get(id).root), colorIDs[0]), "#2060dd", "asset-wide changes also clear stale personal colors in history");
    await choose(colorIDs[0]); await key("Control+d");
    assert.equal(await page.evaluate(() => __check.appearance.colorOf(__check.selected.root)), "#2060dd"); await key("Control+z");
    await choose(colorIDs[3]); await enter("#asset-id", "Rock_01");
    assert.equal(await page.evaluate(() => __check.appearance.colorOf(__check.selected.root)), "#2060dd"); await key("Control+z");
    assert.notEqual(await page.evaluate(() => __check.appearance.colorOf(__check.selected.root)), "#2060dd");
    await choose(colorIDs[0]); await choose(colorIDs[3], ["Shift"]); await colorInput("#selected-color", "#20bb50");
    objectColors = await page.evaluate(() => __check.models.map(model => __check.appearance.colorOf(model.root)));
    assert.deepEqual(objectColors.slice(0, 4), ["#20bb50", "#20bb50", "#20bb50", "#20bb50"]); assert.notEqual(objectColors[4], "#20bb50");
    const colorMap = await page.evaluate(() => MapEditorCore.serializeMap({ mapName: "ColorReload", groundAssetTag: "" }, __check.models));
    colorMap.objects.forEach(object => assert.deepEqual(Object.keys(object), ["assetID", "position", "rotation", "scale"]));
    await page.locator("#file-input").setInputFiles({ name: "ColorReload.json", mimeType: "application/json", buffer: Buffer.from(JSON.stringify(colorMap)) });
    await page.waitForFunction(() => document.querySelector("#notice").textContent.includes("ColorReload") && !document.querySelector("#open-files").disabled);
    assert.ok(await page.evaluate(() => __check.models.filter(model => model.assetID !== "ROCK_01").every(model => __check.appearance.colorOf(model.root) === "#20bb50")), "new instances inherit the session's asset palette");
    colorIDs = await ids(); await choose(colorIDs[0]); await choose(colorIDs[3], ["Shift"]); await page.locator("#reset-selected-color").click();
    assert.ok(await page.evaluate(() => __check.models.every(model => __check.appearance.colorOf(model.root) === document.querySelector("#solid-color").value)));
    await page.locator("#color-by-asset").uncheck(); await page.locator("#display-mode").selectOption("material");
    console.log("PASS exact-asset batch colors, independent assets/case, hidden/locked targets, new instances, retained history, asset-ID undo, multi-kind batches, reset and unchanged game JSON");

    await loadMap([object(-10), object(10)]); let list = await ids();
    const viewport = await page.locator("#viewport").boundingBox();
    await page.mouse.click(viewport.x + 18, viewport.y + 65);
    assert.deepEqual(await selected(), []);
    async function clickModel(id, modifiers = []) {
      const point = await page.evaluate(id => {
        __check.scene.updateMatrixWorld(true); __check.camera.updateMatrixWorld(true);
        const center = new MapEditorThree.Box3().setFromObject(__check.byId.get(id).root).getCenter(new MapEditorThree.Vector3()).project(__check.camera);
        const rect = document.querySelector("canvas").getBoundingClientRect();
        return { x: rect.x + (center.x + 1) * rect.width / 2, y: rect.y + (1 - center.y) * rect.height / 2 };
      }, id);
      for (const modifier of modifiers) await page.keyboard.down(modifier);
      await page.mouse.click(point.x, point.y);
      for (const modifier of modifiers) await page.keyboard.up(modifier);
    }
    await clickModel(list[0]); assert.deepEqual(await selected(), [list[0]]);
    await clickModel(list[1], ["Control"]); assert.equal((await selected()).length, 2);
    await clickModel(list[0], ["Control"]); assert.deepEqual(await selected(), [list[1]]);
    await clickModel(list[0], ["Shift"]); assert.equal((await selected()).length, 2);
    await page.mouse.click(viewport.x + 18, viewport.y + 65); assert.deepEqual(await selected(), []);
    await choose(list[0]); await choose(list[1], ["Control"]); assert.equal((await selected()).length, 2);
    await choose(list[0], ["Control"]); assert.deepEqual(await selected(), [list[1]]);
    await choose(list[0], ["Shift"]); assert.equal((await selected()).length, 2);
    assert.equal(await page.evaluate(() => __check.outlines.size), 2);
    assert.ok(await page.evaluate(() => __check.transform.object === __check.pivot));
    before = await state(); const oldX = Number(await page.locator("#position-x").inputValue());
    await enter("#position-x", oldX + 5); after = await state();
    after.forEach((entry, i) => close(entry.p[0] - before[i].p[0], 5)); await undoRedo(before, after);
    const center = await page.evaluate(() => __check.pivot.position.toArray());
    before = await state(); oldCursor = await cursor(); await enter("#rotation-y", 90); after = await state();
    after.forEach((entry, i) => { close(entry.p[0], center[0] + before[i].p[2] - center[2]); close(entry.p[2], center[2] - (before[i].p[0] - center[0])); });
    assert.equal(await cursor(), oldCursor + 1); await undoRedo(before, after);
    await loadMap([object(-10), object(10)]); await chooseAll();
    before = await state(); const cx = await page.evaluate(() => __check.pivot.position.x);
    await enter("#scale-x", 2); after = await state();
    after.forEach((entry, i) => { close(entry.p[0], cx + (before[i].p[0] - cx) * 2); close(entry.s[0], 2); });
    await undoRedo(before, after);

    before = await state(); const originalIDs = await selected(); await key("Control+d"); after = await state();
    assert.equal(after.length, 4); assert.equal(new Set(after.map(entry => entry.id)).size, 4);
    assert.ok((await selected()).every(id => !originalIDs.includes(id)));
    assert.equal(await page.evaluate(() => __check.transform.mode), "translate");
    after.slice(2).forEach((entry, i) => { assert.equal(entry.assetID, before[i].assetID); assert.deepEqual(entry.p, before[i].p); assert.deepEqual(entry.r, before[i].r); assert.deepEqual(entry.s, before[i].s); });
    assert.ok(await page.evaluate(() => {
      const meshes = __check.models.map(model => { let mesh; model.root.traverse(node => { if (!mesh && node.isMesh) mesh = node; }); return mesh; });
      return meshes[0].geometry === meshes[2].geometry && meshes[0].material === meshes[2].material;
    }), "duplicates share geometry and materials");
    await undoRedo(before, after); const copies = await selected();
    before = await state(); await key("Delete"); after = await state(); assert.equal(after.length, 2);
    await undoRedo(before, after); await key("Control+z"); assert.deepEqual(await selected(), copies);
    // Context menu operates on the selected group and remains distinct from orbit dragging.
    await page.locator(`[data-object-id="${copies[0]}"]`).click({ button: "right" });
    assert.equal(await page.locator("#context-menu").isVisible(), true);
    await page.locator("#context-duplicate").click(); assert.equal((await state()).length, 6);
    await page.locator("#undo").click(); assert.equal((await state()).length, 4);
    await page.locator("#redo").click(); assert.equal((await state()).length, 6);
    console.log("PASS multi-selection, common pivot, grouped TRS, duplicates, delete, stable IDs, context menu, toolbar history");

    await loadMap([object(0.3), object(20.3)]); await chooseAll();
    before = await state(); await page.locator("#position-snap").check(); await enter("#position-step", 2);
    assert.deepEqual(await state(), before, "enabling snap must not move objects");
    const relative = before[1].p.map((v, i) => v - before[0].p[i]);
    oldCursor = await cursor(); await drag("#position-x", 63); after = await state();
    close(Number(await page.locator("#position-x").inputValue()) % 2, 0, "group pivot snapped");
    relative.forEach((v, i) => close(after[1].p[i] - after[0].p[i], v, "group offsets preserved"));
    assert.equal(await cursor(), oldCursor + 1); await undoRedo(before, after);
    await page.locator("#position-snap").uncheck(); list = await ids(); await choose(list[0]); await enter("#position-x", 0.3);
    await page.locator("#position-snap").check(); await enter("#position-step", 1);
    before = await state(); await drag("#position-x", 4, true); assert.deepEqual(await state(), before, "off-grid grab does not jump");
    for (const axis of ["x", "y", "z"]) { await enter("#position-" + axis, 7.2); close((await state())[0].p[["x", "y", "z"].indexOf(axis)], 7); }
    await enter("#position-step", 0); assert.equal(await page.locator("#position-step").inputValue(), "1");
    await page.locator("#position-snap").uncheck(); await enter("#position-x", 7.2); close((await state())[0].p[0], 7.2);
    await page.locator("#rotation-snap").check(); await enter("#rotation-y", 23); close((await state())[0].r[1], Math.PI / 6);
    await enter("#rotation-step", 30); await enter("#rotation-z", 46); close((await state())[0].r[2], Math.PI / 3);
    await page.locator("#rotation-snap").uncheck(); await enter("#rotation-y", 23); close((await state())[0].r[1], 23 * Math.PI / 180);
    console.log("PASS snap toggles, intervals, XYZ, rotation, no initial jump and multi-object relative positions");

    // Actual gizmo pointer drags, including live snapping and history coalescing.
    await loadMap([object(-10), object(10)]); await chooseAll(); await page.locator("#position-snap").check(); await page.locator("#rotation-snap").check();
    await enter("#rotation-step", 15);
    async function gizmoPoint(mode, axis) {
      await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
      return page.evaluate(([mode, axis]) => {
        const { camera, transform, scene } = __check; scene.updateMatrixWorld(true); camera.updateMatrixWorld(true);
        const rect = document.querySelector("canvas").getBoundingClientRect(), origin = transform.object.position.clone().project(camera);
        const cx = rect.x + (origin.x + 1) * rect.width / 2, cy = rect.y + (1 - origin.y) * rect.height / 2;
        const ray = new MapEditorThree.Raycaster();
        for (let radius = 50; radius < 180; radius += 3) for (let angle = 0.3; angle < Math.PI * 2; angle += 0.1) {
          const x = cx + Math.cos(angle) * radius, y = cy + Math.sin(angle) * radius;
          ray.setFromCamera(new MapEditorThree.Vector2((x - rect.x) / rect.width * 2 - 1, -(y - rect.y) / rect.height * 2 + 1), camera);
          if (ray.intersectObject(transform._gizmo.picker[mode], true).find(hit => hit.object.visible)?.object.name === axis) return { x, y };
        }
        return null;
      }, [mode, axis]);
    }
    async function gizmoDrag(shortcut, mode, axis, dx, dy, local = false) {
      await key("3"); await key(shortcut); await key("f");
      const point = await gizmoPoint(mode, axis);
      assert.ok(point, mode + " handle found");
      const prior = await state(), count = await cursor();
      const startX = await page.evaluate(() => __check.pivot.position.x);
      await page.mouse.move(point.x, point.y); await page.mouse.down();
      assert.ok(await page.evaluate(() => __check.transform.dragging));
      assert.equal(await page.locator("#selection-rectangle").isVisible(), false, "gizmo drag has priority over marquee");
      for (let i = 1; i <= 8; i++) {
        await page.mouse.move(point.x + dx * i / 8, point.y + dy * i / 8);
        assert.equal(await cursor(), count, "history stays unchanged during drag");
        if (mode === "translate" && !local) { const x = await page.evaluate(() => __check.pivot.position.x); if (x !== startX) close(x % 1, 0); }
        if (mode === "rotate") { const steps = await page.evaluate(() => __check.transform.rotationAngle * 180 / Math.PI / 15); close(steps, Math.round(steps)); }
      }
      await page.mouse.up(); const later = await state(); assert.notDeepEqual(later, prior);
      assert.equal(await cursor(), count + 1); await undoRedo(prior, later);
      return { prior, later };
    }
    await gizmoDrag("w", "translate", "X", 40, 0);
    await gizmoDrag("e", "rotate", "Y", 40, 30);
    await gizmoDrag("r", "scale", "X", 40, 0);
    console.log("PASS actual grouped gizmo translation, rotation, scaling, live snapping, exact undo/redo");

    // Save through the UI, reload in the same session and a completely fresh page.
    const savedState = JSON.parse(JSON.stringify((await state()).map(({ id, ...rest }) => rest)));
    const download = await downloadMap();
    const json = JSON.parse(fs.readFileSync(await download.path(), "utf8"));
    for (const entry of json.objects) assert.deepEqual(Object.keys(entry), ["assetID", "position", "rotation", "scale"]);
    json.objects.forEach((entry, index) => {
      const rendered = savedState[index];
      assert.deepEqual(entry.position, [rendered.p[0], rendered.p[1], -rendered.p[2]].map(value => value || 0), "downloaded positions use the game's left-handed coordinates");
      assert.deepEqual(entry.rotation, [-rendered.r[0], -rendered.r[1], rendered.r[2]].map(value => value || 0), "downloaded YXZ rotations reflect X/Y");
      assert.equal(rendered.r[3], "YXZ");
      assert.deepEqual(entry.scale, rendered.s);
    });
    assert.ok(json.objects.some(entry => entry.position[2] > 0) && json.objects.some(entry => entry.position[2] < 0), "test both signs of game Z");
    assert.ok(!JSON.stringify(json).includes("selection"));
    await page.locator("#file-input").setInputFiles({ name: "Saved.json", mimeType: "application/json", buffer: Buffer.from(JSON.stringify(json)) });
    await page.waitForFunction(() => !document.querySelector("#open-files").disabled && __check.history.commands.length === 0);
    assert.deepEqual((await state()).map(({ id, ...rest }) => rest), savedState);
    assert.equal(await page.locator("#undo").isDisabled(), true);
    await chooseAll(); await key("Control+d"); before = await state(); const oldCommands = await cursor();
    await page.locator("#file-input").setInputFiles({ name: "Invalid.json", mimeType: "application/json", buffer: Buffer.from(JSON.stringify({ ...json, objects: [{ ...object(0), assetID: "Unknown" }] })) });
    await page.waitForFunction(() => document.querySelector("#notice").className === "error" && !document.querySelector("#open-files").disabled);
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCommands, "failed reload keeps history");
    const fresh = await browser.newPage(); fresh.on("pageerror", error => errors.push(error.message));
    await fresh.goto(`http://127.0.0.1:${server.address().port}/`);
    await fresh.locator("#file-input").setInputFiles([
      { name: "Rock_01.fbx", mimeType: "application/octet-stream", buffer: fs.readFileSync(path.resolve(root, "../../Model/MapObject/Rock/Rock_01.fbx")) },
      { name: "MAP_OBJECT_ASSETS.json", mimeType: "application/json", buffer: fs.readFileSync(path.resolve(root, "../MAP_OBJECT_ASSETS.json")) },
      { name: "Saved.json", mimeType: "application/json", buffer: Buffer.from(JSON.stringify(json)) },
    ]);
    await fresh.waitForFunction(count => __check.models.length === count && !document.querySelector("#open-files").disabled, json.objects.length);
    const freshJSON = await fresh.evaluate(() => MapEditorCore.serializeMap({ mapName: "test", groundAssetTag: "" }, __check.models).objects);
    assert.deepEqual(freshJSON, json.objects);
    await fresh.close();

    // Visibility is editor-only; hidden objects remain in the map and list.
    await loadMap([object(-10), object(10)]); list = await ids(); await chooseAll();
    before = await state(); oldCursor = await cursor();
    await page.locator(`[data-visibility-id="${list[0]}"]`).click();
    assert.equal(await page.evaluate(id => __check.byId.get(id).root.visible, list[0]), false);
    assert.equal(await page.locator("[data-object-id]").count(), 2); assert.deepEqual(await selected(), [list[1]]);
    await clickModel(list[0]); assert.deepEqual(await selected(), [], "hidden object cannot be picked");
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    const hiddenJSON = await page.evaluate(() => MapEditorCore.serializeMap({ mapName: "test", groundAssetTag: "" }, __check.models));
    assert.equal(hiddenJSON.objects.length, 2);
    await page.locator("#show-all").click(); await chooseAll(); await page.locator("#hide-selected").click();
    assert.deepEqual(await selected(), []); assert.equal(await page.evaluate(() => __check.models.filter(model => model.root.visible).length), 0);
    await choose(list[0]); assert.deepEqual(await selected(), []);
    await page.locator(`[data-visibility-id="${list[0]}"]`).click(); assert.equal(await page.evaluate(id => __check.byId.get(id).root.visible, list[0]), true);
    await page.locator("#show-all").click(); assert.deepEqual(await state(), before);
    console.log("PASS visibility row/batch controls, deselection, pick filtering, show-all, unchanged data/history and JSON inclusion");

    await loadMap([object(-10), object(10)]); list = await ids(); await choose(list[0]);
    await enter("#position-x", -7); await page.locator(`[data-lock-id="${list[0]}"]`).click();
    assert.equal(await page.locator("#position-x").isDisabled(), true);
    assert.equal(await page.evaluate(() => __check.transform.object === undefined), true);
    before = await state(); oldCursor = await cursor();
    for (const selector of ["#position-x", "#rotation-y", "#scale-z"]) {
      await page.locator(selector).evaluate(input => { input.value = "99"; input.dispatchEvent(new Event("change", { bubbles: true })); });
    }
    await key("Control+d"); await key("Delete"); assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await key("Control+z"); close((await state())[0].p[0], -10);
    assert.equal(await page.evaluate(id => __check.lockedIds.has(id), list[0]), true, "Undo preserves editor lock");
    await key("Control+y"); close((await state())[0].p[0], -7);
    await clickModel(list[0]); assert.deepEqual(await selected(), [], "locked object cannot be viewport-picked");
    await choose(list[0]); assert.deepEqual(await selected(), [list[0]], "locked row can be referenced");
    await choose(list[1], ["Shift"]); before = await state(); await enter("#position-x", 21); after = await state();
    assert.deepEqual(after[0], before[0]); close(after[1].p[0], 21); await undoRedo(before, after);
    await key("Control+d"); assert.equal((await state()).length, 3, "mixed selection duplicates only editable object");
    await key("Control+z"); await key("Delete"); assert.equal((await state()).length, 1);
    assert.equal((await state())[0].id, list[0]); await key("Control+z");
    await page.locator("#lock-selected").click(); assert.equal(await page.evaluate(() => __check.models.filter(model => __check.lockedIds.has(model.id)).length), 2);
    const lockedJSON = await page.evaluate(() => MapEditorCore.serializeMap({ mapName: "test", groundAssetTag: "" }, __check.models));
    assert.equal(lockedJSON.objects.length, 2); lockedJSON.objects.forEach(entry => assert.deepEqual(Object.keys(entry), ["assetID", "position", "rotation", "scale"]));
    before = await state(); await page.locator("#unlock-all").click(); assert.deepEqual(await state(), before);
    assert.equal(await page.evaluate(() => __check.lockedIds.size), 0);
    console.log("PASS lock row/batch controls, read-only references, viewport filtering, mixed transform/delete/duplicate exclusion, lock-safe history and JSON inclusion");

    await loadMap([object(-10, 1, 0), { ...object(-1, 2, 9), rotation: [0.2, 0.5, 0.3], scale: [2, 3, 4] }, object(4, 3, 30), object(20), object(100)]);
    list = await ids(); await page.locator(`[data-visibility-id="${list[3]}"]`).click(); await page.locator(`[data-lock-id="${list[4]}"]`).click();
    await chooseAll(); await page.locator("#layout-tools").evaluate(details => { details.open = true; });
    before = await state(); oldCursor = await cursor(); await page.locator("#align-objects").click(); after = await state();
    assert.deepEqual(after.slice(0, 3).map(entry => entry.p[0]), [-3, -3, -3]);
    after.forEach((entry, i) => { assert.deepEqual(entry.r, before[i].r); assert.deepEqual(entry.s, before[i].s); close(entry.p[1], before[i].p[1]); close(entry.p[2], before[i].p[2]); });
    assert.deepEqual(after.slice(3), before.slice(3)); assert.equal(await cursor(), oldCursor + 1); await undoRedo(before, after);
    await key("Control+z"); before = await state(); await enter("#layout-gap", 5); await page.locator("#space-objects").click(); after = await state();
    assert.deepEqual(after.slice(0, 3).map(entry => entry.p[0]), [-10, -5, 0]); assert.deepEqual(after.slice(3), before.slice(3)); await undoRedo(before, after);
    await key("Control+z"); before = await state(); await page.locator("#spacing-kind").selectOption("ends"); await page.locator("#space-objects").click(); after = await state();
    assert.deepEqual(after.slice(0, 3).map(entry => entry.p[0]), [-10, -3, 4]); assert.deepEqual(after.slice(3), before.slice(3)); await undoRedo(before, after);
    await page.locator("#layout-axis").selectOption("z"); await page.locator("#spacing-kind").selectOption("step"); await enter("#layout-gap", 2);
    before = await state(); await page.locator("#space-objects").click(); after = await state();
    assert.deepEqual(after.slice(0, 3).map(entry => entry.p[2]), [-26, -28, -30]); await undoRedo(before, after);
    oldCursor = await cursor(); await enter("#layout-gap", -1); await page.locator("#space-objects").click(); assert.equal(await cursor(), oldCursor);
    console.log("PASS alignment and interval/end-fixed spacing, XYZ calculations, lock/visibility exclusion, unchanged rotations/scales, one-command exact undo/redo");

    // Random previews remain provisional until a single batch insertion command.
    await page.locator(".tool-menu > summary").click(); await page.locator("#open-random").click();
    assert.ok(await page.locator("#random-tools").evaluate(details => details.open));
    await page.locator("#random-asset").selectOption("Rock_01");
    // Random settings support scrubbing, direct typing, fine control and rollback.
    before = await state(); oldCursor = await cursor();
    for (const [selector, start, delta] of [["#random-count", 20, 4], ["#random-x", 0, 2], ["#random-y", 0, 2], ["#random-z", 0, 2], ["#random-radius", 10, 2], ["#random-distance", 1, 2], ["#random-yaw-min", 0, 10], ["#random-yaw-max", 90, 10], ["#random-scale-min", 1, 0.2], ["#random-scale-max", 2, 0.2]]) {
      await enter(selector, start); await drag(selector, 20); close(Number(await page.locator(selector).inputValue()), start + delta);
    }
    await enter("#random-x", 10); await drag("#random-x", 20, true); close(Number(await page.locator("#random-x").inputValue()), 10.2);
    await drag("#random-x", 30, false, true); close(Number(await page.locator("#random-x").inputValue()), 10.2);
    await enter("#random-radius", 1); await drag("#random-radius", -40); close(Number(await page.locator("#random-radius").inputValue()), 0.001);
    await enter("#random-count", 1999); await drag("#random-count", 40); assert.equal(await page.locator("#random-count").inputValue(), "2000");
    await page.locator("#random-shape").selectOption("rectangle");
    for (const selector of ["#random-width", "#random-depth"]) { await enter(selector, 10); await drag(selector, 20); close(Number(await page.locator(selector).inputValue()), 12); }
    await page.locator("#random-x").focus();
    const label = page.locator('label:has(> #random-x) > .scrub-label');
    await label.scrollIntoViewIfNeeded(); const labelBox = await label.boundingBox();
    await page.mouse.move(labelBox.x + 10, labelBox.y + 5); await page.mouse.down(); await page.mouse.move(labelBox.x + 30, labelBox.y + 5, { steps: 10 }); await page.mouse.up();
    close(Number(await page.locator("#random-x").inputValue()), 12.2);
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await page.locator("#random-shape").selectOption("circle"); await enter("#random-radius", 10);
    await enter("#random-x", 0.2); await enter("#random-y", 0); await enter("#random-z", 0);
    await key("e"); await page.locator('[data-space="local"]').click();
    await page.locator("#random-move").click();
    assert.ok(await page.evaluate(() => __check.scatter.editingCenter));
    assert.equal(await page.evaluate(() => __check.transform.space), "world");
    await page.locator("#position-snap").check(); await enter("#position-step", 1);
    async function centerDrag(view, axis, dx, dy, cancel = false) {
      await key(view); await key("f");
      const point = await gizmoPoint("translate", axis); assert.ok(point, "center " + axis + " handle");
      const origin = Number(await page.locator("#random-" + axis.toLowerCase()).inputValue());
      await page.mouse.move(point.x, point.y); await page.mouse.down();
      assert.ok(await page.evaluate(() => __check.transform.dragging));
      await page.mouse.move(point.x + dx, point.y + dy, { steps: 8 });
      const moved = Number(await page.locator("#random-" + axis.toLowerCase()).inputValue());
      assert.notEqual(moved, origin); close(moved, Math.round(moved));
      if (cancel) await page.keyboard.press("Escape");
      await page.mouse.up();
      if (cancel) close(Number(await page.locator("#random-" + axis.toLowerCase()).inputValue()), origin);
      assert.ok(await page.evaluate(() => __check.scatter.guide.position.equals(__check.transform.object.position)));
      assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    }
    await centerDrag("3", "X", 40, 0); await centerDrag("1", "Y", 0, -40); await centerDrag("3", "Z", 0, 40); await centerDrag("4", "X", 40, 0, true);
    await enter("#random-x", 31.25); close(await page.evaluate(() => __check.transform.object.position.x), 31.25);
    await page.locator("#random-move").click();
    assert.equal(await page.evaluate(() => __check.transform.mode), "rotate"); assert.equal(await page.evaluate(() => __check.transform.space), "local");
    assert.ok(await page.evaluate(() => __check.transform.object === __check.pivot));
    await page.locator("#random-move").click(); await choose((await selected())[0]); assert.equal(await page.evaluate(() => __check.scatter.editingCenter), false);
    await page.locator("#position-snap").uncheck(); await page.locator('[data-space="world"]').click();
    console.log("PASS random numeric scrubbing, direct input, fine adjustment, bounds, cancellation; shared center gizmo XYZ, live snap, view changes, exact cancellation and model editing restoration");
    await enter("#random-count", 24); await enter("#random-x", 100); await enter("#random-y", 5); await enter("#random-z", 100);
    await enter("#random-radius", 10); await enter("#random-distance", 1); await enter("#random-yaw-min", -30); await enter("#random-yaw-max", 60);
    await enter("#random-scale-min", 0.5); await enter("#random-scale-max", 1.8); await enter("#random-seed", "map-42");
    const previewState = () => page.evaluate(() => __check.scatter.preview.map(model => ({ id: model.id, assetID: model.assetID, p: model.root.position.toArray(), r: model.root.rotation.toArray(), s: model.root.scale.toArray() })));
    async function generate() { await page.locator("#random-preview").click(); await page.waitForFunction(() => !__check.scatter.generating && document.querySelector("#random-status").textContent.startsWith("未確定")); }
    before = await state(); oldCursor = await cursor(); await generate(); let provisional = await previewState();
    assert.equal(provisional.length, 24); assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    assert.ok(await page.evaluate(() => __check.scatter.guide.geometry.getAttribute("position").count > 8));
    provisional.forEach((entry, i) => {
      assert.equal(entry.id, null); assert.equal(entry.assetID, "Rock_01"); assert.ok(Math.hypot(entry.p[0] - 100, entry.p[2] - 100) <= 10);
      close(entry.p[1], 5); assert.ok(entry.r[1] >= -Math.PI / 6 && entry.r[1] <= Math.PI / 3);
      assert.ok(entry.s[0] >= 0.5 && entry.s[0] <= 1.8); close(entry.s[0], entry.s[1]); close(entry.s[0], entry.s[2]);
      provisional.slice(0, i).forEach(other => assert.ok(Math.hypot(entry.p[0] - other.p[0], entry.p[2] - other.p[2]) >= 1));
    });
    const previewJSON = await page.evaluate(() => MapEditorCore.serializeMap({ mapName: "test", groundAssetTag: "" }, __check.models));
    assert.equal(previewJSON.objects.length, before.length);
    await generate(); assert.deepEqual(await previewState(), provisional, "same seed/config/world gives same provisional transforms");
    await drag("#random-radius", 20); assert.equal((await previewState()).length, 0, "scrubbing invalidates the old preview");
    await enter("#random-radius", 10); await generate(); assert.deepEqual(await previewState(), provisional);
    await page.locator("#random-move").click();
    await page.locator("#position-snap").check(); await centerDrag("3", "X", 40, 0);
    assert.equal((await previewState()).length, 0, "moving the center invalidates the old preview");
    await enter("#random-x", 100); await page.locator("#position-snap").uncheck();
    await generate(); assert.deepEqual(await previewState(), provisional, "center editing also permits normal preview generation");
    await page.locator("#random-move").click();
    await enter("#random-seed", "changed"); assert.equal((await previewState()).length, 0); assert.equal(await page.locator("#random-confirm").isDisabled(), true);
    await enter("#random-seed", "map-42"); await generate(); assert.deepEqual(await previewState(), provisional);
    await page.locator("#random-frame").click(); await page.waitForFunction(() => !__check.focusAnimation);
    await page.locator("#random-cancel").click(); assert.equal((await previewState()).length, 0); assert.equal(await page.evaluate(() => __check.scatter.guide), null);
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await page.locator("#random-shape").selectOption("rectangle"); await enter("#random-count", 12); await enter("#random-x", -5); await enter("#random-z", 30);
    await enter("#random-width", 8); await enter("#random-depth", 12); await enter("#random-distance", 0.2);
    await page.locator("#random-yaw").uncheck(); await page.locator("#random-scale").uncheck(); await generate(); provisional = await previewState();
    assert.equal(provisional.length, 12); assert.equal(await page.evaluate(() => __check.scatter.guide.geometry.getAttribute("position").count), 8);
    provisional.forEach(entry => { assert.ok(Math.abs(entry.p[0] + 5) <= 4 && Math.abs(entry.p[2] - 30) <= 6); assert.deepEqual(entry.r, [0, 0, 0, "YXZ"]); assert.deepEqual(entry.s, [1, 1, 1]); });
    await page.locator("#random-confirm").click(); after = await state(); assert.equal(after.length, before.length + 12); assert.equal(await cursor(), oldCursor + 1);
    assert.equal(new Set(after.map(entry => entry.id)).size, after.length); assert.equal((await selected()).length, 12);
    assert.ok((await selected()).every(id => !before.some(entry => entry.id === id))); await undoRedo(before, after);
    assert.equal((await previewState()).length, 0);
    const scatterDownload = await downloadMap();
    const scatterJSON = JSON.parse(fs.readFileSync(await scatterDownload.path(), "utf8"));
    assert.equal(scatterJSON.objects.length, after.length); scatterJSON.objects.forEach(entry => assert.deepEqual(Object.keys(entry), ["assetID", "position", "rotation", "scale"]));
    await page.locator("#file-input").setInputFiles({ name: "Scatter.json", mimeType: "application/json", buffer: Buffer.from(JSON.stringify(scatterJSON)) });
    await page.waitForFunction(() => !document.querySelector("#open-files").disabled && __check.history.cursor === 0);
    assert.deepEqual(await page.evaluate(() => MapEditorCore.serializeMap({ mapName: "test", groundAssetTag: "" }, __check.models).objects), scatterJSON.objects);
    assert.equal(await page.evaluate(() => __check.hiddenIds.size + __check.lockedIds.size), 0);
    before = await state(); oldCursor = await cursor();
    await page.locator("#random-shape").selectOption("circle"); await enter("#random-count", 10); await enter("#random-x", 0); await enter("#random-z", 0);
    await enter("#random-radius", 0.01); await enter("#random-distance", 1000); await generate();
    assert.equal((await previewState()).length, 0); assert.ok((await page.locator("#random-status").textContent()).includes("試行上限")); assert.equal(await page.locator("#random-confirm").isDisabled(), true);
    await page.locator("#terrain-input").setInputFiles({ name: "ScatterGround.fbx", mimeType: "application/octet-stream", buffer: require("./MapEditor.fixtures.cjs").terrainFBX() });
    await page.waitForFunction(() => __check.terrainReferences.size === 1 && !document.querySelector("#open-terrain").disabled);
    await enter("#random-count", 4); await enter("#random-x", 30); await enter("#random-y", 100); await enter("#random-z", 30); await enter("#random-radius", 5); await enter("#random-distance", 0);
    await page.locator("#random-ground").check(); await generate(); assert.equal((await previewState()).length, 4);
    const contacts = await page.evaluate(() => __check.scatter.preview.map(model => { const box = __check.geometryTools.boundsOf([model.root], true); return { bottom: box.min.y, height: 0.2 * box.max.x + 0.1 * box.max.z + 3 }; }));
    contacts.forEach(contact => close(contact.bottom, contact.height));
    await page.locator("#random-cancel").click(); await enter("#random-x", 1000); await enter("#random-z", 1000); await enter("#random-count", 3); await generate();
    assert.equal((await previewState()).length, 0); assert.ok((await page.locator("#random-status").textContent()).includes("地面なし"));
    await enter("#random-count", 2000); await page.locator("#random-preview").click(); await page.waitForFunction(() => __check.scatter.generating);
    await page.locator("#random-cancel").click(); await page.waitForFunction(() => !__check.scatter.generating);
    assert.equal((await previewState()).length, 0); assert.equal(await page.evaluate(() => __check.scatter.guide), null);
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await page.locator("#random-tools").evaluate(details => { details.open = false; });
    await page.locator(".terrain-tools").evaluate(details => { details.open = true; }); await page.locator("#clear-terrain").click();
    console.log("PASS seeded circle/rectangle preview, ranges/spacing/guides, no preview serialization/history, cancel, batch commit/undo/redo, JSON round-trip, ground reuse/failure, bounded generation and live cancellation");

    // Search is view-only, preserving hidden selections and independent IDs.
    await loadMap([object(-12), object(12), object(36)]); list = await ids();
    await choose(list[1]); await enter("#asset-id", "Tree_Search");
    await choose(list[2]); await enter("#asset-id", "ROCK_01");
    before = await state(); oldCursor = await cursor();
    await page.locator("#object-search").fill("rOcK_");
    assert.equal(await page.locator("[data-object-id]").count(), 2);
    assert.equal(await page.locator("#model-count").textContent(), "2 / 3 件");
    await choose(list[0]); await choose(list[2], ["Control"]); assert.equal((await selected()).length, 2);
    await page.locator("#object-search").fill("no match"); assert.equal(await page.locator("[data-object-id]").count(), 0);
    assert.equal((await selected()).length, 2); assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await page.locator("#clear-search").click(); assert.equal(await page.locator("[data-object-id]").count(), 3);
    await page.locator("#object-search").fill("tree"); await choose(list[1]); assert.deepEqual(await selected(), [list[1]]);
    await page.locator("#clear-search").click();
    console.log("PASS live case-insensitive search, counts, clear, duplicate IDs, filtered multi-selection, no scene/history changes");

    // Space switches affect only the proxy and use the last selected object.
    await loadMap([object(-10), { ...object(10), rotation: [0, Math.PI / 2, 0] }]); list = await ids(); await chooseAll();
    before = await state(); oldCursor = await cursor();
    await page.locator('[data-space="local"]').click();
    assert.equal(await page.evaluate(() => __check.selected.id), list[1]);
    assert.equal(await page.evaluate(() => __check.transform.space), "local");
    close(await page.evaluate(() => Math.abs(__check.pivot.quaternion.dot(__check.selected.root.quaternion))), 1);
    await page.locator('[data-space="world"]').click(); await page.locator('[data-space="local"]').click();
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await page.locator("#position-snap").check(); await enter("#position-step", 1); await page.locator("#rotation-snap").check(); await enter("#rotation-step", 15);
    const localMove = await gizmoDrag("w", "translate", "X", 0, 40, true);
    localMove.later.forEach((entry, i) => {
      close(entry.p[0], localMove.prior[i].p[0]);
      const delta = entry.p[2] - localMove.prior[i].p[2]; close(delta, Math.round(delta));
      assert.deepEqual(entry.r, localMove.prior[i].r);
    });
    await gizmoDrag("e", "rotate", "Y", 40, 30, true);
    await gizmoDrag("r", "scale", "X", 0, 40, true);
    before = await state(); await page.locator('[data-space="world"]').click(); assert.deepEqual(await state(), before);
    await gizmoDrag("w", "translate", "X", 40, 0);
    console.log("PASS active-object Local/World axes, switching without TRS/history changes, real local/world gizmos, snapping, scale and exact undo/redo");

    // Load a real sloped FBX as a reference: y = 0.2*x + 0.1*z + 3.
    await page.locator("#position-snap").uncheck(); await page.locator("#rotation-snap").uncheck();
    await loadMap([
      { ...object(-12, 80), rotation: [0.3, 0.5, 0.2], scale: [2, 3, 1.5] },
      { ...object(12, -5), rotation: [-0.2, 0.4, -0.3], scale: [0.7, 2, 1] },
      object(1000, 12),
      { ...object(0, 500), scale: [200, 1, 200] }, // A building above the terrain must not be a hit target.
    ]);
    list = await ids(); before = await state(); oldCursor = await cursor();
    await page.locator("#terrain-input").setInputFiles({ name: "SlopedTerrain.fbx", mimeType: "application/octet-stream", buffer: require("./MapEditor.fixtures.cjs").terrainFBX() });
    await page.waitForFunction(() => __check.terrainReferences.size === 1 && !document.querySelector("#open-terrain").disabled);
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    const expectedHeights = await page.evaluate(ids => ids.slice(0, 2).map(id => {
      const root = __check.byId.get(id).root, bounds = __check.geometryTools.boundsOf([root], true);
      return root.position.y + 0.2 * bounds.max.x + 0.1 * bounds.max.z + 3 - bounds.min.y;
    }), list);
    await choose(list[0]); await choose(list[1], ["Shift"]); await choose(list[2], ["Shift"]);
    await page.locator("#snap-ground").click(); after = await state();
    assert.equal(await cursor(), oldCursor + 1);
    after.forEach((entry, i) => {
      close(entry.p[0], before[i].p[0]); close(entry.p[2], before[i].p[2]);
      assert.deepEqual(entry.r, before[i].r); assert.deepEqual(entry.s, before[i].s);
      if (i < 2) close(entry.p[1], expectedHeights[i]); else assert.deepEqual(entry.p, before[i].p);
    });
    await undoRedo(before, after);
    assert.equal((await state()).length, 4, "reference terrain is not a placed object");
    await key("Control+z"); await page.locator(`[data-lock-id="${list[0]}"]`).click();
    const lockedDropBefore = await state(); await page.locator("#snap-ground").click(); const lockedDropAfter = await state();
    assert.deepEqual(lockedDropAfter[0], lockedDropBefore[0], "locked target excluded from ground drop");
    close(lockedDropAfter[1].p[1], expectedHeights[1]); await undoRedo(lockedDropBefore, lockedDropAfter);
    await page.locator("#unlock-all").click();
    await page.locator(".terrain-tools").evaluate(details => { details.open = true; });
    await page.locator("#clear-terrain").click(); before = await state(); oldCursor = await cursor();
    await page.locator("#snap-ground").click(); assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor);
    await choose(list[3]); await page.locator("#is-terrain").check();
    assert.equal(await page.evaluate(id => __check.terrainIds.has(id), list[3]), true);
    before = await state(); oldCursor = await cursor(); await page.locator("#snap-ground").click();
    assert.deepEqual(await state(), before); assert.equal(await cursor(), oldCursor, "terrain does not snap to itself");
    await key("Delete"); await key("Control+z"); assert.equal(await page.evaluate(id => __check.terrainIds.has(id), list[3]), true);
    const terrainJSON = await page.evaluate(() => MapEditorCore.serializeMap({ mapName: "test", groundAssetTag: "" }, __check.models));
    terrainJSON.objects.forEach(entry => assert.deepEqual(Object.keys(entry), ["assetID", "position", "rotation", "scale"]));
    console.log("PASS uneven terrain FBX, transformed bounds, Y-only multi-drop, one-command undo/redo, ignoring buildings/self/missing ground, editor-only terrain roles and unchanged JSON");

    // Camera focus animates, fits geometric bounds and keeps view orientation.
    await loadMap([object(-20), object(20)]); list = await ids(); await choose(list[0]); await key("4");
    const directionBefore = await page.evaluate(() => __check.camera.position.clone().sub(__check.controls.target).normalize().toArray());
    await page.locator(`[data-object-id="${list[0]}"]`).dblclick();
    await page.waitForFunction(() => !__check.focusAnimation);
    const singleFocus = await page.evaluate(id => ({
      center: __check.geometryTools.boundsOf([__check.byId.get(id).root]).getCenter(new MapEditorThree.Vector3()).toArray(),
      target: __check.controls.target.toArray(),
      direction: __check.camera.position.clone().sub(__check.controls.target).normalize().toArray(),
    }), list[0]);
    singleFocus.center.forEach((v, i) => close(v, singleFocus.target[i]));
    singleFocus.direction.forEach((v, i) => close(v, directionBefore[i]));
    await chooseAll(); await page.locator("#viewport").focus(); await page.keyboard.press("f");
    assert.ok(await page.evaluate(() => !!__check.focusAnimation), "focus is interpolated, not instantaneous");
    await page.waitForFunction(() => !__check.focusAnimation);
    assert.ok(await page.evaluate(() => {
      __check.camera.updateMatrixWorld(true);
      const box = __check.geometryTools.boundsOf(__check.models.map(model => model.root));
      for (const x of [box.min.x, box.max.x]) for (const y of [box.min.y, box.max.y]) for (const z of [box.min.z, box.max.z]) {
        const p = new MapEditorThree.Vector3(x, y, z).project(__check.camera);
        if (Math.abs(p.x) > 1 || Math.abs(p.y) > 1 || Math.abs(p.z) > 1) return false;
      }
      return true;
    }), "all selected geometry fits the view");
    await choose(list[0]); await page.locator("#viewport").focus(); await page.keyboard.press("f");
    const focusRect = await page.locator("#viewport").boundingBox(); await page.mouse.move(focusRect.x + 100, focusRect.y + 100); await page.mouse.wheel(0, 100);
    await page.waitForFunction(() => !__check.focusAnimation);
    await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
    const interruptedTarget = await page.evaluate(() => __check.controls.target.toArray());
    await page.waitForTimeout(280); assert.deepEqual(await page.evaluate(() => __check.controls.target.toArray()), interruptedTarget, "manual zoom cancels interpolation");
    await page.locator("#object-search").focus(); await page.keyboard.press("f"); assert.equal(await page.evaluate(() => !!__check.focusAnimation), false);
    await page.locator("#clear-search").click();
    await page.mouse.click(focusRect.x + 18, focusRect.y + 65);
    const noSelectionCamera = await page.evaluate(() => __check.camera.position.toArray()); await key("f");
    assert.deepEqual(await page.evaluate(() => __check.camera.position.toArray()), noSelectionCamera);
    await key("3"); await chooseAll(); await key("f");
    assert.ok(await page.evaluate(() => __check.camera.isOrthographicCamera));
    console.log("PASS double-click/F focus, single/multi bounds, smooth animation, view direction, manual cancellation, input guard, no-selection and orthographic camera");

    // Focus guards: native text editing must not delete, duplicate, undo or switch views.
    list = await ids(); await choose(list[0]); await page.locator("#position-x").focus();
    const countBeforeTyping = (await state()).length, cursorBeforeTyping = await cursor();
    for (const shortcut of ["Control+z", "Control+y", "Control+Shift+z", "Control+d", "Delete"]) await page.keyboard.press(shortcut);
    assert.equal((await state()).length, countBeforeTyping); assert.equal(await cursor(), cursorBeforeTyping);
    await page.locator("#position-x").fill("1"); await page.keyboard.press("1"); assert.equal(await page.locator("#view-name").textContent(), "上・平行投影");
    await page.locator("#position-x").press("Tab");
    // Browser-native text Undo/Redo can restore edits in another input, including
    // the search field. Clear that view-only condition before choosing all rows.
    if (await page.locator("#object-search").inputValue()) await page.locator("#clear-search").click();
    await key("4"); assert.ok(await page.evaluate(() => __check.camera.isPerspectiveCamera));
    await key("1"); await key("2"); await key("3"); await key("f");
    const rect = await page.locator("#viewport").boundingBox(), x = rect.x + rect.width / 2, y = rect.y + rect.height / 2;
    await page.mouse.move(x, y); await page.mouse.wheel(0, -120);
    await page.mouse.down({ button: "middle" }); await page.mouse.move(x + 30, y + 20); await page.mouse.up({ button: "middle" });
    await page.mouse.down({ button: "right" }); await page.mouse.move(x + 90, y + 50); await page.mouse.up({ button: "right" });
    assert.equal(await page.locator("#context-menu").isVisible(), false);
    await chooseAll(); await key("f");
    await page.screenshot({ path: path.join(artifactDir, "map-editor-multiselect.png") });
    await page.setViewportSize({ width: 650, height: 900 });
    assert.ok(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
    assert.deepEqual(errors, []);
    console.log("PASS saved JSON reload (same and fresh sessions), schema, reset/failed load, input guards, camera operations, responsive UI, no browser errors");
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exitCode = 1; }).finally(() => server.close());
