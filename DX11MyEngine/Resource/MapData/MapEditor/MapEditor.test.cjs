"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const C = require("./MapEditor.core.js");

let value = 0;
const history = new C.History(3);
const command = next => { const previous = value; return { label: String(next), undo: () => { value = previous; }, redo: () => { value = next; } }; };
history.commit(command(1)); history.commit(command(2));
history.undo(); assert.equal(value, 1); history.redo(); assert.equal(value, 2);
history.undo(); history.commit(command(3)); assert.equal(history.canRedo, false);
history.commit(command(4)); history.commit(command(5)); assert.equal(history.commands.length, 3);
while (history.canUndo) history.undo(); assert.equal(value, 1);
while (history.canRedo) history.redo(); assert.equal(value, 5);
history.clear(); assert.equal(history.canUndo, false); assert.equal(history.canRedo, false);

// IDs, rather than list positions or asset IDs, identify transform targets.
const objects = new Map([[31, { assetID: "Rock_01", position: [0, 1, 2] }], [47, { assetID: "Rock_01", position: [10, 1, 2] }]]);
const before = [...objects].map(([id, state]) => ({ id, ...structuredClone(state) }));
const after = before.map(state => ({ ...state, position: state.position.map(v => v + 5) }));
const apply = states => { for (const state of states) objects.set(state.id, structuredClone(state)); };
history.commit(new C.TransformCommand("group", before, after, {}, {}, apply));
assert.deepEqual(objects.get(47).position, [15, 6, 7]);
history.undo(); assert.deepEqual(objects.get(47).position, [10, 1, 2]);
history.redo(); assert.deepEqual(objects.get(31).position, [5, 6, 7]);
assert.equal(history.commands.length, 1);

assert.equal(C.snap(2.7, 1), 3); assert.equal(C.snap(-2.7, 1), -3);
assert.equal(C.snap(23, 15), 30);
assert.equal(C.snapDrag(0.31, 0.3, 1), 0.3); // no jump on pointer-down
assert.equal(C.snapDrag(0.6, 0.3, 1), 1);
assert.equal(C.snapDrag(-0.6, -0.3, 1), -1);
assert.equal(C.jsonFileName("MountainsA"), "MountainsA.json");
assert.equal(C.jsonFileName("MAP_MOUNTAINS_A.json"), "MAP_MOUNTAINS_A.json");
assert.equal(C.jsonFileName(" map:name?.JSON "), "map_name_.JSON");
assert.equal(C.jsonFileName("..."), "Map.json");

const fixture = JSON.parse(fs.readFileSync(path.join(__dirname, "../MAP_MOUNTAINS_A.json"), "utf8"));
C.validateMap(fixture);
const vector = values => ({ toArray: () => [...values] });
const models = fixture.objects.map((entry, id) => {
  const state = C.fromGameTransform(entry);
  return { id, selected: true, assetID: entry.assetID,
    root: { position: vector(state.position), rotation: { x: state.rotation[0], y: state.rotation[1], z: state.rotation[2] }, scale: vector(state.scale) } };
});
const serialized = C.serializeMap(fixture, models);
assert.deepEqual(JSON.parse(JSON.stringify(serialized)), fixture);
for (const entry of serialized.objects) assert.deepEqual(Object.keys(entry), ["assetID", "position", "rotation", "scale"]);
assert.throws(() => C.validateMap({ ...fixture, objects: [{ ...fixture.objects[0], position: [NaN, 1, 2] }] }));
assert.throws(() => C.validateMap({ ...fixture, objects: [{ ...fixture.objects[0], assetID: "" }] }));
assert.equal(C.equal(before, structuredClone(before)), true);
const P = require("./MapEditor.placement.js");
const layout = [{ id: 8, position: [10, 2, 9] }, { id: 3, position: [-2, 5, 0] }, { id: 5, position: [1, -4, 4] }];
const layoutBefore = structuredClone(layout);
assert.deepEqual(P.alignmentPlan(layout, "x", "center").map(item => item.position[0]), [4, 4, 4]);
assert.deepEqual(P.alignmentPlan(layout, "y", "min").map(item => item.position[1]), [-4, -4, -4]);
assert.deepEqual(P.alignmentPlan(layout, "z", "max").map(item => item.position[2]), [9, 9, 9]);
assert.deepEqual(P.spacingPlan(layout, "x", "step", 3).map(item => [item.id, item.position[0]]), [[3, -2], [5, 1], [8, 4]]);
assert.deepEqual(P.spacingPlan(layout, "x", "ends", 0).map(item => item.position[0]), [-2, 4, 10]);
assert.deepEqual(layout, layoutBefore);
assert.throws(() => P.spacingPlan(layout.slice(0, 2), "x", "ends", 1));
assert.throws(() => P.spacingPlan(layout, "x", "step", -1));
const scatterConfig = { count: 100, shape: "circle", centerX: 4, centerY: 7, centerZ: -3, radius: 10, width: 20, depth: 12, minDistance: 1, randomYaw: true, yawMin: -45, yawMax: 60, randomScale: true, scaleMin: 0.5, scaleMax: 2, seed: "map-test" };
function scatterResult(config, existing = []) {
  const generator = P.createScatter(config, existing), result = [];
  for (let step = generator.next(); !step.done; step = generator.next()) if (step.candidate) { result.push(step.candidate); generator.accept(step.candidate); }
  return { result, attempts: generator.attempts, limit: generator.limit };
}
const samples = scatterResult(scatterConfig).result;
assert.equal(samples.length, 100); assert.deepEqual(scatterResult(scatterConfig).result, samples);
assert.notDeepEqual(scatterResult({ ...scatterConfig, seed: "different" }).result, samples);
samples.forEach((sample, index) => {
  assert.ok(Math.hypot(sample.position[0] - 4, sample.position[2] + 3) <= 10);
  assert.equal(sample.position[1], 7); assert.equal(sample.rotation[2], 0); assert.equal(sample.rotation[3], C.rotationOrder);
  assert.ok(sample.rotation[1] >= -Math.PI / 4 && sample.rotation[1] <= Math.PI / 3);
  assert.ok(sample.scale[0] >= 0.5 && sample.scale[0] <= 2); assert.equal(sample.scale[0], sample.scale[1]); assert.equal(sample.scale[0], sample.scale[2]);
  samples.slice(0, index).forEach(other => assert.ok(Math.hypot(sample.position[0] - other.position[0], sample.position[2] - other.position[2]) >= 1));
});
const rectangles = scatterResult({ ...scatterConfig, shape: "rectangle", randomYaw: false, randomScale: false }).result;
rectangles.forEach(sample => { assert.ok(Math.abs(sample.position[0] - 4) <= 10 && Math.abs(sample.position[2] + 3) <= 6); assert.deepEqual(sample.rotation, [0, 0, 0, C.rotationOrder]); assert.deepEqual(sample.scale, [1, 1, 1]); });
const impossible = scatterResult({ ...scatterConfig, radius: 0.01, minDistance: 100 }, [[4, 999, -3]]);
assert.equal(impossible.result.length, 0); assert.equal(impossible.attempts, impossible.limit);
const uniform = scatterResult({ ...scatterConfig, count: 2000, minDistance: 0 }).result;
const meanRadiusSquared = uniform.reduce((sum, sample) => sum + (sample.position[0] - 4) ** 2 + (sample.position[2] + 3) ** 2, 0) / uniform.length;
assert.ok(meanRadiusSquared > 45 && meanRadiusSquared < 55, "uniform area distribution has E[r²] = R²/2");
assert.throws(() => P.validateScatter({ ...scatterConfig, count: 2001 }));
assert.deepEqual(C.filterObjects([{ id: 1, assetID: "Rock_01" }, { id: 2, assetID: "ROCK_01" }, { id: 3, assetID: "Tree_01" }], "oCk").map(model => model.id), [1, 2]);
assert.equal(C.filterObjects(models, "   ").length, models.length);

// Use the actual bundled geometry/raycast implementation without a WebGL renderer.
const vm = require("node:vm");
const context = { console, TextDecoder, TextEncoder, AbortController, AbortSignal, setTimeout, clearTimeout };
try { vm.runInNewContext(fs.readFileSync(path.join(__dirname, "vendor/three-tools.js"), "utf8"), context); }
catch (error) { throw new Error(`Geometry test runtime: ${error.message}`); }
const T = context.MapEditorThree;
// Independent reference: the scalar implementation of DirectXMath's
// XMQuaternionRotationRollPitchYaw in the game's installed Windows SDK.
function gameQuaternion([pitch, yaw, roll]) {
  const cp = Math.cos(pitch / 2), sp = Math.sin(pitch / 2), cy = Math.cos(yaw / 2), sy = Math.sin(yaw / 2), cr = Math.cos(roll / 2), sr = Math.sin(roll / 2);
  return { x: cr * sp * cy + sr * cp * sy, y: cr * cp * sy - sr * sp * cy, z: sr * cp * cy - cr * sp * sy, w: cr * cp * cy + sr * sp * sy };
}
for (const rotation of [[0, 0, 0], [0, Math.PI / 2, 0], [0.4, -0.7, 1.2], [-0.6, 0.3, -0.8], [Math.PI / 2, -1.3, 0.5], [8, -10, 3]]) {
  for (const z of [-120, 0, 75]) {
    const game = { assetID: "Rock_01", position: [7, -3, z], rotation, scale: [2, 3, 0.5] };
    const state = C.fromGameTransform(game), preview = new T.Group();
    preview.position.fromArray(state.position); preview.rotation.fromArray(state.rotation); preview.scale.fromArray(state.scale); preview.updateMatrixWorld(true);
    assert.equal(preview.position.z, z === 0 ? 0 : -z);
    assert.equal(preview.rotation.order, "YXZ");
    const roundTrip = C.serializeMap({ mapName: "axes", groundAssetTag: "" }, [{ assetID: game.assetID, root: preview }]);
    assert.deepEqual(JSON.parse(JSON.stringify(roundTrip.objects[0])), game);
    for (const vertex of [[2, 4, -5], [-7, 1, 9], [0, 0, 0]]) {
      const editorPoint = new T.Vector3(...vertex).applyMatrix4(preview.matrixWorld);
      // Assimp MakeLeftHanded reflects the source FBX vertex along Z.
      const gamePoint = new T.Vector3(vertex[0], vertex[1], -vertex[2]).multiply(new T.Vector3(...game.scale)).applyQuaternion(gameQuaternion(rotation)).add(new T.Vector3(...game.position));
      gamePoint.z = -gamePoint.z;
      assert.ok(editorPoint.distanceTo(gamePoint) < 1e-10, "editor preview must match the reflected game TRS, including compound rotations");
    }
  }
}
console.log("PASS: game/editor Z conversion in both directions; compound YXZ rotations match DirectXMath; unchanged JSON schema and exact transform round-trip.");
const tools = require("./MapEditor.scene.js")(T);
const bytes = require("./MapEditor.fixtures.cjs").terrainFBX();
const terrain = new T.FBXLoader().parse(bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength), "");
let mesh; terrain.traverse(node => { if (node.isMesh) mesh = node; });
assert.ok(mesh, "FBX slope fixture loaded");
context.window = context;
vm.runInNewContext(fs.readFileSync(path.join(__dirname, "MapEditor.appearance.js"), "utf8"), context);
const appearance = context.createMapEditorAppearance(), colorRootA = new T.Group(), colorRootB = new T.Group();
const colorMeshA = mesh.clone(), colorMeshB = mesh.clone(); colorRootA.add(colorMeshA); colorRootB.add(colorMeshB);
const importedMaterial = mesh.material, importedColor = importedMaterial.color.getHexString(), importedMap = importedMaterial.map;
const mockTexture = { isTexture: true }; importedMaterial.map = mockTexture;
appearance.setMode("solid"); appearance.setBaseColor("#0000ff"); appearance.setColor(colorRootA, "#ff0000");
let renderedOverride;
appearance.render({ render() {
  assert.notEqual(colorMeshA.material, importedMaterial); assert.notEqual(colorMeshA.material, colorMeshB.material);
  assert.equal(colorMeshA.material.color.getHexString(), "ff0000"); assert.equal(colorMeshB.material.color.getHexString(), "0000ff");
  assert.equal(colorMeshA.material.map, null); assert.equal(colorMeshA.material.wireframe, false); renderedOverride = colorMeshA.material;
} }, null, null, [colorRootA, colorRootB]);
assert.equal(colorMeshA.material, importedMaterial); assert.equal(importedMaterial.map, mockTexture); assert.equal(importedMaterial.color.getHexString(), importedColor);
appearance.setMode("wireframe");
assert.throws(() => appearance.render({ render() { assert.equal(colorMeshA.material.wireframe, true); throw new Error("render probe"); } }, null, null, [colorRootA]), /render probe/);
assert.equal(colorMeshA.material, importedMaterial, "restore originals even after failed renders");
appearance.copyColor(colorRootA, colorRootB); assert.equal(appearance.colorOf(colorRootB), "#ff0000");
appearance.clearColor(colorRootB); assert.equal(appearance.colorOf(colorRootB), "#0000ff");
appearance.setAsset(colorRootA, "Tree_01"); appearance.setAsset(colorRootB, "Tree_01"); appearance.clearColor(colorRootA);
appearance.setAssetColor("Tree_01", "#22aa33");
assert.equal(appearance.colorOf(colorRootA), "#22aa33"); assert.equal(appearance.colorOf(colorRootB), "#22aa33");
const laterTree = new T.Group(); appearance.setAsset(laterTree, "Tree_01"); assert.equal(appearance.colorOf(laterTree), "#22aa33");
appearance.setColor(colorRootB, "#ff8800"); assert.equal(appearance.colorOf(colorRootB), "#ff8800");
appearance.clearColor(colorRootB); appearance.setAsset(colorRootB, "TREE_01"); assert.equal(appearance.colorOf(colorRootB), "#0000ff");
appearance.setAsset(colorRootB, "Tree_01"); assert.equal(appearance.colorOf(colorRootB), "#22aa33");
appearance.clearAssetColor("Tree_01"); assert.equal(appearance.colorOf(colorRootA), "#0000ff");
let materialReleased = false; renderedOverride.addEventListener("dispose", () => { materialReleased = true; });
appearance.release(colorRootA); appearance.release(colorRootB); assert.equal(materialReleased, true);
importedMaterial.map = importedMap;
console.log("PASS: solid/wire display overrides, per-object colors, shared texture/material preservation, exception restoration and override disposal.");
const close = (actual, expected) => assert.ok(Math.abs(actual - expected) < 1e-6, `${actual} != ${expected}`);
// Reuse the Mesh class from the loaded FBX to build a box whose origin is offset.
const geometry = new mesh.geometry.constructor();
const vertices = new Float32Array([-1,0,-1,1,0,-1,1,2,-1,-1,2,-1,-1,0,1,1,0,1,1,2,1,-1,2,1]);
geometry.setAttribute("position", new (mesh.geometry.getAttribute("position").constructor)(vertices, 3));
const selectLeft = new mesh.constructor(geometry, mesh.material), selectRight = selectLeft.clone(), selectBehind = selectLeft.clone(), selectFar = selectLeft.clone(), selectHidden = selectLeft.clone();
selectLeft.position.x = -4; selectRight.position.x = 4; selectBehind.position.z = 30; selectFar.position.z = -200; selectHidden.visible = false;
const selectOffset = new T.Group(), offsetChild = selectLeft.clone(); selectOffset.position.x = 100; offsetChild.position.x = -104; selectOffset.add(offsetChild);
const boxCamera = new T.PerspectiveCamera(60, 1, 0.1, 100); boxCamera.position.set(0, 1, 20); boxCamera.lookAt(0, 1, 0);
const boxCandidates = [selectLeft, selectRight, selectBehind, selectFar, selectHidden, selectOffset];
const leftRect = { left: -0.8, right: -0.15, top: 0.5, bottom: -0.5 }, fullRect = { left: -1, right: 1, top: 1, bottom: -1 };
assert.deepEqual(tools.rectangleSelection(boxCandidates, boxCamera, leftRect), [selectLeft, selectOffset]);
assert.deepEqual(tools.rectangleSelection(boxCandidates, boxCamera, fullRect), [selectLeft, selectRight, selectOffset]);
const flatCamera = new T.OrthographicCamera(-10, 10, 10, -10, 0.1, 100); flatCamera.position.copy(boxCamera.position); flatCamera.lookAt(0, 1, 0);
assert.deepEqual(tools.rectangleSelection(boxCandidates, flatCamera, leftRect), [selectLeft, selectOffset]);
assert.deepEqual(tools.rectangleSelection(boxCandidates, flatCamera, fullRect), [selectLeft, selectRight, selectOffset]);
const nearCamera = selectLeft.clone(); nearCamera.position.set(0, 0, 19.95);
assert.equal(tools.rectangleSelection([nearCamera], boxCamera, fullRect).length, 1, "bounds crossing the near plane remain selectable");
console.log("PASS: rectangle sub-frustum bounds in perspective/orthographic views, behind/far/hidden exclusion, offset origins and near-plane crossings; asset colors inherit independently of instance colors.");
const box = new mesh.constructor(geometry, mesh.material);
box.position.set(10, 50, 20); box.rotation.set(0.3, 0.5, 0.2); box.scale.set(2, 3, 1.5);
const boxBefore = { p: box.position.toArray(), r: box.rotation.toArray(), s: box.scale.toArray() };
const bounds = tools.boundsOf([box], true);
const expectedY = box.position.y + (0.2 * bounds.max.x + 0.1 * bounds.max.z + 3) - bounds.min.y;
close(tools.groundY(box, [terrain]), expectedY);
assert.deepEqual({ p: box.position.toArray(), r: box.rotation.toArray(), s: box.scale.toArray() }, boxBefore, "contact calculation is read-only");
const oldSide = mesh.material.side; tools.groundY(box, [terrain]); assert.equal(mesh.material.side, oldSide);
box.position.x = 1000; assert.equal(tools.groundY(box, [terrain]), null);
assert.equal(tools.groundY(terrain, [terrain]), null, "never snap to self");
terrain.visible = false; box.position.x = 0; assert.equal(tools.groundY(box, [terrain]), null); terrain.visible = true;
const camera = new T.PerspectiveCamera(50, 1, 0.01, 10000);
for (const size of [1e-8, 1, 1e12]) {
  const bounds = new T.Box3(new T.Vector3(-size, -size, -size), new T.Vector3(size, size, size));
  const frame = tools.focusFrame(bounds, camera, 1);
  assert.ok(frame.distance > frame.radius && frame.near < frame.distance && frame.far > frame.distance);
  assert.ok([frame.distance, frame.height, frame.near, frame.far].every(Number.isFinite));
}
assert.equal(tools.focusFrame(new T.Box3(), camera, 1), null);
console.log("PASS: history undo/redo, branching, limit, reset; stable IDs and duplicate asset IDs; group command; snap boundaries; existing map JSON round-trip and validation.");
console.log("PASS: case-insensitive search; real FBX terrain raycasts; rotated/scaled/off-center bounds; missing/hidden/self terrain; focus frame at extreme scales.");
console.log("PASS: immutable alignment/spacing plans; deterministic seeded circle/rectangle scatter; uniform area; ranges; minimum XZ spacing; bounded failure.");
